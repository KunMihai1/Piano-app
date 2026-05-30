/*
  ==============================================================================

    AudioHandler.cpp
    Created: 30 May 2026
    Author:  Antigravity

  ==============================================================================
*/

#include "AudioHandler.h"

AudioHandler::AudioHandler(MidiHandler& mh) : midiHandler(mh)
{
    formatManager.registerBasicFormats();
    
    // Add voices to the synths (16 per channel)
    for (int i = 0; i < 16; ++i)
    {
        for (int v = 0; v < 16; ++v)
        {
            sfzSynths[i].addVoice(new sfzero::Voice());
        }
    }
}

AudioHandler::~AudioHandler()
{
}

void AudioHandler::audioDeviceIOCallbackWithContext (const float* const* inputChannelData, int numInputChannels,
                                                     float* const* outputChannelData, int numOutputChannels,
                                                     int numSamples, const juce::AudioIODeviceCallbackContext& context)
{
    juce::ignoreUnused(inputChannelData, numInputChannels, context);

    // Clear the output buffers
    for (int i = 0; i < numOutputChannels; ++i)
    {
        if (outputChannelData[i] != nullptr)
            juce::FloatVectorOperations::clear(outputChannelData[i], numSamples);
    }

    if (numOutputChannels == 0)
        return;

    // Get the accumulated MIDI messages from MidiHandler
    juce::MidiBuffer incomingMidi;
    midiHandler.getNextMidiBlock(incomingMidi, 0, numSamples);

    // Wrap the output channels into an AudioBuffer for the synthesizer
    juce::AudioBuffer<float> buffer(outputChannelData, numOutputChannels, numSamples);

    // Filter MIDI messages by channel and render each synth
    for (int channel = 1; channel <= 16; ++channel)
    {
        juce::MidiBuffer channelMidi;
        for (const auto metadata : incomingMidi)
        {
            auto message = metadata.getMessage();
            if (message.getChannel() == channel)
            {
                channelMidi.addEvent(message, metadata.samplePosition);
            }
        }
        sfzSynths[channel - 1].renderNextBlock(buffer, channelMidi, 0, numSamples);
    }
}

void AudioHandler::audioDeviceAboutToStart (juce::AudioIODevice* device)
{
    currentSampleRate = device->getCurrentSampleRate();
    for (int i = 0; i < 16; ++i)
        sfzSynths[i].setCurrentPlaybackSampleRate(currentSampleRate);
}

void AudioHandler::audioDeviceStopped()
{
}

void AudioHandler::loadSfz(const juce::File& sfzFile, int midiChannel)
{
    if (!sfzFile.existsAsFile() || midiChannel < 1 || midiChannel > 16)
        return;

    auto* sound = new sfzero::Sound(sfzFile);
    sound->loadRegions();
    sound->loadSamples(&formatManager);

    // Replace the current sound(s) with the new one
    sfzSynths[midiChannel - 1].clearSounds();
    sfzSynths[midiChannel - 1].addSound(sound);
}
