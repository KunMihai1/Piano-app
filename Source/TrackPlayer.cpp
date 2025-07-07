/*
  ==============================================================================

    TrackPlayer.cpp
    Created: 7 Jul 2025 9:02:50pm
    Author:  Kisuke

  ==============================================================================
*/

#include "TrackPlayer.h"

MultipleTrackPlayer::MultipleTrackPlayer(juce::MidiOutput* out): outputDevice{out}
{

}

void MultipleTrackPlayer::setTracks(const std::vector<TrackEntry>& newTracks)
{
    tracks.clear();
    DBG("YOU'RE WRONG" << juce::String(newTracks.size()));
    for (auto& tr : newTracks)
    {
        if (tr.sequence.getNumEvents() > 0)
            tracks.push_back(TrackPlaybackData{ &tr.sequence,0 });
    }
}

void MultipleTrackPlayer::stop()
{
    DBG("STOP CALLED!");
    stopTimer();

    if (outputDevice)
    {
        for (int channel = 1; channel <= 16; ++channel)
            outputDevice->sendMessageNow(juce::MidiMessage::allNotesOff(channel));
    }
}

void MultipleTrackPlayer::start()
{

    DBG("Ticks per second: " << juce::Time::getHighResolutionTicksPerSecond());
    startTime = static_cast<double>(juce::Time::getHighResolutionTicks()) / static_cast<double>(juce::Time::getHighResolutionTicksPerSecond());
    for (auto& track : tracks)
        track.nextEventIndex = 0;
    startTimer(10);
}

void MultipleTrackPlayer::hiResTimerCallback()
{
    DBG("hiResTimerCallback running");
    if (!outputDevice)
    {
        DBG("output issue");
        return;
    }

    double now = static_cast<double>(juce::Time::getHighResolutionTicks()) / static_cast<double>(juce::Time::getHighResolutionTicksPerSecond());
    double elapsed = now - startTime;
    DBG("Elapsed time: " << elapsed);

    for (auto& track : tracks)
    {
        while (track.nextEventIndex < track.sequence->getNumEvents())
        {
            auto& midiEvent = track.sequence->getEventPointer(track.nextEventIndex)->message;
            double eventTime = midiEvent.getTimeStamp();
            auto& nextMidiEvent = track.sequence->getEventPointer(track.nextEventIndex)->message;
            DBG("Next event time: " << nextMidiEvent.getTimeStamp());

            if (eventTime <= elapsed)
            {
                if (midiEvent.isNoteOn() && midiEvent.getVelocity() > 0)
                {
                    DBG("Sending Note On: note " << midiEvent.getNoteNumber() << ", velocity " << (int)midiEvent.getVelocity() << ", time " << eventTime);
                }
                DBG("Sending MIDI event at time " << eventTime);
                DBG("Sending MIDI event: " << midiEvent.getDescription());
                if (outputDevice)
                    outputDevice->sendMessageNow(midiEvent);

                track.nextEventIndex++;
            }
            else {
                break;
            }
        }
    }

    bool allDone = std::all_of(tracks.begin(), tracks.end(),
        [](const TrackPlaybackData& t) {
            return t.nextEventIndex >= t.sequence->getNumEvents();
        });

    if (allDone)
        stop();

}
