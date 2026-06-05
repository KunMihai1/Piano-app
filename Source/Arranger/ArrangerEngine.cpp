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
        for (const auto& tr : sec.tracks)
            for (const auto& ev : tr.pattern)
                merged.push_back (ev);

        const double bars = (double) juce::jmax (1, sec.lengthBars);
        ArrangerScheduler s;
        s.setLoop (std::move (merged), bars * bpb);
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
    int firstOfType = -1, nameMatch = -1;
    for (int i = 0; i < (int) style.sections.size(); ++i)
    {
        if (style.sections[i].type != type) continue;
        if (firstOfType < 0) firstOfType = i;
        if (style.sections[i].name == name) nameMatch = i;
    }
    return (nameMatch >= 0) ? nameMatch : firstOfType;
}

int ArrangerEngine::getActiveSectionIndex() const
{
    return sequencer.getActiveIndex();
}

void ArrangerEngine::queueSection (ArrangerSectionType type, const juce::String& name)
{
    sequencer.queue (type, name);
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
}

void ArrangerEngine::dispatch (const juce::MidiMessage& m)
{
    if (auto out = outputDevice.lock())
        out->sendMessageNow (m);
    if (onMidiMessage)
        onMidiMessage (m);
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
                    dispatch (e.message);
                schedulers[currentSchedulerIndex].reset();
            }
            currentSchedulerIndex = seg.sectionIndex;
            if (currentSchedulerIndex >= 0 && currentSchedulerIndex < (int) schedulers.size())
                schedulers[currentSchedulerIndex].reset();
        }

        if (seg.sectionIndex >= 0 && seg.sectionIndex < (int) schedulers.size())
            for (auto& e : schedulers[seg.sectionIndex].advance (seg.localFromBeats, seg.localToBeats))
                dispatch (e.message);
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
    if (pendingStartIndex >= 0)
        sequencer.startAt (pendingStartIndex);
    pendingStartIndex = -1;
    currentSchedulerIndex = -1;
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
    playing.store (false);
    sendAllNotesOff();
    for (auto& s : schedulers) s.reset();
    sequencer.reset();
    currentSchedulerIndex = -1;
    pendingStartIndex = -1;
    playheadBeats = 0.0;
    if (onElapsedBeats)   // reset the beat bar to the downbeat, like the classic player does
        juce::MessageManager::callAsync ([this] { if (onElapsedBeats) onElapsedBeats (0.0); });
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

    const double deltaBeats = ArrangerTime::secondsToBeats (deltaSeconds, currentBpm);
    const double from = playheadBeats;
    const double to   = playheadBeats + deltaBeats;

    renderRange (from, to);
    playheadBeats = to;

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
