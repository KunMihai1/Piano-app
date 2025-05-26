/*
  ==============================================================================

    MidiRecordPlayer.cpp
    Created: 25 May 2025 1:21:59am
    Author:  Kisuke

  ==============================================================================
*/

#include "MidiRecordPlayer.h"

MidiRecordPlayer::MidiRecordPlayer(juce::MidiOutput* midiOut): midiOutputDevice{midiOut}, program{0}
{
}

void MidiRecordPlayer::startRecording()
{
    allEventsPlayed.clear();
    allEventsPlayedFile.clear();
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
    if (midiOutputDevice)
    {
        for (int channel = 1; channel <= 16; ++channel)
            midiOutputDevice->sendMessageNow(juce::MidiMessage::allNotesOff(channel));
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
    if (midiOutputDevice)
    {
        for (int channel = 1; channel <= 16; ++channel)
        {
            midiOutputDevice->sendMessageNow(juce::MidiMessage::allNotesOff(channel));
        }
    }

    stopTimer();
}

void MidiRecordPlayer::handleIncomingMessage(const juce::MidiMessage& message)
{
    if (!isRecording)
        return;
    double now = juce::Time::getMillisecondCounterHiRes() * 0.001;
    RecordedEvent ev{ message,now - recordStartTime };
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
            if (midiOutputDevice)
            {
                if (isPlaying)
                    midiOutputDevice->sendMessageNow(allEventsPlayed[nextEventIndex].message);
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
            if (midiOutputDevice)
            {
                if (isPlayingFile)
                    midiOutputDevice->sendMessageNow(allEventsPlayedFile[nextEventFileIndex].message);
                nextEventFileIndex++;
            }
        }
        if (nextEventFileIndex >= allEventsPlayedFile.size())
            stopRecordingFilePlaying();
    }

}

void MidiRecordPlayer::setOutputDevice(juce::MidiOutput* outputDev)
{
    midiOutputDevice = outputDev;
}

void MidiRecordPlayer::setProgarmNumber(int newProgram)
{
    program = newProgram;
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

    sequence.addEvent(juce::MidiMessage::programChange(1, program), 0);
    sequence.addEvent(juce::MidiMessage::controllerEvent(1, 91, 80), 0);
    sequence.addEvent(juce::MidiMessage::controllerEvent(1, 74, 100), 0);
    sequence.addEvent(juce::MidiMessage::controllerEvent(1, 11, reverb), 0);
    sequence.addEvent(juce::MidiMessage::controllerEvent(1, 93, 70), 0);

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

std::vector<RecordedEvent>& MidiRecordPlayer::getAllRecordedEvents()
{
    return this->allEventsPlayed;
}