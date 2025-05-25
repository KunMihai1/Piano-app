/*
  ==============================================================================

    temporaryNotificationUI.h
    Created: 25 May 2025 2:56:51am
    Author:  Kisuke

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

class TemporaryMessage: public juce::Component, private juce::Timer
{
public:
    using FinishedCallback = std::function<void()>;

    TemporaryMessage(const juce::String& message="");

    void paint(juce::Graphics& g) override;

    void resized() override;

    void setFinishedCallBack(FinishedCallback callBack);

    void restartTimer();

    void updateText(const juce::String& newText);

    void setCurrentAlpha(float alpha);

private:
    void timerCallback() override;
    float currentAlpha = 1.0f;

    juce::Label labelToShow;
    FinishedCallback finishedCallback;
};