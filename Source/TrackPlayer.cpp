/*
  ==============================================================================

    trackPlayer.cpp
    Created: 7 Jul 2025 9:02:50pm
    Author:  Kisuke

  ==============================================================================
*/

#include "TrackPlayer.h"

MultipleTrackPlayer::MultipleTrackPlayer(std::weak_ptr<juce::MidiOutput> out): outputDevice{out}, currentElapsedTime{0.0}
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

        //apply small startup delay, not in the same for so it's safer glitch wise because of pairs note on-note off
        if (filteredSeq.getNumEvents() > 0)
        {
            filteredSeq.updateMatchedPairs();
            
            /*
            for (int e = 0; e < filteredSeq.getNumEvents(); ++e)
            {
                auto* event = filteredSeq.getEventPointer(e);
                juce::MidiMessage shiftedMsg = event->message;
                shiftedMsg.setTimeStamp(shiftedMsg.getTimeStamp() + startupDelay);
                filteredSeq.getEventPointer(e)->message = shiftedMsg;
            }
            */

            filteredSequences.push_back(filteredSeq);
            int newIndex = static_cast<int>(filteredSequences.size()) - 1;
            tracks.push_back(TrackPlaybackData{ newIndex, 0 });
            
        }
    }
}

void MultipleTrackPlayer::setCurrentBPM(int newBPM)
{
    this->currentBPM = newBPM;
}

void MultipleTrackPlayer::stop()
{

    stopTimer();
    this->lastKnownSequenceTime = 0.0;
    if (onElapsedUpdate)
    {
        juce::MessageManager::callAsync([this]() {
            onElapsedUpdate(0.0);
            });
    }

    if (auto sharedPtrDev=outputDevice.lock())
    {
        for (int channel = 1; channel <= 16; ++channel)
            sharedPtrDev->sendMessageNow(juce::MidiMessage::allNotesOff(channel));
    }
}

void MultipleTrackPlayer::start()
{

    startTime = static_cast<double>(juce::Time::getHighResolutionTicks()) / static_cast<double>(juce::Time::getHighResolutionTicksPerSecond());
    lastKnownSequenceTime = 0.0;
    currentElapsedTime = 0.0;
    DBG("Playback started at time: " << startTime);

    for (auto& track : tracks)
        track.nextEventIndex = 0;

    startTimer(10);
}

void MultipleTrackPlayer::updatePlaybackSettings(int channel, int newVolume, int newInstrument)
{
    if (auto sharedPtrDev=outputDevice.lock())
    {
        if(newInstrument!=-1 && channel!=10)
            sharedPtrDev->sendMessageNow(juce::MidiMessage::programChange(channel, newInstrument));

        if (newVolume != -1)
            sharedPtrDev->sendMessageNow(juce::MidiMessage::controllerEvent(channel, 7, newVolume));
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

    double tpqn = 960.0;

    double newSecondsPerTick = 60.0 / (newBPM * tpqn);

    for (size_t i = 0; i < filteredSequences.size(); ++i)
    {
        auto& filteredSeq = filteredSequences[i];

        for (int e = 0; e < filteredSeq.getNumEvents(); ++e)
        {
            auto* event = filteredSeq.getEventPointer(e);

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

void MultipleTrackPlayer::setDeviceOutputTrackPlayer(std::weak_ptr<juce::MidiOutput> newOutput)
{
    this->outputDevice = newOutput;
}

void MultipleTrackPlayer::setTimeSignatureDenominator(int newDenominator)
{
    this->timeSignatureDenominator = newDenominator;
}

void MultipleTrackPlayer::setTimeSignatureNumerator(int newNumerator)
{
    this->timeSignatureNumerator = newNumerator;
}

void MultipleTrackPlayer::hiResTimerCallback()
{
    if (auto midiOut = outputDevice.lock())
    {
        double now = juce::Time::getHighResolutionTicks() /
            static_cast<double>(juce::Time::getHighResolutionTicksPerSecond());
        double elapsed = now - startTime;  // seconds elapsed since start
        currentElapsedTime = elapsed;

        double rawBeatsElapsed = elapsed * (currentBPM / 60.0);


        double beatsElapsed = elapsed * (currentBPM / 60.0);

        juce::MessageManager::callAsync([this,beatsElapsed,elapsed]
            {
                onElapsedUpdate(beatsElapsed);
                notifyMidPlay(elapsed);
            });


        for (auto& track : tracks)
        {
            auto& sequence = filteredSequences[track.filteredSequenceIndex];
            while (track.nextEventIndex < sequence.getNumEvents())
            {
                const auto& midiEvent = sequence.getEventPointer(track.nextEventIndex)->message;
                double eventTime = midiEvent.getTimeStamp();

                if (eventTime <= elapsed)
                {
                    midiOut->sendMessageNow(midiEvent); 
                    track.nextEventIndex++;
                }
                else break;
            }
        }

        bool allDone = std::all_of(tracks.begin(), tracks.end(),
            [this](const TrackPlaybackData& t) {
                return t.nextEventIndex >= filteredSequences[t.filteredSequenceIndex].getNumEvents();
            });

        if (allDone)
        {
            stop();
            return;
        }
    }
    else
    {
        stop();
        return;
    }
}
