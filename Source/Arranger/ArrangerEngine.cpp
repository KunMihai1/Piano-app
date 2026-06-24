#include "ArrangerEngine.h"
#include "ArrangerTime.h"

ArrangerEngine::ArrangerEngine (std::weak_ptr<juce::MidiOutput> out) : outputDevice (out) {}

ArrangerEngine::~ArrangerEngine()
{
    stopTimer();
}

void ArrangerEngine::setDeviceOutput (std::weak_ptr<juce::MidiOutput> out)
{
    outputDevice = out;
}

void ArrangerEngine::setBpm (double bpm)
{
    if (bpm > 0.0) currentBpm = bpm;
}

void ArrangerEngine::rebuildFromStyle()
{
    schedulers.clear();
    const double bpb = ArrangerTime::beatsPerBar (style.timeSigNum, style.timeSigDenom);

    for (const auto& sec : style.sections)
    {
        std::vector<TimedBeatEvent> merged;
        std::vector<PartKind>       parts;
        for (const auto& tr : sec.tracks)
        {
            // Drums/Perc never transpose; Bass honours bass inversion; Acc is plain harmony.
            const PartKind pk = (tr.partType == ArrangerPartType::Bass) ? PartKind::Bass
                              : (tr.partType == ArrangerPartType::Acc)  ? PartKind::Acc
                                                                        : PartKind::Fixed;
            for (const auto& ev : tr.pattern) { merged.push_back (ev); parts.push_back (pk); }
        }

        const double bars = (double) juce::jmax (1, sec.lengthBars);
        ArrangerScheduler s;
        s.setLoop (std::move (merged), std::move (parts), bars * bpb);
        schedulers.push_back (std::move (s));
    }

    sequencer.setStyle (style);
    currentSchedulerIndex = -1;
    updateActiveLoopLength();
}

void ArrangerEngine::updateActiveLoopLength()
{
    if (style.sections.empty())
    {
        loopLengthBeats = 0.0;
        return;
    }
    const double bpb = ArrangerTime::beatsPerBar (style.timeSigNum, style.timeSigDenom);
    const int idx = juce::jlimit (0, (int) style.sections.size() - 1, sequencer.getActiveIndex());
    loopLengthBeats = (double) juce::jmax (1, style.sections[idx].lengthBars) * bpb;
}

int ArrangerEngine::indexOfSection (ArrangerSectionType type, const juce::String& name) const
{
    // Match by type + trailing number (like the sequencer's findSection), so the live button labels
    // ("Var 2") resolve to the matching section ("Variation 2") even though the strings differ.
    const int wantN = name.getTrailingIntValue();
    int firstOfType = -1, numMatch = -1;
    for (int i = 0; i < (int) style.sections.size(); ++i)
    {
        if (style.sections[i].type != type) continue;
        if (firstOfType < 0) firstOfType = i;
        if (style.sections[i].name.getTrailingIntValue() == wantN) numMatch = i;
    }
    return (numMatch >= 0) ? numMatch : firstOfType;
}

int ArrangerEngine::getActiveSectionIndex() const
{
    return sequencer.getActiveIndex();
}

void ArrangerEngine::queueSection (ArrangerSectionType type, const juce::String& name)
{
    // Called from the message thread. Hand the request to the timer thread instead of touching the
    // sequencer directly (it's advanced on the timer thread; a direct write here is a data race).
    const juce::ScopedLock sl (requestLock);
    requestedType    = type;
    requestedName    = name;
    hasQueuedRequest = true;
}

void ArrangerEngine::selectStartSection (ArrangerSectionType type, const juce::String& name)
{
    pendingStartIndex = indexOfSection (type, name);
}

void ArrangerEngine::setStyle (ArrangerStyle newStyle)
{
    style = std::move (newStyle);
    if (style.originalTempo > 0.0) currentBpm = style.originalTempo;
    rebuildFromStyle();
    // The recorded chord the NTT maps out of (style-level). bassNote = root (no inversion baked in).
    transposer.setOriginalChord ({ style.originalRoot, style.originalQuality, style.originalRoot });
    // Start the new style in its OWN home key — don't carry the chord held under the previous style.
    transposer.setActiveChord ({});
    {
        const juce::ScopedLock sl (chordLock);
        pendingChord   = {};
        hasChordUpdate = false;
    }
}

void ArrangerEngine::setActiveChord (ArrangerChord played)
{
    // Called from the MIDI input thread. Hand the chord to the timer thread via a guarded mailbox;
    // it is applied to the transposer at the top of renderRange (timer thread only).
    const juce::ScopedLock sl (chordLock);
    pendingChord   = played;
    hasChordUpdate = true;
}

void ArrangerEngine::setOriginalChord (ArrangerChord recorded)
{
    transposer.setOriginalChord (recorded);
}

void ArrangerEngine::dispatch (const juce::MidiMessage& m)
{
    if (auto out = outputDevice.lock())
        out->sendMessageNow (m);
    if (onMidiMessage)
        onMidiMessage (m);
}

void ArrangerEngine::dispatchEmitted (const EmittedEvent& e)
{
    juce::MidiMessage m = e.message;

    if (m.isNoteOn() && m.getVelocity() > 0 && e.part != PartKind::Fixed)
    {
        // Transpose the pitched note and remember the played pitch, keyed by its ORIGINAL note, so
        // the matching note-off (which carries the original pitch) closes the same sounding note even
        // if the chord changed in between.
        const int orig   = m.getNoteNumber();
        const int played = transposer.transpose (orig, e.part);
        activePlayedNote[{ m.getChannel(), orig }] = played;
        m = juce::MidiMessage::noteOn (m.getChannel(), played, m.getVelocity());
    }
    else if (m.isNoteOff() || (m.isNoteOn() && m.getVelocity() == 0))
    {
        const auto key = std::make_pair (m.getChannel(), m.getNoteNumber());
        const auto it  = activePlayedNote.find (key);
        if (it != activePlayedNote.end())
        {
            m = juce::MidiMessage::noteOff (m.getChannel(), it->second);
            activePlayedNote.erase (it);
        }
    }

    dispatch (m);
}

void ArrangerEngine::sendAllNotesOff()
{
    for (int ch = 1; ch <= 16; ++ch)
        dispatch (juce::MidiMessage::allNotesOff (ch));
}

void ArrangerEngine::sendInstrumentSetup()
{
    // Select each track's instrument + channel volume across all sections, so any section
    // we switch to already has the right sound. Drum channel (10) keeps its fixed kit. The
    // SFZ engine ignores program-change but honours CC7 volume; an external synth honours both.
    for (const auto& sec : style.sections)
        for (const auto& tr : sec.tracks)
        {
            if (tr.instrument >= 0 && tr.channel != 10)
                dispatch (juce::MidiMessage::programChange (tr.channel, tr.instrument));

            dispatch (juce::MidiMessage::controllerEvent (tr.channel, 7, juce::jlimit (0, 127, (int) tr.volume)));
        }
}

void ArrangerEngine::renderRange (double fromBeats, double toBeats)
{
    if (schedulers.empty())
        return;

    // Apply any UI-queued section switch before advancing. Done here (not only in the timer callback)
    // so the request is honoured whether we're driven by the timer or by a direct renderRange call
    // (e.g. unit tests). The lock guards the request fields; sequencer mutation stays on this thread.
    {
        const juce::ScopedLock sl (requestLock);
        if (hasQueuedRequest)
        {
            sequencer.queue (requestedType, requestedName);
            hasQueuedRequest = false;
        }
    }

    // Apply the latest played chord (from the MIDI input thread) before emitting this window's notes.
    {
        const juce::ScopedLock sl (chordLock);
        if (hasChordUpdate)
        {
            transposer.setActiveChord (pendingChord);
            hasChordUpdate = false;
        }
    }

    SequencerStep step = sequencer.advance (fromBeats, toBeats);

    for (const auto& seg : step.segments)
    {
        if (seg.sectionIndex != currentSchedulerIndex)
        {
            // Flush the outgoing section's hung notes, then reset both sides so the
            // incoming section enters clean at its own bar 0.
            if (currentSchedulerIndex >= 0 && currentSchedulerIndex < (int) schedulers.size())
            {
                for (auto& e : schedulers[currentSchedulerIndex].flushActiveNotes (0.0))
                    dispatchEmitted (e);
                schedulers[currentSchedulerIndex].reset();
            }
            currentSchedulerIndex = seg.sectionIndex;
            if (currentSchedulerIndex >= 0 && currentSchedulerIndex < (int) schedulers.size())
                schedulers[currentSchedulerIndex].reset();
        }

        if (seg.sectionIndex >= 0 && seg.sectionIndex < (int) schedulers.size())
            for (auto& e : schedulers[seg.sectionIndex].advance (seg.localFromBeats, seg.localToBeats))
                dispatchEmitted (e);
    }

    if (step.stopRequested)
    {
        haltAudio();            // silence + reset, but do not touch the timer here
        timerShouldStop = true; // the timer callback will stop the timer on the message thread
        return;
    }

    updateActiveLoopLength();
}

void ArrangerEngine::start()
{
    if (schedulers.empty() || loopLengthBeats <= 0.0)
        return;

    for (auto& s : schedulers) s.reset();
    sequencer.reset();
    activePlayedNote.clear();
    if (pendingStartIndex >= 0)
        sequencer.startAt (pendingStartIndex);
    pendingStartIndex = -1;
    currentSchedulerIndex = -1;
    lastReportedSectionIndex = -1;   // so the first tick reports the starting section
    timerShouldStop = false;

    sendInstrumentSetup();   // select instruments + volumes before the first notes play
    playheadBeats = 0.0;
    lastNowSeconds = (double) juce::Time::getHighResolutionTicks()
                     / (double) juce::Time::getHighResolutionTicksPerSecond();
    playing.store (true);
    startTimer (10);
}

void ArrangerEngine::stop()
{
    stopTimer();   // only safe off the timer thread; the Ending path defers this via callAsync
    haltAudio();
}

void ArrangerEngine::haltAudio()
{
    const bool wasPlaying = playing.exchange (false);
    sendAllNotesOff();
    for (auto& s : schedulers) s.reset();
    sequencer.reset();
    activePlayedNote.clear();
    currentSchedulerIndex = -1;
    pendingStartIndex = -1;
    playheadBeats = 0.0;
    lastReportedSectionIndex = -1;
    if (onElapsedBeats)   // reset the beat bar to the downbeat, like the classic player does
        juce::MessageManager::callAsync ([this] { if (onElapsedBeats) onElapsedBeats (0.0); });

    // Only when we were actually playing: re-arm the first variation (sequencer.reset() landed on it)
    // and tell the UI we stopped, so the buttons clear the Ending and the beat bar leaves its play state.
    if (wasPlaying)
    {
        notifyActiveSection (true);
        if (onStoppedItself)
            juce::MessageManager::callAsync ([this] { if (onStoppedItself) onStoppedItself(); });
    }
}

void ArrangerEngine::notifyActiveSection (bool force)
{
    const int idx = sequencer.getActiveIndex();
    if (! force && idx == lastReportedSectionIndex)
        return;
    lastReportedSectionIndex = idx;

    ArrangerSectionType type = ArrangerSectionType::Variation;
    juce::String name;
    if (idx >= 0 && idx < (int) style.sections.size())
    {
        type = style.sections[(size_t) idx].type;
        name = style.sections[(size_t) idx].name;
    }
    if (onActiveSectionChanged)
        juce::MessageManager::callAsync ([this, idx, type, name]
            { if (onActiveSectionChanged) onActiveSectionChanged (idx, type, name); });
}

void ArrangerEngine::hiResTimerCallback()
{
    if (! playing.load())
        return;

    // Accumulate beats from the per-tick wall-clock delta so that a BPM change mid-playback
    // changes the rate going forward without jumping the musical position.
    const double now = (double) juce::Time::getHighResolutionTicks()
                       / (double) juce::Time::getHighResolutionTicksPerSecond();
    const double deltaSeconds = now - lastNowSeconds;
    lastNowSeconds = now;

    // (A UI-queued section switch is drained inside renderRange, below.)
    const double deltaBeats = ArrangerTime::secondsToBeats (deltaSeconds, currentBpm);
    const double from = playheadBeats;
    const double to   = playheadBeats + deltaBeats;

    renderRange (from, to);
    playheadBeats = to;

    notifyActiveSection (false);   // highlight the live button for the section now sounding

    if (timerShouldStop)
    {
        timerShouldStop = false;
        juce::MessageManager::callAsync ([this] { stop(); }); // stopTimer() safely off the timer thread
        return;
    }

    if (onElapsedBeats)
    {
        const double beats = playheadBeats;
        juce::MessageManager::callAsync ([this, beats] { if (onElapsedBeats) onElapsedBeats (beats); });
    }
}
