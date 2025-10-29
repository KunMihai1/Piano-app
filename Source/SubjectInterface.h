/*
  ==============================================================================

    SubjectInterface.h
    Created: 10 Jul 2025 3:02:19am
    Author: Kisuke

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#include "TrackListener.h"
#include "trackListComponentListener.h"
#include "TrackPlayerListener.h"

/**
 * @class Subject
 * @brief Generic publisher class that manages listeners and notifies them of events.
 *
 * This class uses the Observer pattern. It stores a list of listeners of type ListenerType,
 * and provides methods to notify them of different events, such as playback changes,
 * track list updates, and MIDI playback progress.
 *
 * @tparam ListenerType The type of listener objects that must implement the expected callback methods.
 */
template <typename ListenerType>
class Subject
{
public:
    /**
     * @brief Adds a listener to receive notifications.
     * @param listener Pointer to a listener of type ListenerType.
     */
    void addListener(ListenerType* listener)
    {
        listeners.add(listener);
    }

    /**
     * @brief Removes a listener from receiving notifications.
     * @param listener Pointer to the listener to remove.
     */
    void removeListener(ListenerType* listener)
    {
        listeners.remove(listener);
    }

    /**
     * @brief Notifies all listeners about playback settings changes.
     * @param channel MIDI channel that changed.
     * @param newVolume New volume value.
     * @param newInstrument New instrument value.
     *
     * Calls the `updatePlaybackSettings(channel, newVolume, newInstrument)` method on all listeners.
     */
    void notify(int channel, int newVolume, int newInstrument)
    {
        listeners.call([=](ListenerType& listener)
            {
                listener.updatePlaybackSettings(channel, newVolume, newInstrument);
            });
    }

    /**
     * @brief Notifies all listeners that a track is being added.
     *
     * Calls `updateUIbeforeAnyLoadingCase()` on all listeners to allow them to prepare the UI.
     */
    void notifyAddingToTrackList()
    {
        listeners.call([=](ListenerType& listener)
            {
                listener.updateUIbeforeAnyLoadingCase();
            });
    }

    /**
     * @brief Notifies all listeners about the current playback time.
     * @param currentTime Current playback time in seconds.
     *
     * Calls `updateCurrentRowBasedOnTime(currentTime)` on all listeners.
     */
    void notifyMidPlay(double currentTime)
    {
        listeners.call([=](ListenerType& listener)
            {
                listener.updateCurrentRowBasedOnTime(currentTime);
            });
    }

private:
    juce::ListenerList<ListenerType> listeners; /**< Internal list of listeners */
};
