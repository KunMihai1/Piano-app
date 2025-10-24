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


/**
 * @class Subject
 * @brief Template class implementing the Observer pattern for notifying listeners.
 *
 * This class maintains a list of listeners and provides methods to add, remove,
 * and notify them about various events. It is designed to work with different types
 * of listeners that implement the required callback interface.
 *
 * @tparam ListenerType Type of the listener class.
 */
template <typename ListenerType>
class Subject
{
public:
    /**
     * @brief Adds a listener to the subject.
     * @param listener Pointer to the listener to be added.
     */
    void addListener(ListenerType* listener)
    {
        listeners.add(listener);
    }

    /**
     * @brief Removes a listener from the subject.
     * @param listener Pointer to the listener to be removed.
     */
    void removeListener(ListenerType* listener)
    {
        listeners.remove(listener);
    }

    /**
     * @brief Notifies all listeners about a playback settings update.
     * @param channel MIDI channel affected.
     * @param newVolume New volume value.
     * @param newInstrument New instrument/program number.
     */
    void notify(int channel, int newVolume, int newInstrument)
    {
        listeners.call([=](ListenerType& listener)
            {
                listener.updatePlaybackSettings(channel, newVolume, newInstrument);
            });
    }

    /**
     * @brief Notifies all listeners that a track is being added and requires normalization.
     */
    void notifyAddingToTrackList()
    {
        listeners.call([=](ListenerType& listener)
            {
                listener.normalizeAddingTrackCase();
            });
    }

    /**
     * @brief Notifies all listeners of the current playback time.
     * @param currentTime Current playback time in seconds.
     */
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
