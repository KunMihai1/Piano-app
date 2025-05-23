/*
  ==============================================================================

    SynthVoice.h
    Created: 23 May 2025 3:22:16pm
    Author:  Kisuke

  ==============================================================================
*/

#pragma once

#include "JuceHeader.h"
#include "SynthSound.h"

class SynthVoice : public juce::SynthesiserVoice
{
public:
    bool canPlaySound(juce::SynthesiserSound* sound) override
    {
        return dynamic_cast<SynthSound*> (sound) != nullptr;
    }

    void startNote(int midiNoteNumber, float velocity, juce::SynthesiserSound*, int /*currentPitchWheelPosition*/) override
    {
        DBG("NOTE ON: " << midiNoteNumber);
        currentAngle = 0.0;
        level = velocity;
        angleDelta = juce::MathConstants<double>::twoPi * juce::MidiMessage::getMidiNoteInHertz(midiNoteNumber) / getSampleRate();
    }

    void stopNote(float /*velocity*/, bool allowTailOff) override
    {
        clearCurrentNote();
        angleDelta = 0.0;
    }

    void pitchWheelMoved(int) override {}
    void controllerMoved(int, int) override {}

    void renderNextBlock(juce::AudioBuffer<float>& outputBuffer, int startSample, int numSamples) override
    {
        DBG("SynthVoice::renderNextBlock running");
        while (--numSamples >= 0)
        {
            float sample = (float)(std::sin(currentAngle) * level);
            currentAngle += angleDelta;

            for (int channel = 0; channel < outputBuffer.getNumChannels(); ++channel)
                outputBuffer.addSample(channel, startSample, sample);

            ++startSample;
        }
    }

private:
    double currentAngle = 0.0;
    double angleDelta = 0.0;
    float level = 0.0f;
};
