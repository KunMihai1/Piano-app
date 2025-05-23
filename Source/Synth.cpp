/*
  ==============================================================================

    Synth.cpp
    Created: 23 May 2025 2:28:51pm
    Author:  Kisuke

  ==============================================================================
*/

#include "SynthVoice.h"
#include "Synth.h"
#include "SynthSound.h"

MySynth::MySynth(const std::string& samplePath)
{
    /*
    sampleFile = juce::File{ samplePath };
    juce::AudioFormatManager formatManager;
    formatManager.registerBasicFormats();
    reader.reset(formatManager.createReaderFor(sampleFile));

    if (reader)
    {
        createSynthReverb();
    }
    */
    for (int i = 0; i < 8; ++i)
        addVoice(new SynthVoice());

    addSound(new SynthSound());

}

void MySynth::createSynthReverb()
{
    for (int i = 0; i < 8; i++)
        addVoice(new juce::SamplerVoice());

    addSound(new juce::SamplerSound("default",
        *reader,
        juce::BigInteger().setRange(0,128,true) ,
        60,    // root MIDI note
        0.1,   // attack time
        0.1,   // release time
        reader->lengthInSamples / reader->sampleRate));// max sample length
}
