/*
  ==============================================================================

    MidiRecordPlayer.cpp
    Created: 25 May 2025 1:21:59am
    Author:  Kisuke

  ==============================================================================
*/

#include "MidiRecordPlayer.h"

MidiRecordPlayer::MidiRecordPlayer(): programLeftHand{0}, programRightHand{0}
{

}

MidiRecordPlayer::MidiRecordPlayer(std::weak_ptr<juce::MidiOutput> out): midiOutputDevice{out}, programLeftHand{0}, programRightHand{0}
{
}

void MidiRecordPlayer::startRecording()
{
    allEventsPlayed.clear();
    allEventsPlayedFile.clear();
    applyPresetFunction();
    recordStartTime = juce::Time::getMillisecondCounterHiRes() * 0.001;
    isRecording = true;
}

bool MidiRecordPlayer::stopRecording()
{
    if (isRecording == false) //looks unecessary but it's actually necessary
        return 0;

    isRecording = false;
    return 1;
}

bool MidiRecordPlayer::startPlayBack()
{
    if (allEventsPlayed.size()<=1)
    {
        notifyFunction();
        return 0;
    }
    if (isPlaying)
        stopPlayBack();
    else if (isPlayingFile)
        stopRecordingFilePlaying();

    playBackStartTime = juce::Time::getMillisecondCounterHiRes() * 0.001;
    isPlaying = true;
    nextEventIndex = 0;
    startTimer(1);
    return 1;
}

void MidiRecordPlayer::stopPlayBack()
{
    isPlaying = false;
    notifyFunction();
    if (auto midiOutShared = midiOutputDevice.lock())
    {
        for (int channel = 1; channel <= 16; ++channel)
            midiOutShared->sendMessageNow(juce::MidiMessage::allNotesOff(channel));
    }

    stopTimer();
}

void MidiRecordPlayer::startRecordingFilePlaying()
{
    if (isPlayingFile)
    {
        stopRecordingFilePlaying();
    }
    else if (isPlaying)
        stopPlayBack();
    isPlayingFile = true;
    nextEventFileIndex = 0;
    playBackStartTime = juce::Time::getMillisecondCounterHiRes() * 0.001;
    startTimer(1);
}

void MidiRecordPlayer::stopRecordingFilePlaying()
{
    isPlayingFile = false;
    if (auto midiOutShared = midiOutputDevice.lock())
    {
        for (int channel = 1; channel <= 16; ++channel)
        {
            midiOutShared->sendMessageNow(juce::MidiMessage::allNotesOff(channel));
        }
    }

    stopTimer();
}

void MidiRecordPlayer::handleIncomingMessage(const juce::MidiMessage& message)
{
    if (!isRecording)
        return;

    juce::MidiMessage newMessage = remapChannel(message);

    double now = juce::Time::getMillisecondCounterHiRes() * 0.001;
    RecordedEvent ev{ newMessage,now - recordStartTime };

    const SpinLock::ScopedLockType sl(lock);
    allEventsPlayed.push_back(ev);
}

void MidiRecordPlayer::timerCallback()
{
    if (!isPlaying && !isPlayingFile)
    {
        return;
    }
    double elapsedTime = juce::Time::getMillisecondCounterHiRes() * 0.001- playBackStartTime;

    if (isPlaying)
    {
        while (nextEventIndex < allEventsPlayed.size() && allEventsPlayed[nextEventIndex].timeFromStart <= elapsedTime)
        {
            if (auto midiOutShared = midiOutputDevice.lock())
            {
                if (isPlaying)
                    midiOutShared->sendMessageNow(allEventsPlayed[nextEventIndex].message);
                nextEventIndex++;
            }
        }
        if (nextEventIndex >= allEventsPlayed.size())
            stopPlayBack();
    }

    else if (isPlayingFile)
    {
        while (nextEventFileIndex < allEventsPlayedFile.size() && allEventsPlayedFile[nextEventFileIndex].timeFromStart <= elapsedTime)
        {
            if (auto midiOutShared = midiOutputDevice.lock())
            {
                if (isPlayingFile)
                    midiOutShared->sendMessageNow(allEventsPlayedFile[nextEventFileIndex].message);
                nextEventFileIndex++;
            }
        }
        if (nextEventFileIndex >= allEventsPlayedFile.size())
            stopRecordingFilePlaying();
    }

}

void MidiRecordPlayer::setOutputDevice(std::weak_ptr<juce::MidiOutput> outputDev)
{
    midiOutputDevice = outputDev;
}

void MidiRecordPlayer::setProgarmNumber(int newProgram, const juce::String& choice)
{
    if (choice.toLowerCase() == "left")
        programLeftHand = newProgram;
    else if (choice.toLowerCase() == "right")
        programRightHand = newProgram;
}

void MidiRecordPlayer::setInitialProgram(int value)
{
    initialProgram = value;
}

void MidiRecordPlayer::setReverb(int value)
{
    reverb = value;
}

bool MidiRecordPlayer::getIsRecording()
{
    return isRecording == true;
}

bool MidiRecordPlayer::getIsPlaying()
{
    return isPlaying == true;
}

bool MidiRecordPlayer::saveRecordingToFile(const juce::File& fileToSaveTo, juce::String& errorMsg, double tempo)
{
    if (allEventsPlayed.size() <= 1)
    {
        errorMsg = "No recorded events to save!";
        return false;
    }
    //applyPresetFunction();
    //maybe mutex unti

    juce::MidiFile midiFile;
    const int ticksPerQuarterNote = 960;
    midiFile.setTicksPerQuarterNote(ticksPerQuarterNote);

    juce::MidiMessageSequence sequence;

    const double secondsPerBeat = 60.0 / tempo;
    const double ticksPerSecond = ticksPerQuarterNote / secondsPerBeat;
    const int microsecondsPerQuarterNote = static_cast<int>(60000000 / tempo);

    juce::MidiMessageSequence tempoSequence;
    tempoSequence.addEvent(juce::MidiMessage::tempoMetaEvent(microsecondsPerQuarterNote), 0);

    midiFile.addTrack(tempoSequence);
    //TODO to fix recording with these hardcoded messages sent
    /*
    sequence.addEvent(juce::MidiMessage::programChange(1, program), 0);
    sequence.addEvent(juce::MidiMessage::controllerEvent(1, 91, 80), 0);
    sequence.addEvent(juce::MidiMessage::controllerEvent(1, 74, 100), 0);
    sequence.addEvent(juce::MidiMessage::controllerEvent(1, 11, reverb), 0);
    sequence.addEvent(juce::MidiMessage::controllerEvent(1, 93, 70), 0);
    */

    for (const auto& event : allEventsPlayed)
    {
        int tickPosition = static_cast<int>(event.timeFromStart * ticksPerSecond);
        sequence.addEvent(event.message, tickPosition);
    }

    midiFile.addTrack(sequence);

    juce::FileOutputStream outputStream(fileToSaveTo);
    if (!outputStream.openedOk())
    {
        errorMsg = "Failed to open file for saving!";
        return false;
    }
    midiFile.writeTo(outputStream);
    return true;
}

bool MidiRecordPlayer::parseRecordingFromFile(const juce::File& fileToParse, juce::String& errorMsg)
{
    if (!fileToParse.existsAsFile())
    {
        errorMsg = "Selected file does not exist.";
        return false;
    }

    juce::FileInputStream inputStream(fileToParse);
    if (!inputStream.openedOk())
    {
        errorMsg = "Failed to open file for reading.";
        return false;
    }

    juce::MidiFile midiFile;
    if (!midiFile.readFrom(inputStream))
    {
        errorMsg = "Failed to parse MIDI data.";
        return false;
    }

    midiFile.convertTimestampTicksToSeconds();
    allEventsPlayedFile.clear();
    const int numTracks = midiFile.getNumTracks();
    for (int trackIndex = 0; trackIndex < numTracks; ++trackIndex)
    {
        auto* sequence = midiFile.getTrack(trackIndex);
        if (sequence == nullptr)
            continue;

        for (int i = 0; i < sequence->getNumEvents(); ++i)
        {
            const auto& eventHolder = sequence->getEventPointer(i);
            if (eventHolder != nullptr)
            {
                const auto& msg = eventHolder->message;
                double timeFromStart = msg.getTimeStamp();
                allEventsPlayedFile.push_back(RecordedEvent{ msg, timeFromStart });
            }
        }
    }
    std::sort(allEventsPlayedFile.begin(), allEventsPlayedFile.end(),
        [](const RecordedEvent& a, const RecordedEvent& b)
        {
            return a.timeFromStart < b.timeFromStart;
        });

    return true;
}

int MidiRecordPlayer::getSizeRecorded()
{
    return allEventsPlayed.size();
}

int MidiRecordPlayer::getProgramLeftHand()
{
    return programLeftHand;
}

int MidiRecordPlayer::getProgramRightHand()
{
    return programRightHand;
}

std::vector<RecordedEvent>& MidiRecordPlayer::getAllRecordedEvents()
{
    return this->allEventsPlayed;
}

juce::MidiMessage MidiRecordPlayer::remapChannel(const juce::MidiMessage& message)
{
    int newChannel = message.getChannel();

    if (message.getChannel() == 1)
        newChannel = 14;
    else if (message.getChannel() == 16)
        newChannel = 15;

    if (message.isNoteOn())
        return juce::MidiMessage::noteOn(newChannel, message.getNoteNumber(), message.getVelocity());

    if (message.isNoteOff())
        return juce::MidiMessage::noteOff(newChannel, message.getNoteNumber());

    if (message.isController())
        return juce::MidiMessage::controllerEvent(newChannel, message.getControllerNumber(), message.getControllerValue());

    if (message.isProgramChange())
        return juce::MidiMessage::programChange(newChannel, message.getProgramChangeNumber());

    if (message.isPitchWheel())
        return juce::MidiMessage::pitchWheel(newChannel, message.getPitchWheelValue());

    return message;
}
