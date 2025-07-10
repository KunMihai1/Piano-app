/*
  ==============================================================================

    TrackListener.h
    Created: 10 Jul 2025 2:42:48am
    Author:  Kisuke

  ==============================================================================
*/

#pragma once

class TrackListener
{
public:
    virtual void updatePlaybackSettings(int channel, int newVolume, int newInstrument)=0;

    virtual ~TrackListener() = default;
};