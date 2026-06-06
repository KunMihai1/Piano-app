/*
  ==============================================================================

    DisplayListener.h
    Created: 8 Apr 2026
    Author:  Kisuke

  ==============================================================================
*/

#pragma once
#include "PlayBackSettingsCustomComponent.h"

/**
 * @class DisplayListener
 * @brief Abstract interface to receive updates from the Display component.
 *
 * Classes implementing `DisplayListener` can receive notifications when
 * playback settings are changed.
 */
class DisplayListener {
public:

    /** @brief Destructor */
    virtual ~DisplayListener() = default;

    /**
     * @brief Called when playback settings change.
     * @param settings The new playback settings
     */
    virtual void playBackSettingsChanged(const PlayBackSettings& settings)=0;

    virtual void playBackSettingsTransposeChanged(int transposeValue) = 0;
};
