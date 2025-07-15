/*
  ==============================================================================

    CustomBeatBar.h
    Created: 15 Jul 2025 5:59:41pm
    Author:  Kisuke

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>

class BeatBar: public juce::Component
{
public:
    BeatBar();

    void paint(juce::Graphics& g) override;
    void resized() override;
    int getNumerator() const;
    int getDenominator() const;
    void setNumerator(int newNumerator);
    void setDenominator(int newDenominator);
    void setCurrentBeatsElapsed(double elapsedBeats);

private:
    int numerator;
    int denominator;
    double currentBeatsElapsed = 0.0;
};