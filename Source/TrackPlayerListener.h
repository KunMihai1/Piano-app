/*
  ==============================================================================

    TrackPlayerListener.h
    Created: 24 Jul 2025 4:15:03am
    Author: Kisuke

  ==============================================================================
*/

#pragma once

/**
 * @class TrackPlayerListener
 * @brief Interface for objects that want to respond to track playback updates.
 *
 * Classes implementing this listener will be notified of the current playback
 * position and can update their UI (e.g., highlight current row in a track view)
 * or internal state accordingly.
 */
class TrackPlayerListener
{
public:
    /**
     * @brief Called to update the current playback row based on elapsed time.
     *
     * @param currentTime The current playback time in seconds.
     *
     * Implementing classes should update their internal representation of
     * the playback position, such as highlighting a note or row in a piano roll
     * or tracker view.
     */
    virtual void updateCurrentRowBasedOnTime(double currentTime) = 0;



    /** Virtual destructor for safe cleanup in derived classes. */
    virtual ~TrackPlayerListener() = default;
};


class TrackPlayerListenerModifyStateObjects {
public:
    virtual void updateObjects() = 0;

    virtual ~TrackPlayerListenerModifyStateObjects() = default;
};