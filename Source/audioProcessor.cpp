/*
  ==============================================================================

    audioProcessor.cpp
    Created: 23 May 2025 3:15:57am
    Author:  Kisuke

  ==============================================================================
*/

#include "audioProcessor.h"

ReverbProcessor::ReverbProcessor()
{
    juce::Reverb::Parameters reverbParams;
    reverbParams.roomSize = 0.5f;
    reverbParams.wetLevel = 0.3f;
    reverbParams.dryLevel = 0.7f;
    reverbParams.width = 1.0f;
    reverbParams.damping = 0.5f;
    reverb.setParameters(reverbParams);
}

void ReverbProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    reverb.setSampleRate(sampleRate);
}

void ReverbProcessor::releaseResources()
{
}

void ReverbProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    reverb.processStereo(buffer.getWritePointer(0), buffer.getWritePointer(1), buffer.getNumSamples());
}

const juce::String ReverbProcessor::getName() const
{
    return juce::String();
}

bool ReverbProcessor::acceptsMidi() const
{
    return false;
}

bool ReverbProcessor::producesMidi() const
{
    return false;
}

double ReverbProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

bool ReverbProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    return false;
}

void ReverbProcessor::getStateInformation(juce::MemoryBlock& destData)
{
}

void ReverbProcessor::setStateInformation(const void* data, int sizeInBytes)
{
}

void ReverbProcessor::setReverbAmmount(float ammount)
{
    juce::Reverb::Parameters paramReverb = reverb.getParameters();
    paramReverb.wetLevel = juce::jlimit(0.0f, 1.0f, ammount);
    reverb.setParameters(paramReverb);
}
