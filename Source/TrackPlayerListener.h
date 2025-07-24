/*
  ==============================================================================

    TrackPlayerListener.h
    Created: 24 Jul 2025 4:15:03am
    Author:  Kisuke

  ==============================================================================
*/

#pragma once

class TrackPlayerListener
{
public:
    
    virtual void updateCurrentRowBasedOnTime(double currentTime) = 0;

    virtual ~TrackPlayerListener() = default;
};