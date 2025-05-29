/*
  ==============================================================================

    audioProcessor.h
    Created: 23 May 2025 3:15:57am
    Author:  Kisuke

  ==============================================================================
*/

#pragma once
#include "Synth.h"
#include <JuceHeader.h>
#include "MidiHandler.h"


class AudioProcessorMIDIhandler : public juce::AudioProcessor
{
public:
    AudioProcessorMIDIhandler(MidiHandler& midiHandlerReference);
    ~AudioProcessorMIDIhandler() override {}

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

    void processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&) override;

    const juce::String getName() const override;
    bool acceptsMidi() const override;
    bool producesMidi() const override;
    double getTailLengthSeconds() const override;
    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram(int index) override;
    const juce::String getProgramName(int index) override;
    void changeProgramName(int index, const juce::String& newName) override;

private:
    MidiHandler& midiHandler;
    MySynth reverbSynth;
    juce::Reverb reverb;
};
