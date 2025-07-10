/*
  ==============================================================================

    SubjectInterface.h
    Created: 10 Jul 2025 3:02:19am
    Author:  Kisuke

  ==============================================================================
*/
#pragma once
#include <JuceHeader.h>
#include "TrackListener.h"

class Subject
{
public:
    void addListener(TrackListener* listener);
    void removeListener(TrackListener* listener);
    void notify(int channel, int newVolume=-1, int newInstrument=-1);

private:
    juce::ListenerList<TrackListener> listeners;

};