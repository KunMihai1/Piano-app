/*
  ==============================================================================

    CustomToolTip.cpp
    Created: 9 Jul 2025 6:27:48pm
    Author:  Kisuke

  ==============================================================================
*/

#include "CustomToolTip.h"

CustomToolTip::CustomToolTip(const juce::String& text)
{
    label.setFont(14.0f);
    label.setText(text, juce::dontSendNotification);
    label.setColour(juce::Label::backgroundColourId, juce::Colours::black);
    label.setColour(juce::Label::textColourId, juce::Colours::white);
    label.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(label);

    int textWidth = label.getFont().getStringWidth(text);
    int textHeight = (int)label.getFont().getHeight();

    int padding = 5;

    setSize(textWidth + padding * 2, textHeight + padding);
}

void CustomToolTip::paint(juce::Graphics& g)
{
    g.setColour(juce::Colours::black.withAlpha(0.8f));
    g.fillRoundedRectangle(getLocalBounds().toFloat(), 5.0f);
}

void CustomToolTip::setNewText(const juce::String& newText)
{
    label.setText(newText, juce::dontSendNotification);
}

void CustomToolTip::resized()
{
    label.setBounds(getLocalBounds().reduced(10, 5));
}
