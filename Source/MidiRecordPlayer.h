/*
  ==============================================================================

    MidiRecordPlayer.h
    Created: 25 May 2025 1:21:59am
    Author:  Kisuke

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "MidiHandler.h"
#include "MidiHandlerAbstractSubject.h"

class MidiRecordPlayer : private juce::Timer, public MidiHandlerListener
{
public:
    std::function<void()> notifyFunction;

    MidiRecordPlayer(juce::MidiOutput* midiOut=nullptr);

    void startRecording();

    void stopRecording();

    bool startPlayBack();

    void stopPlayBack();

    void handleIncomingMessage(const juce::MidiMessage& message) override;

    void timerCallback() override;

    void setOutputDevice(juce::MidiOutput* outputDev);
    
    void setProgarmNumber(int newProgram);

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
    int program;
};
