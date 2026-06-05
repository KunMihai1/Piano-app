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

void ArrangerEngine::rebuildScheduler()
{
    std::vector<TimedBeatEvent> merged;
    double bars = 1.0;

    if (! style.sections.empty())
    {
        const auto& sec = style.sections.front();
        bars = (double) juce::jmax (1, sec.lengthBars);
        for (const auto& tr : sec.tracks)
            for (const auto& ev : tr.pattern)
                merged.push_back (ev);
    }

    const double bpb = ArrangerTime::beatsPerBar (style.timeSigNum, style.timeSigDenom);
    loopLengthBeats = bars * bpb;
    scheduler.setLoop (std::move (merged), loopLengthBeats);
}

void ArrangerEngine::setStyle (ArrangerStyle newStyle)
{
    style = std::move (newStyle);
    if (style.originalTempo > 0.0) currentBpm = style.originalTempo;
    rebuildScheduler();
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
    // Mirror MultipleTrackPlayer::syncPlaybackSettings: select each track's instrument and
    // set its channel volume, so the arranger doesn't play everything as the default sound.
    // Drum channel (10) gets no program change (its kit is fixed). The SFZ engine ignores
    // program-change but honours CC7 volume, and an external MIDI synth honours both.
    if (style.sections.empty())
        return;

    for (const auto& tr : style.sections.front().tracks)
    {
        if (tr.instrument >= 0 && tr.channel != 10)
            dispatch (juce::MidiMessage::programChange (tr.channel, tr.instrument));

        dispatch (juce::MidiMessage::controllerEvent (tr.channel, 7, juce::jlimit (0, 127, (int) tr.volume)));
    }
}

void ArrangerEngine::renderRange (double fromBeats, double toBeats)
{
    auto events = scheduler.advance (fromBeats, toBeats);
    for (auto& e : events)
        dispatch (e.message);
}

void ArrangerEngine::start()
{
    if (loopLengthBeats <= 0.0)
        return;

    scheduler.reset();
    sendInstrumentSetup();   // select instruments + volumes before the first notes play
    playheadBeats = 0.0;
    lastNowSeconds = (double) juce::Time::getHighResolutionTicks()
                     / (double) juce::Time::getHighResolutionTicksPerSecond();
    playing.store (true);
    startTimer (10);
}

void ArrangerEngine::stop()
{
    stopTimer();
    playing.store (false);
    sendAllNotesOff();
    scheduler.reset();
    playheadBeats = 0.0;
    if (onElapsedBeats)   // reset the beat bar to the downbeat, like the classic player does
        juce::MessageManager::callAsync ([this] { if (onElapsedBeats) onElapsedBeats (0.0); });
}

void ArrangerEngine::hiResTimerCallback()
{
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

    if (onElapsedBeats)
    {
        const double beats = playheadBeats;
        juce::MessageManager::callAsync ([this, beats] { if (onElapsedBeats) onElapsedBeats (beats); });
    }
}
