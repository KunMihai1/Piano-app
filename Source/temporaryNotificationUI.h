/*
  ==============================================================================

    temporaryNotificationUI.h
    Created: 25 May 2025 2:56:51am
    Author:  Kisuke

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>


/**
 * @class TemporaryMessage
 * @brief UI component that displays a fading temporary notification message.
 *
 * This component renders a short-lived message on screen with a fade-out animation.
 * After the fade is complete, a user-provided callback is invoked to allow cleanup
 * or removal from the UI hierarchy.
 *
 * The fading behavior is driven by a timer that gradually reduces the component's
 * alpha value.
 */
class TemporaryMessage: public juce::Component, private juce::Timer
{
public:
    using FinishedCallback = std::function<void()>;


    /**
     * @brief Creates a temporary notification using a fading background label.
     *
     * The message will appear immediately at full opacity and will gradually
     * fade out using a timed animation. Once fully faded, an optional callback
     * function (set via setFinishedCallBack()) can trigger cleanup logic.
     *
     * @param message Text to be displayed inside the notification component.
     */
    TemporaryMessage(const juce::String& message="");

    /**
     * @brief Draws a rounded background rectangle with the current fade alpha value.
     * @param g Graphics context used for rendering.
     */
    void paint(juce::Graphics& g) override;

    /**
     * @brief Ensures the internal label fits the component area.
     */
    void resized() override;

    /**
     * @brief Sets a callback that is triggered once the fade animation finishes.
     * @param callBack Function to invoke after the message disappears.
     */
    void setFinishedCallBack(FinishedCallback callBack);

    /**
     * @brief Restarts the fade-out animation from full opacity.
     */
    void restartTimer();

    /**
     * @brief Updates the temporary message text while visible.
     * @param newText New text to display in the notification.
     */
    void updateText(const juce::String& newText);

    /**
     * @brief Manually updates the transparency level (0.0–1.0).
     * @param alpha New alpha value for the fade progress.
     */
    void setCurrentAlpha(float alpha);

private:
    /**
     * @brief Handles the fade-out progress and triggers callback upon completion.
     *
     * Runs every timer tick, decreasing visibility. Once fully transparent,
     * the component stops the timer and calls the completion callback if set.
     */
    void timerCallback() override;
    float currentAlpha = 1.0f;

    juce::Label labelToShow;
    FinishedCallback finishedCallback;
};
