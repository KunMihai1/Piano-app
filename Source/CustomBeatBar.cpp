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
    g.fillAll(juce::Colours::black);
    float height = area.getHeight();

    float beatsPerBar = static_cast<float>(numerator);
    float subBeatsPerBeat = static_cast<float>(denominator) / 4.0f;
    float totalSubdivisions = beatsPerBar * subBeatsPerBeat;
    float subBeatWidth = area.getWidth() / totalSubdivisions;

    double beatInBar = std::fmod(currentBeatsElapsed, beatsPerBar);
    if (beatInBar < 0) beatInBar += beatsPerBar;
    int currentSubdivision = static_cast<int>(std::floor(beatInBar * subBeatsPerBeat));

    float cornerSize = juce::jmin(subBeatWidth, height) * 0.3f;

    for (int i = 0; i < static_cast<int>(totalSubdivisions); ++i)
    {
        juce::Rectangle<float> beatRect(area.getX() + i * subBeatWidth, area.getY(), subBeatWidth, height);

        bool isCurrent = (i == currentSubdivision);

        if (isCurrent)
        {
            if (i == 0 && isPlayingCheck && isPlayingCheck())
                g.setColour(juce::Colours::red.withAlpha(0.85f));
            else
                g.setColour(juce::Colours::yellow.withAlpha(0.85f));
        }
        else
        {
            g.setColour(juce::Colours::dimgrey); 
        }

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