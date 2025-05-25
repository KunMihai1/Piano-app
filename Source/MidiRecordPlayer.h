/*
  ==============================================================================

    MidiRecordPlayer.h
    Created: 25 May 2025 1:21:59am
    Author:  Kisuke

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

class MidiRecordPlayer : private juce::Timer
{
public:
    MidiRecordPlayer(juce::MidiOutput* midiOut);

    void startRecording();

    void stopRecording();

    void startPlayBack();

    void stopPlayBack();

    void processIncomingMessage(juce::MidiMessage& message);

    void timerCallback() override;

    void setOutputDevice(juce::MidiOutput* outputDev);

private:

    struct RecordedEvent {
        juce::MidiMessage message;
        double timeFromStart;
    };

    bool isRecording = false;
    bool isPlaying = false;
    double recordStartTime=0;
    double playBackStartTime = 0;
    int nextEventIndex = 0;

    juce::MidiOutput* midiOutputDevice=nullptr;
    std::vector<RecordedEvent> allEventsPlayed;
};
