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
    PlaytimeTracker(std::function<void(int sec)> onTimeElapsed, int batchInterval);

    int getAccumlatedSeconds();

private:
    void timerCallback() override;

    int accumulatedSeconds = 0;

    int startTimerInSec = 60;

    int batchIntervalSeconds = 0;

    juce::Time lastTickTime;

    std::function<void(int sec)> callbackFunction;
};