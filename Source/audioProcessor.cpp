/*
  ==============================================================================

    audioProcessor.cpp
    Created: 23 May 2025 3:15:57am
    Author:  Kisuke

  ==============================================================================
*/

#include "audioProcessor.h"

AudioProcessorMIDIhandler::AudioProcessorMIDIhandler(MidiHandler& midiHandlerReference) : reverbSynth{ "ok.wav" }, midiHandler{midiHandlerReference}
{
    juce::Reverb::Parameters reverbParams;
    reverbParams.roomSize = 0.5f;
    reverbParams.wetLevel = 0.3f;
    reverbParams.dryLevel = 0.7f;
    reverbParams.width = 1.0f;
    reverbParams.damping = 0.5f;
    reverb.setParameters(reverbParams);
}

void AudioProcessorMIDIhandler::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    reverbSynth.setCurrentPlaybackSampleRate(sampleRate);
    reverb.setSampleRate(sampleRate);
}

void AudioProcessorMIDIhandler::releaseResources()
{
}

void AudioProcessorMIDIhandler::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    DBG("PROCESS BLOCK RUNNING!\n");
    
    juce::MidiBuffer handlerMidi;

    midiHandler.getNextMidiBlock(handlerMidi, 0, buffer.getNumSamples());

    midiMessages.addEvents(handlerMidi, 0, buffer.getNumSamples(), 0);

    for (const auto metadata : midiMessages)
    {
        if (metadata.getMessage().isNoteOn())
            DBG("PROCESSBLOCK GOT NOTE ON: " << metadata.getMessage().getNoteNumber());
    }

   // reverbSynth.renderNextBlock(buffer, midiMessages, 0, buffer.getNumSamples());

    //reverb.processStereo(buffer.getWritePointer(0), buffer.getWritePointer(1), buffer.getNumSamples());
}

const juce::String AudioProcessorMIDIhandler::getName() const
{
    return juce::String();
}

bool AudioProcessorMIDIhandler::acceptsMidi() const
{
    return true;
}

bool AudioProcessorMIDIhandler::producesMidi() const
{
    return false;
}

double AudioProcessorMIDIhandler::getTailLengthSeconds() const
{
    return 0.0;
}

bool AudioProcessorMIDIhandler::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    auto input = layouts.getMainInputChannelSet();
    auto output = layouts.getMainOutputChannelSet();

    DBG("Input channels: " << input.size());
    DBG("Output channels: " << output.size());

    // Accept only no input or stereo input, and stereo output
    bool inputOk = input.isDisabled() || input == juce::AudioChannelSet::stereo();
    bool outputOk = output == juce::AudioChannelSet::stereo();

    return inputOk && outputOk;
}

void AudioProcessorMIDIhandler::getStateInformation(juce::MemoryBlock& destData)
{
}

void AudioProcessorMIDIhandler::setStateInformation(const void* data, int sizeInBytes)
{
}

juce::AudioProcessorEditor* AudioProcessorMIDIhandler::createEditor()
{
    return nullptr;
}

bool AudioProcessorMIDIhandler::hasEditor() const
{
    return false;
}

int AudioProcessorMIDIhandler::getNumPrograms()
{
    return 0;
}

int AudioProcessorMIDIhandler::getCurrentProgram()
{
    return 0;
}

void AudioProcessorMIDIhandler::setCurrentProgram(int index)
{
}

const juce::String AudioProcessorMIDIhandler::getProgramName(int index)
{
    return juce::String();
}

void AudioProcessorMIDIhandler::changeProgramName(int index, const juce::String& newName)
{
}
