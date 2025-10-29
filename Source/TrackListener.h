/*
  ==============================================================================

    TrackListener.h
    Created: 10 Jul 2025 2:42:48am
    Author: Kisuke

  ==============================================================================
*/

#pragma once

/**
 * @class TrackListener
 * @brief Interface for objects that want to receive updates about track playback settings.
 *
 * Implementers of this interface can be notified whenever a track's playback settings
 * (volume or instrument) change.
 */
class TrackListener
{
public:
    /**
     * @brief Called when a track's playback settings are updated.
     * @param channel The MIDI channel of the track being updated.
     * @param newVolume The new volume level for the track.
     * @param newInstrument The new instrument assigned to the track.
     *
     * Implement this method to handle changes in playback settings, such as updating the UI
     * or sending MIDI messages.
     */
    virtual void updatePlaybackSettings(int channel, int newVolume, int newInstrument) = 0;

    /**
     * @brief Virtual destructor.
     */
    virtual ~TrackListener() = default;
};
