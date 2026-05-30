/*
  ==============================================================================

    AudioHandler.h
    Created: 30 May 2026
    Author:  Antigravity

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "MidiHandler.h"

class AudioHandler : public juce::AudioIODeviceCallback
{
public:
    AudioHandler(MidiHandler& mh);
    ~AudioHandler() override;

    void audioDeviceIOCallbackWithContext (const float* const* inputChannelData, int numInputChannels,
                                           float* const* outputChannelData, int numOutputChannels,
                                           int numSamples, const juce::AudioIODeviceCallbackContext& context) override;
    void audioDeviceAboutToStart (juce::AudioIODevice* device) override;
    void audioDeviceStopped() override;

    void loadSfz(const juce::File& sfzFile, int midiChannel);

private:
    MidiHandler& midiHandler;
    sfzero::Synth sfzSynths[16];
    juce::AudioFormatManager formatManager;
    double currentSampleRate = 44100.0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioHandler)
};
