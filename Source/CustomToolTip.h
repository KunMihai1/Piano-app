/*
  ==============================================================================

    CustomToolTip.h
    Created: 9 Jul 2025 6:27:48pm
    Author:  Kisuke

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

/**
 * @class TrackNameToolTip
 * @brief A simple tooltip component that displays a track name.
 */
class CustomToolTip: public juce::Component
{
public:
    /**
     * @brief Constructor.
     * @param text Text to display in the tooltip.
     */
    CustomToolTip(const juce::String& text);

    /**
     * @brief Paint the tooltip background.
     * @param g Graphics context to draw with.
     */
    void paint(juce::Graphics& g) override;

    void setNewText(const juce::String& newText);

    /**
     * @brief Called when the component is resized. Updates label bounds.
     */
    void resized() override;

private:
    juce::Label label; ///< Label showing the tooltip text
};
