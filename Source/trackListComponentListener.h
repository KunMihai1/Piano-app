/*
  ==============================================================================

    trackListComponentListener.h
    Created: 13 Jul 2025 12:28:41am
    Author:  Kisuke

  ==============================================================================
*/

/**
 * @class TrackListListener
 * @brief Interface for listening to track list events.
 *
 * Provides a callback to update the UI before any track loading occurs.
 * Classes implementing this interface must define the `updateUIbeforeAnyLoadingCase` method.
 */
class TrackListListener
{
public:
    /**
     * @brief Called before any track loading to update the UI.
     *
     * Implement this function to handle UI updates or preparation steps
     * before a track or multiple tracks are loaded.
     */
    virtual void updateUIbeforeAnyLoadingCase() = 0;

    /** @brief Virtual destructor for proper cleanup of derived classes. */
    virtual ~TrackListListener() = default;
};
