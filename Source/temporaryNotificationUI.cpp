/*
  ==============================================================================

    temporaryNotificationUI.cpp
    Created: 25 May 2025 2:56:51am
    Author:  Kisuke

  ==============================================================================
*/

#include "temporaryNotificationUI.h"

TemporaryMessage::TemporaryMessage(const juce::String& message)
{
    labelToShow.setText(message, juce::dontSendNotification);
    labelToShow.setColour(juce::Label::backgroundColourId, juce::Colours::black.withAlpha(0.7f));
    labelToShow.setColour(juce::Label::textColourId, juce::Colours::white);
    labelToShow.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(labelToShow);

    setSize(300, 50);
    startTimer(60);
}

void TemporaryMessage::paint(juce::Graphics& g)
{
    g.setColour(juce::Colours::black.withAlpha(currentAlpha));
    g.fillRoundedRectangle(getLocalBounds().toFloat(), 10.0f);
}

void TemporaryMessage::resized()
{
    labelToShow.setBounds(getLocalBounds());
}

void TemporaryMessage::setFinishedCallBack(FinishedCallback callBack)
{
    finishedCallback = std::move(callBack);
}

void TemporaryMessage::restartTimer()
{
    currentAlpha = 1.0f;
    startTimer(60);
}

void TemporaryMessage::updateText(const juce::String& newText)
{
    labelToShow.setText(newText, juce::dontSendNotification);
}

void TemporaryMessage::setCurrentAlpha(float alpha)
{
    currentAlpha = alpha;
}

void TemporaryMessage::timerCallback()
{
    currentAlpha -= 0.03f;

    if (currentAlpha <= 0.0f)
    {
        currentAlpha = 0.0f;
        stopTimer();
        if (finishedCallback)
            finishedCallback();
        return;
    }
    labelToShow.setColour(juce::Label::backgroundColourId, juce::Colours::black.withAlpha(currentAlpha));
    repaint();
}
