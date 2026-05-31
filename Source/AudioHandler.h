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

struct ChannelDSP
{
    float expression      = 1.0f;
    float reverbMix       = 0.0f;
    float chorusMix       = 0.0f;
    float tremoloDepth    = 0.0f;
    float tremoloPhase    = 0.0f;
    float delayMix        = 0.0f;
    float distortionDrive = 0.0f;
    float randomModDepth  = 0.0f;
    float randomModSmoothed = 0.0f;

    juce::dsp::StateVariableTPTFilter<float> filter;
    juce::Reverb                             reverb;
    juce::Reverb::Parameters                 reverbParams;
    juce::dsp::Chorus<float>                 chorus;

    std::vector<float> delayLineL, delayLineR;
    int    delayWritePos  = 0;
    int    delayReadOffset = 0;
    double sampleRate     = 44100.0;

    juce::Random rng;

    void prepare(double sr, int blockSize);
    void updateCC(int ccNumber, int value);
    void process(juce::AudioBuffer<float>& buffer, int numSamples);
};

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

    std::function<void()> onSfzLoadStart;
    std::function<void()> onSfzLoadComplete;

private:
    std::atomic<int> pendingLoads { 0 };
    MidiHandler& midiHandler;
    sfzero::Synth sfzSynths[16];
    juce::AudioFormatManager formatManager;
    double currentSampleRate = 44100.0;

    juce::AudioBuffer<float> tempBuffer;
    float channelGains[16];
    float channelPans[16];
    ChannelDSP channelDSP[16];

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioHandler)
};
