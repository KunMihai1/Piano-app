/*
  ==============================================================================

    PlaytimeTracker.cpp
    Created: 28 Feb 2026 3:15:21am
    Author:  Kisuke

  ==============================================================================
*/

#include "PlaytimeTracker.h"

PlaytimeTracker::PlaytimeTracker(std::function<void()> onMinuteElapsed): callbackFunction{onMinuteElapsed}
{
    startTimer(60000);
}

void PlaytimeTracker::timerCallback()
{
    if (callbackFunction)
        callbackFunction();
}
