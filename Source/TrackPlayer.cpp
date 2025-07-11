/*
  ==============================================================================

    trackPlayer.cpp
    Created: 7 Jul 2025 9:02:50pm
    Author:  Kisuke

  ==============================================================================
*/

#include "TrackPlayer.h"

MultipleTrackPlayer::MultipleTrackPlayer(juce::MidiOutput* out): outputDevice{out}, currentElapsedTime{0.0}
{

}

void MultipleTrackPlayer::setTracks(const std::vector<TrackEntry>& newTracks)
{
    currentTracks = newTracks;
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

void MultipleTrackPlayer::setCurrentBPM(int newBPM)
{
    this->currentBPM = newBPM;
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
        DBG("Volume for channel " << channel << " is " << newVolume);
        if(newInstrument!=-1 && channel!=10)
            outputDevice->sendMessageNow(juce::MidiMessage::programChange(channel, newInstrument));

        if (newVolume != -1)
            outputDevice->sendMessageNow(juce::MidiMessage::controllerEvent(channel, 7, newVolume));
    }
}

MultipleTrackPlayer::~MultipleTrackPlayer()
{

}

void MultipleTrackPlayer::applyBPMchangeDuringPlayback(double newBPM)
{
    if (currentTracks.empty())
        return;

    double oldElapsed = currentElapsedTime;

    // MIDI ticks per quarter note (you should get this from your MIDI files!)
    double tpqn = 960.0;

    // Calculate new seconds per tick based on new BPM
    double newSecondsPerTick = 60.0 / (newBPM * tpqn);

    // Update timestamps in filteredSequences, preserving channels and startup delays
    for (size_t i = 0; i < filteredSequences.size(); ++i)
    {
        auto& filteredSeq = filteredSequences[i];

        for (int e = 0; e < filteredSeq.getNumEvents(); ++e)
        {
            auto* event = filteredSeq.getEventPointer(e);
            // Original tick timestamp (convert from seconds back to ticks first)
            // This assumes original event timestamps were in seconds at old BPM
            // We'll calculate new timestamp with new BPM.

            // To do this accurately, you should store the original tick timestamp per event somewhere!
            // If you don't, you can approximate by:
            double originalTickTime = event->message.getTimeStamp() / (60.0 / (currentBPM * tpqn));
            double newTimeInSeconds = originalTickTime * newSecondsPerTick;

            // Preserve startup delay (assuming first event time includes startup delay)
            // You may want to handle startup delay separately

            juce::MidiMessage newMessage = event->message;
            newMessage.setTimeStamp(newTimeInSeconds);
            filteredSeq.getEventPointer(e)->message = newMessage;
        }

        filteredSeq.updateMatchedPairs();
    }

    // Update playback timing
    double now = static_cast<double>(juce::Time::getHighResolutionTicks()) /
        static_cast<double>(juce::Time::getHighResolutionTicksPerSecond());

    startTime = now - oldElapsed;
    currentElapsedTime = oldElapsed;

    // Update nextEventIndex to resume playback at correct event
    for (auto& track : tracks)
    {
        auto& sequence = filteredSequences[track.filteredSequenceIndex];
        track.nextEventIndex = findNextEventIndex(filteredSequences[track.filteredSequenceIndex], currentElapsedTime);
    }

    currentBPM = newBPM; // store the new BPM somewhere accessible
}

int MultipleTrackPlayer::findNextEventIndex(const juce::MidiMessageSequence& seq, double currentTime)
{
    int left = 0;
    int right = seq.getNumEvents() - 1;
    int result = seq.getNumEvents();

    while (left <= right)
    {
        int mid = (left + right) / 2;
        if (seq.getEventPointer(mid)->message.getTimeStamp() >= currentTime)
        {
            result = mid;
            right = mid - 1;
        }
        else
        {
            left = mid + 1;
        }
    }

    return result;
}

void MultipleTrackPlayer::applyBPMchangeBeforePlayback(double baseBPM, double newBPM)
{
    if (baseBPM <= 0.0) baseBPM = 120.0; 

    currentBPM = newBPM;
    int j = 0;

    double bpmRatio = newBPM / baseBPM;

    filteredSequences.clear();

    for (const auto& tr : currentTracks)
    {
        int channel;
        if (tr.type == TrackType::Percussion)
            channel = 10;
        else {
            channel = j + 2;
            j += 1;
        }

        juce::MidiMessageSequence newSequence;

        for (int i = 0; i < tr.sequence.getNumEvents(); ++i)
        {
            const auto& event = tr.sequence.getEventPointer(i)->message;

            // Scale relative to original time in seconds
            double scaledTime = event.getTimeStamp() * (baseBPM / newBPM);

            juce::MidiMessage newMessage = event;
            newMessage.setTimeStamp(scaledTime);
            newMessage.setChannel(channel);
            newSequence.addEvent(newMessage);
        }

        newSequence.updateMatchedPairs();
        filteredSequences.push_back(newSequence);
    }

    tracks.clear();
    for (int i = 0; i < (int)filteredSequences.size(); ++i)
    {
        tracks.push_back(TrackPlaybackData{ i, 0 });
    }
}

void MultipleTrackPlayer::syncPlaybackSettings()
{
    int j = 0;
    for (int i = 0; i < currentTracks.size(); i++)
    {
        auto& track = currentTracks[i];
        int channel;
        if (track.type == TrackType::Percussion)
            channel = 10;
        else {
            channel = j + 2;
            j++;
        }
        int instrument = track.instrumentAssociated;
        int volume = static_cast<int>(track.volumeAssociated);
        updatePlaybackSettings(channel, volume, instrument);
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
    currentElapsedTime = elapsed;
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
