#pragma once
#include <JuceHeader.h>
#include "AppColours.h"

class PlayScreenLookAndFeel : public juce::LookAndFeel_V4
{
public:
    PlayScreenLookAndFeel()
    {
        setColour(juce::TextButton::textColourOffId,        AppColours::titleText);
        setColour(juce::TextButton::textColourOnId,         AppColours::titleText);
        setColour(juce::TextButton::buttonColourId,         AppColours::accent2);
        setColour(juce::TextButton::buttonOnColourId,       AppColours::accent3);
        setColour(juce::ToggleButton::textColourId,         AppColours::titleText);
        setColour(juce::ToggleButton::tickColourId,         AppColours::accent3);
        setColour(juce::ToggleButton::tickDisabledColourId, AppColours::rowLabel);
    }

    void drawButtonBackground(juce::Graphics& g,
                               juce::Button& button,
                               const juce::Colour& backgroundColour,
                               bool isMouseOverButton,
                               bool isButtonDown) override
    {
        auto bounds = button.getLocalBounds().toFloat().reduced(0.5f);
        constexpr float cornerRadius = 6.0f;

        juce::Colour fill = AppColours::cardBg;
        if (isButtonDown)
            fill = fill.darker(0.15f);
        else if (isMouseOverButton)
            fill = fill.brighter(0.15f);

        g.setColour(fill);
        g.fillRoundedRectangle(bounds, cornerRadius);

        g.setColour(backgroundColour.withAlpha(isMouseOverButton ? 1.0f : 0.75f));
        g.drawRoundedRectangle(bounds, cornerRadius, 1.5f);
    }

    juce::Font getTextButtonFont(juce::TextButton&, int) override
    {
        return juce::Font(13.0f);
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PlayScreenLookAndFeel)
};
