/*
  ==============================================================================

    PlaytimeTracker.cpp
    Created: 28 Feb 2026 3:15:21am
    Author:  Kisuke

  ==============================================================================
*/

#include "PlaytimeTracker.h"

PlaytimeTracker::PlaytimeTracker(std::function<void(int sec)> onTimeElapsed, int batchIntervalSec=300): callbackFunction{onTimeElapsed}, batchIntervalSeconds{batchIntervalSec}
{
    lastTickTime = juce::Time::getCurrentTime();
    startTimer(startTimerInSec*1000);
    
}

int PlaytimeTracker::getAccumlatedSeconds()
{
    return accumulatedSeconds;
}

void PlaytimeTracker::timerCallback()
{
    auto now = juce::Time::getCurrentTime();
    int deltaSec = static_cast<int>((now.toMilliseconds() - lastTickTime.toMilliseconds()) / 1000);
    lastTickTime = now;

    accumulatedSeconds += deltaSec;

    if (callbackFunction && accumulatedSeconds>=300)
    {
        callbackFunction(accumulatedSeconds);
        accumulatedSeconds = 0;
    }
}
