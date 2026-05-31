/*
  ==============================================================================

    AudioHandler.cpp
    Created: 30 May 2026
    Author:  Antigravity

  ==============================================================================
*/

#include "AudioHandler.h"

//==============================================================================
// ChannelDSP

void ChannelDSP::prepare(double sr, int blockSize)
{
    sampleRate = sr;

    juce::dsp::ProcessSpec spec;
    spec.sampleRate       = sr;
    spec.maximumBlockSize = static_cast<juce::uint32>(blockSize);
    spec.numChannels      = 2;

    filter.prepare(spec);
    filter.setType(juce::dsp::StateVariableTPTFilterType::lowpass);
    filter.setCutoffFrequency(20000.0f);
    filter.setResonance(0.707f);

    reverbParams          = juce::Reverb::Parameters{};
    reverbParams.roomSize = 0.5f;
    reverbParams.damping  = 0.5f;
    reverbParams.wetLevel = 0.0f;
    reverbParams.dryLevel = 1.0f;
    reverbParams.width    = 1.0f;
    reverb.setParameters(reverbParams);
    reverb.setSampleRate(sr);
    reverb.reset();

    chorus.prepare(spec);
    chorus.setRate(1.5f);
    chorus.setDepth(0.3f);
    chorus.setCentreDelay(15.0f);
    chorus.setFeedback(0.1f);
    chorus.setMix(0.0f);

    const int maxDelaySamples = static_cast<int>(sr * 0.5);
    delayLineL.assign(maxDelaySamples, 0.0f);
    delayLineR.assign(maxDelaySamples, 0.0f);
    delayWritePos   = 0;
    delayReadOffset = static_cast<int>(sr * 0.25);
    tremoloPhase    = 0.0f;
    randomModSmoothed = 0.0f;
}

void ChannelDSP::updateCC(int ccNumber, int value)
{
    const float norm = value / 127.0f;
    switch (ccNumber)
    {
        case 11: // expression
            expression = norm;
            break;

        case 71: // resonance → filter Q
            filter.setResonance(0.5f + norm * 9.5f);
            break;

        case 74: // brightness → LP filter cutoff (100 Hz – 20 kHz, log)
        {
            const float hz = 100.0f * std::pow(200.0f, norm);
            filter.setCutoffFrequency(juce::jlimit(20.0f, 20000.0f, hz));
            break;
        }

        case 80: // distortion drive
            distortionDrive = norm;
            break;

        case 91: // reverb wet amount
            reverbMix = norm * 0.85f;
            break;

        case 92: // tremolo depth
            tremoloDepth = norm;
            break;

        case 93: // chorus mix
            chorusMix = norm;
            chorus.setMix(norm);
            break;

        case 94: // delay — mix + time
            delayMix = norm * 0.8f;
            if (!delayLineL.empty())
            {
                int offset = static_cast<int>(sampleRate * (0.05 + norm * 0.35));
                delayReadOffset = juce::jlimit(1, static_cast<int>(delayLineL.size()) - 1, offset);
            }
            break;

        case 95: // random mod depth
            randomModDepth = norm;
            break;

        // CC1 (vibrato): passed through to sfzero — works if the SFZ file defines modwheel
        // CC73/75/72 (attack/decay/release): sfzero reads these from the SFZ region at load
        //   time, not from CC, so they have no effect here
        // CC76 (filterTrack): key-tracking concept doesn't map to post-render DSP
        default: break;
    }
}

void ChannelDSP::process(juce::AudioBuffer<float>& buffer, int numSamples)
{
    // tempBuffer is always prepared with 2 channels
    float* left  = buffer.getWritePointer(0);
    float* right = buffer.getWritePointer(1);

    // --- LP filter (CC74 brightness + CC71 resonance) ---
    {
        juce::dsp::AudioBlock<float> block(buffer.getArrayOfWritePointers(),
                                           (size_t)buffer.getNumChannels(),
                                           (size_t)numSamples);
        filter.process(juce::dsp::ProcessContextReplacing<float>(block));
    }

    // --- Distortion (CC80) ---
    if (distortionDrive > 0.005f)
    {
        const float drive      = 1.0f + distortionDrive * 15.0f;
        const float normFactor = 1.0f / std::tanh(drive);
        for (int i = 0; i < numSamples; ++i)
        {
            left[i]  = std::tanh(left[i]  * drive) * normFactor;
            right[i] = std::tanh(right[i] * drive) * normFactor;
        }
    }

    // --- Chorus (CC93) ---
    if (chorusMix > 0.005f)
    {
        juce::dsp::AudioBlock<float> block(buffer.getArrayOfWritePointers(),
                                           (size_t)buffer.getNumChannels(),
                                           (size_t)numSamples);
        chorus.process(juce::dsp::ProcessContextReplacing<float>(block));
    }

    // --- Reverb (CC91) ---
    if (reverbMix > 0.005f)
    {
        reverbParams.wetLevel = reverbMix;
        reverbParams.dryLevel = 1.0f;
        reverb.setParameters(reverbParams);
        reverb.processStereo(left, right, numSamples);
    }

    // --- Tremolo (CC92) — 5 Hz amplitude LFO ---
    if (tremoloDepth > 0.005f)
    {
        const float phaseInc = juce::MathConstants<float>::twoPi * 5.0f
                               / static_cast<float>(sampleRate);
        for (int i = 0; i < numSamples; ++i)
        {
            const float mod = 1.0f - tremoloDepth * 0.5f * (1.0f + std::sin(tremoloPhase));
            left[i]  *= mod;
            right[i] *= mod;
            tremoloPhase += phaseInc;
            if (tremoloPhase >= juce::MathConstants<float>::twoPi)
                tremoloPhase -= juce::MathConstants<float>::twoPi;
        }
    }

    // --- Delay (CC94) ---
    if (delayMix > 0.005f && !delayLineL.empty())
    {
        const int delaySize = static_cast<int>(delayLineL.size());
        for (int i = 0; i < numSamples; ++i)
        {
            const int readPos = (delayWritePos - delayReadOffset + delaySize) % delaySize;
            delayLineL[delayWritePos] = left[i];
            delayLineR[delayWritePos] = right[i];
            left[i]  += delayMix * delayLineL[readPos];
            right[i] += delayMix * delayLineR[readPos];
            delayWritePos = (delayWritePos + 1) % delaySize;
        }
    }

    // --- Random Mod (CC95) — smoothed noise amplitude flutter ---
    if (randomModDepth > 0.005f)
    {
        for (int i = 0; i < numSamples; ++i)
        {
            randomModSmoothed = 0.998f * randomModSmoothed + 0.002f * rng.nextFloat();
            const float mod = 1.0f - randomModDepth * randomModSmoothed;
            left[i]  *= mod;
            right[i] *= mod;
        }
    }

    // --- Expression (CC11) ---
    if (expression < 0.999f)
        buffer.applyGain(0, numSamples, expression);
}

//==============================================================================
// AudioHandler

AudioHandler::AudioHandler(MidiHandler& mh) : midiHandler(mh)
{
    formatManager.registerBasicFormats();
    for (int i = 0; i < 16; ++i)
    {
        for (int v = 0; v < 16; ++v)
            sfzSynths[i].addVoice(new sfzero::Voice());
        channelGains[i] = 1.0f;
        channelPans[i]  = 0.5f;
    }
}

AudioHandler::~AudioHandler()
{
}

void AudioHandler::audioDeviceAboutToStart (juce::AudioIODevice* device)
{
    currentSampleRate = device->getCurrentSampleRate();
    const int bufferSize = device->getCurrentBufferSizeSamples();
    tempBuffer.setSize(2, bufferSize, false, true);

    for (int i = 0; i < 16; ++i)
    {
        sfzSynths[i].setCurrentPlaybackSampleRate(currentSampleRate);
        channelDSP[i].prepare(currentSampleRate, bufferSize);
    }
}

void AudioHandler::audioDeviceStopped()
{
}

void AudioHandler::audioDeviceIOCallbackWithContext (const float* const* inputChannelData, int numInputChannels,
                                                     float* const* outputChannelData, int numOutputChannels,
                                                     int numSamples, const juce::AudioIODeviceCallbackContext& context)
{
    juce::ignoreUnused(inputChannelData, numInputChannels, context);

    for (int i = 0; i < numOutputChannels; ++i)
        if (outputChannelData[i] != nullptr)
            juce::FloatVectorOperations::clear(outputChannelData[i], numSamples);

    if (numOutputChannels == 0)
        return;

    juce::MidiBuffer incomingMidi;
    midiHandler.getNextMidiBlock(incomingMidi, 0, numSamples);

    // Update per-channel state from incoming CCs before rendering
    for (const auto metadata : incomingMidi)
    {
        const auto msg = metadata.getMessage();
        if (msg.isController())
        {
            const int ch  = msg.getChannel() - 1;
            const int cc  = msg.getControllerNumber();
            const int val = msg.getControllerValue();
            if (ch >= 0 && ch < 16)
            {
                if      (cc == 7)  channelGains[ch] = val / 127.0f;
                else if (cc == 10) channelPans[ch]  = val / 127.0f;
                else               channelDSP[ch].updateCC(cc, val);
            }
        }
    }

    // tempBuffer is always 2-channel; reuse its allocation if large enough
    tempBuffer.setSize(2, numSamples, false, false, true);

    juce::AudioBuffer<float> mainBuffer(outputChannelData, numOutputChannels, numSamples);

    for (int channel = 1; channel <= 16; ++channel)
    {
        juce::MidiBuffer channelMidi;
        for (const auto metadata : incomingMidi)
        {
            const auto message = metadata.getMessage();
            if (message.getChannel() == channel)
                channelMidi.addEvent(message, metadata.samplePosition);
        }

        tempBuffer.clear();
        sfzSynths[channel - 1].renderNextBlock(tempBuffer, channelMidi, 0, numSamples);

        channelDSP[channel - 1].process(tempBuffer, numSamples);

        const float gain = channelGains[channel - 1];
        const float pan  = channelPans[channel - 1];

        if (numOutputChannels >= 2)
        {
            const float leftGain  = gain * std::cos(pan * juce::MathConstants<float>::halfPi);
            const float rightGain = gain * std::sin(pan * juce::MathConstants<float>::halfPi);
            mainBuffer.addFrom(0, 0, tempBuffer, 0, 0, numSamples, leftGain);
            mainBuffer.addFrom(1, 0, tempBuffer, 1, 0, numSamples, rightGain);
        }
        else
        {
            mainBuffer.addFrom(0, 0, tempBuffer, 0, 0, numSamples, gain);
        }
    }
}

void AudioHandler::loadSfz(const juce::File& sfzFile, int midiChannel)
{
    if (!sfzFile.existsAsFile() || midiChannel < 1 || midiChannel > 16)
        return;

    ++pendingLoads;
    if (onSfzLoadStart)
        onSfzLoadStart();

    juce::Thread::launch([this, sfzFile, midiChannel]()
    {
        auto* sound = new sfzero::Sound(sfzFile);
        sound->loadRegions();
        sound->loadSamples(&formatManager);
        sfzSynths[midiChannel - 1].clearSounds();
        sfzSynths[midiChannel - 1].addSound(sound);

        if (--pendingLoads == 0)
            juce::MessageManager::callAsync([this]() {
                if (onSfzLoadComplete) onSfzLoadComplete();
            });
    });
}
