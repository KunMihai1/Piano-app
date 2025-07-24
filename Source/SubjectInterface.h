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
#include "trackListComponentListener.h"
#include "TrackPlayerListener.h"

template <typename ListenerType>
class Subject
{
public:
    void addListener(ListenerType* listener)
    {
        listeners.add(listener);
    }
    void removeListener(ListenerType* listener)
    {
        listeners.remove(listener);
    }
    void notify(int channel, int newVolume, int newInstrument)
    {
        listeners.call([=](ListenerType& listener)
            {
                listener.updatePlaybackSettings(channel, newVolume, newInstrument);
            });
    }

    void notifyAddingToTrackList()
    {
        listeners.call([=](ListenerType& listener)
            {
                listener.updateUIbeforeAnyLoadingCase();
            });
    }

    void notifyMidPlay(double currentTime)
    {
        listeners.call([=](ListenerType& listener)
            {
                listener.updateCurrentRowBasedOnTime(currentTime);
            });
    }

private:
    juce::ListenerList<ListenerType> listeners;

};