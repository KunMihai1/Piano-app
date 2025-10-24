/*
  ==============================================================================

    TrackPlayerListener.h
    Created: 24 Jul 2025 4:15:03am
    Author:  Kisuke

  ==============================================================================
*/

#pragma once

/**
 * @file TrackPlayerListener.h
 * @brief Interface for objects that want to listen to track playback updates.
 * 
 * This interface allows implementing classes to receive updates on the current
 * playback position of a track, enabling synchronized UI updates or other logic.
 */


/**
 * @class TrackPlayerListener
 * @brief Listener interface for track playback events.
 */
class TrackPlayerListener
{
public:

    /**
     * @brief Called to update the current row or position based on playback time.
     * @param currentTime Current playback time in seconds.
     */
    virtual void updateCurrentRowBasedOnTime(double currentTime) = 0;

    /**
     * @brief Virtual destructor.
     */
    virtual ~TrackPlayerListener() = default;
};
