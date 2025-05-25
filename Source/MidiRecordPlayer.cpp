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
    recordStartTime = juce::Time::getMillisecondCounterHiRes() * 0.001;
    isRecording = true;
}

bool MidiRecordPlayer::stopRecording()
{
    if (isRecording == false)
        return 0;

    isRecording = false;
    return 1;
}

bool MidiRecordPlayer::startPlayBack()
{
    if (allEventsPlayed.empty())
    {
        notifyFunction();
        return 0;
    }
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
    if (!isPlaying)
    {
        return;
    }

    double elapsedTime = juce::Time::getMillisecondCounterHiRes() * 0.001- playBackStartTime;

    while (nextEventIndex < allEventsPlayed.size() && allEventsPlayed[nextEventIndex].timeFromStart <= elapsedTime)
    {
        if (midiOutputDevice)
        {
            midiOutputDevice->sendMessageNow(allEventsPlayed[nextEventIndex].message);
            nextEventIndex++;
        }
    }
    if (nextEventIndex >= allEventsPlayed.size())
        stopPlayBack();
}

void MidiRecordPlayer::setOutputDevice(juce::MidiOutput* outputDev)
{
    midiOutputDevice = outputDev;
}

void MidiRecordPlayer::setProgarmNumber(int newProgram)
{
    program = newProgram;
    if (midiOutputDevice)
        midiOutputDevice->sendMessageNow(juce::MidiMessage::programChange(1, newProgram));
}

std::vector<RecordedEvent>& MidiRecordPlayer::getAllRecordedEvents()
{
    return this->allEventsPlayed;
}
