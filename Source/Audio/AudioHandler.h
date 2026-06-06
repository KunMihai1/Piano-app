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
    float expression           = 1.0f;
    float reverbMix            = 0.0f;
    float chorusMix            = 0.0f;
    float tremoloDepth         = 0.0f;
    float tremoloPhase         = 0.0f;
    float tremoloPhaseInc      = 0.0f;   // cached: 2π*5/sampleRate
    float delayMix             = 0.0f;
    float distortionDrive      = 0.0f;
    float distortionNormFactor = 1.0f;   // cached: 1/tanh(drive)
    float randomModDepth       = 0.0f;
    float randomModSmoothed    = 0.0f;
    int   filterCutoffCC       = 127;    // raw CC74 value; 127 = neutral (no filtering)
    int   filterResonanceCC    = 0;      // raw CC71 value; 0 = neutral

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
    std::function<void(int channelMask)> onNoSfzForChannels;

private:
    std::atomic<int>  pendingLoads      { 0 };
    std::atomic<int>  noSfzChannelMask  { 0 };
    std::atomic<bool> noSfzNotifyPending { false };

    MidiHandler& midiHandler;
    sfzero::Synth sfzSynths[16];
    juce::AudioFormatManager formatManager;
    double currentSampleRate = 44100.0;

    std::atomic<bool> channelHasSfz[16];
    juce::String      loadedSfzPath[16]; // message-thread only

    juce::AudioBuffer<float> tempBuffer;
    float channelGains[16];
    float channelPans[16];
    ChannelDSP channelDSP[16];

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioHandler)
};
