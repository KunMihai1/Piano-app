/*
  ==============================================================================

    SynthSound.h
    Created: 23 May 2025 3:26:07pm
    Author:  Kisuke

  ==============================================================================
*/

#pragma once
#include "JuceHeader.h"

class SynthSound : public juce::SynthesiserSound
{
public:
    bool appliesToNote(int) override { return true; }
    bool appliesToChannel(int) override { return true; }
};