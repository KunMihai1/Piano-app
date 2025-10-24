/*
  ==============================================================================

    CustomBeatBar.h
    Created: 15 Jul 2025 5:59:41pm
    Author:  Kisuke

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>


/**
 * @brief A component that visually represents a beat bar, such as a
 *        progress indicator within a musical measure.
 */
class BeatBar: public juce::Component
{
public:

    /**
     * @brief Constructs a BeatBar with default time signature and no elapsed beats.
     */
    BeatBar();

    /**
     * @brief Draws the beat bar based on the current elapsed beats.
     * @param g Graphics context used for painting within the component bounds.
     */
    void paint(juce::Graphics& g) override;

    /**
     * @brief Called when the component’s size changes.
     *        Override to update layout or internal display positions.
     */
    void resized() override;

    /**
     * @brief Gets the current time signature numerator.
     * @return Number of beats per bar (e.g., 4 in 4/4 time).
     */
    int getNumerator() const;

    /**
     * @brief Gets the time signature denominator.
     * @return Beat unit (e.g., 4 means quarter note = 1 beat).
     */
    int getDenominator() const;

    /**
     * @brief Sets the time signature numerator.
     * @param newNumerator New number of beats per bar.
     * @note Does not clamp input — caller must ensure valid musical values.
     */
    void setNumerator(int newNumerator);

    /**
     * @brief Sets the time signature denominator.
     * @param newDenominator New beat unit value.
     * @note Does not clamp input — caller must ensure valid musical values.
     */
    void setDenominator(int newDenominator);

    /**
     * @brief Updates the visual progress based on beats elapsed in the current bar.
     * @param elapsedBeats Fractional beat position (e.g., 1.5 = halfway through beat 2)
     * @note Ensure elapsedBeats resets or wraps properly when a bar completes.
     */
    void setCurrentBeatsElapsed(double elapsedBeats);

private:
    int numerator;
    int denominator;
    double bpm;
    double currentTimeSeconds;
    double currentBeatsElapsed = 0.0;
};
