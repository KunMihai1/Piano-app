/*
  ==============================================================================

    trackListComponentListener.h
    Created: 13 Jul 2025 12:28:41am
    Author:  Kisuke

  ==============================================================================
*/

#pragma once

/**
 * @class TrackListListener
 * @brief Listener interface for components handling track list changes.
 *
 * Implement this interface to respond to updates in a track list UI or data model,
 * specifically when new tracks are added and visual or structural normalization
 * is required.
 */
class TrackListListener
{
public:

    /**
     * @brief Called when a new track is added to the track list.
     *
     * This should trigger UI normalization or layout updates
     * so the track list remains visually consistent and properly aligned.
     */
    virtual void normalizeAddingTrackCase() = 0;

    /** @brief Virtual destructor for safe polymorphic cleanup. */
    virtual ~TrackListListener() = default;
};
