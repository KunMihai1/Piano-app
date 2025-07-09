/*
  ==============================================================================

    CustomToolTip.h
    Created: 9 Jul 2025 6:27:48pm
    Author:  Kisuke

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

class TrackNameToolTip: public juce::Component
{
public:
    TrackNameToolTip(const juce::String& text);

    void paint(juce::Graphics& g) override;

    void resized() override;

private:
    juce::Label label;
};