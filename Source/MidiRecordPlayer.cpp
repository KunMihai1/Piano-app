/*
  ==============================================================================

    MidiRecordPlayer.cpp
    Created: 25 May 2025 1:21:59am
    Author:  Kisuke

  ==============================================================================
*/

#include "MidiRecordPlayer.h"

MidiRecordPlayer::MidiRecordPlayer(juce::MidiOutput* midiOut): midiOutputDevice{midiOut}
{
}

void MidiRecordPlayer::startRecording()
{
    DBG("Started recording");
    allEventsPlayed.clear();
    recordStartTime = juce::Time::getMillisecondCounterHiRes() * 0.001;
    isRecording = true;
}

void MidiRecordPlayer::stopRecording()
{
    DBG("Stopped recording");
    isRecording = false;
}

void MidiRecordPlayer::startPlayBack()
{
    if (allEventsPlayed.empty())
        return;
    playBackStartTime = juce::Time::getMillisecondCounterHiRes() * 0.001;
    isPlaying = true;
    nextEventIndex = 0;
    startTimer(1);
}

void MidiRecordPlayer::stopPlayBack()
{
    isPlaying = false;
    stopTimer();
}

void MidiRecordPlayer::processIncomingMessage(juce::MidiMessage& message)
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
        return;

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
