/*
  ==============================================================================

    CustomBeatBar.cpp
    Created: 15 Jul 2025 5:59:41pm
    Author:  Kisuke

  ==============================================================================
*/

#include "CustomBeatBar.h"

BeatBar::BeatBar(): numerator{4}, denominator{4}
{

}

void BeatBar::paint(juce::Graphics& g)
{
    auto area = getLocalBounds().toFloat();

    float beatWidth = area.getWidth() / numerator;
    float height = area.getHeight();

    g.fillAll(juce::Colours::black);

    int currentBeat = static_cast<int>(std::floor(currentBeatsElapsed)) % numerator;

    float cornerSize = juce::jmin(beatWidth, height) * 0.3f;

    for (int i = 0; i < numerator; ++i)
    {
        juce::Rectangle<float> beatRect(area.getX() + i * beatWidth, area.getY(), beatWidth, height);
        if (i == currentBeat)
            g.setColour(juce::Colours::yellow.withAlpha(0.8f));
        else
            g.setColour(juce::Colours::dimgrey);

        g.fillRoundedRectangle(beatRect, cornerSize);

        g.setColour(juce::Colours::blanchedalmond.withAlpha(0.3f));
        g.drawRoundedRectangle(beatRect, cornerSize, 1.0f);
    }

}

void BeatBar::resized()
{

}

int BeatBar::getNumerator() const
{
    return this->numerator;
}

int BeatBar::getDenominator() const 
{
    return this->denominator;
}

void BeatBar::setNumerator(int newNumerator)
{
    this->numerator = newNumerator;
}

void BeatBar::setDenominator(int newDenominator)
{
    this->denominator = newDenominator;
}

void BeatBar::setCurrentBeatsElapsed(double beatsElapsed)
{
    this->currentBeatsElapsed = beatsElapsed;
    repaint();
}