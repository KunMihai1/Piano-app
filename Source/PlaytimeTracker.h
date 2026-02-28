/*
  ==============================================================================

    PlaytimeTracker.h
    Created: 28 Feb 2026 3:15:21am
    Author:  Kisuke

  ==============================================================================
*/

#pragma once

#include "JuceHeader.h"

class PlaytimeTracker : private juce::Timer
{
public:
    PlaytimeTracker(std::function<void()> onMinuteElapsed);


private:
    void timerCallback() override;

    std::function<void()> callbackFunction;
};