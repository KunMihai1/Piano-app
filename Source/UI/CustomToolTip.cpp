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

    updateSizeForText(text);
}

void CustomToolTip::updateSizeForText(const juce::String& text)
{
    const int textWidth  = juce::GlyphArrangement::getStringWidthInt(label.getFont(), text);
    const int textHeight = (int) label.getFont().getHeight();

    // resized() lays the label out with getLocalBounds().reduced(hInset, vInset), so the tooltip must
    // be that much larger than the text or the inset would shrink the label below the string width and
    // clip an edge glyph. The small extra guards against getStringWidthInt under-measuring.
    const int hInset = 10, vInset = 5;   // must match resized()
    setSize(textWidth + hInset * 2 + 4, textHeight + vInset * 2 + 2);
}

void CustomToolTip::paint(juce::Graphics& g)
{
    g.setColour(juce::Colours::black.withAlpha(0.8f));
    g.fillRoundedRectangle(getLocalBounds().toFloat(), 5.0f);
}

void CustomToolTip::setNewText(const juce::String& newText)
{
    label.setText(newText, juce::dontSendNotification);
    updateSizeForText(newText);   // re-fit so a longer string doesn't clip
}

void CustomToolTip::resized()
{
    label.setBounds(getLocalBounds().reduced(10, 5));
}
