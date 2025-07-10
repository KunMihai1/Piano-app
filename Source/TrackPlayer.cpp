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
    filteredSequences.clear();  // Clear old sequences first

    const double startupDelay = 0.05;  // 50 ms delay before starting playback

    int j = 0;

    for (auto& tr : newTracks)
    {
        juce::MidiMessageSequence filteredSeq;
        int channel;
        if (tr.type == TrackType::Percussion)
            channel = 10;
        else
        {
            channel = j + 2;
            j++;
        }

        for (int i = 0; i < tr.sequence.getNumEvents(); ++i)
        {
            const auto& msg = tr.sequence.getEventPointer(i)->message;
            if (msg.isNoteOn() || msg.isNoteOff())
            {
                auto modifiedMsg = msg;
                modifiedMsg.setChannel(channel);
                filteredSeq.addEvent(modifiedMsg);
            }
        }

        if (filteredSeq.getNumEvents() > 0)
        {
            filteredSeq.updateMatchedPairs();

            for (int e = 0; e < filteredSeq.getNumEvents(); ++e)
            {
                auto* event = filteredSeq.getEventPointer(e);
                juce::MidiMessage shiftedMsg = event->message;
                shiftedMsg.setTimeStamp(shiftedMsg.getTimeStamp() + startupDelay);
                filteredSeq.getEventPointer(e)->message = shiftedMsg;
            }

            filteredSequences.push_back(filteredSeq);
            int newIndex = static_cast<int>(filteredSequences.size()) - 1;
            tracks.push_back(TrackPlaybackData{ newIndex, 0 });
        }
    }

    DBG("Tracks loaded: " << tracks.size());
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

void MultipleTrackPlayer::updatePlaybackSettings(int channel, int newVolume, int newInstrument)
{
    if (outputDevice)
    {
        DBG("Channel:"+ juce::String(channel));
        if(newInstrument!=-1 && channel!=10)
            outputDevice->sendMessageNow(juce::MidiMessage::programChange(channel, newInstrument));

        if (newVolume != -1)
            outputDevice->sendMessageNow(juce::MidiMessage::controllerEvent(channel, 7, newVolume));
    }
}

MultipleTrackPlayer::~MultipleTrackPlayer()
{

}

void MultipleTrackPlayer::applyBPMchangeDuringPlayback(double newBPM, double elapsedTime)
{
    filteredSequences.clear();

    double secondsPerQuarterNote = 60.0 / newBPM;
    int tpqn = 960;
    double secondsPerTick = secondsPerQuarterNote / tpqn;

    int j = 0;
    for (auto& track : tracks)

    {

    }
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
    if (onElapsedUpdate)
    {
        auto lambda = [this, elapsed]()
        {
            onElapsedUpdate(elapsed);
        };
        juce::MessageManager::callAsync(lambda);
    }

    for (auto& track : tracks)
    {
        auto& sequence = filteredSequences[track.filteredSequenceIndex];
        while (track.nextEventIndex < sequence.getNumEvents())
        {
            auto& midiEvent = sequence.getEventPointer(track.nextEventIndex)->message;
            double eventTime = midiEvent.getTimeStamp();
            auto& nextMidiEvent = sequence.getEventPointer(track.nextEventIndex)->message;
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
        [this](const TrackPlaybackData& t) {
            return t.nextEventIndex >= filteredSequences[t.filteredSequenceIndex].getNumEvents();
        });

    if (allDone)
        stop();

}
