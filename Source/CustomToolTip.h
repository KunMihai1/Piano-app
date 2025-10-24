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
 * @brief A custom tooltip component that displays a track name with a styled label.
 *
 * @details
 * The tooltip shows a semi-transparent black rounded rectangle with
 * white text. The size of the component is automatically determined
 * based on the length of the text plus padding.
 */
class TrackNameToolTip: public juce::Component
{
public:

    /**
     * @brief Constructs a tooltip displaying the given text.
     *
     * @param text The text to display inside the tooltip.
     *
     * @details
     * Initializes the label with font size, colours, and justification.
     * Automatically sizes the component to fit the text with padding.
     */
    TrackNameToolTip(const juce::String& text);

     /**
     * @brief Paints the tooltip background.
     *
     * @param g The JUCE Graphics context used for drawing.
     *
     * @details
     * Draws a semi-transparent black rounded rectangle behind the label.
     */
    void paint(juce::Graphics& g) override;


    /**
     * @brief Updates the bounds of the internal label when the component is resized.
     *
     * @details
     * The label is inset slightly from the component bounds to provide padding.
     */
    void resized() override;

private:
    juce::Label label;
};
