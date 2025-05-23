/*
  ==============================================================================

    Synth.h
    Created: 23 May 2025 2:28:51pm
    Author:  Kisuke

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>

class MySynth : public juce::Synthesiser
{
public:
    MySynth(const std::string& samplePath);

private:
    void createSynthReverb();

    juce::File sampleFile;
    std::unique_ptr<juce::AudioFormatReader> reader;
};