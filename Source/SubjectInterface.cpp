/*
  ==============================================================================

    SubjectInterface.cpp
    Created: 10 Jul 2025 3:02:19am
    Author:  Kisuke

  ==============================================================================
*/

#include "SubjectInterface.h"

void Subject::addListener(TrackListener* listener)
{
    listeners.add(listener);
}

void Subject::removeListener(TrackListener* listener)
{
    listeners.remove(listener);
}

void Subject::notify(int channel, int newVolume, int newInstrument)
{
    listeners.call([=](TrackListener& listener)
        {
            listener.updatePlaybackSettings(channel, newVolume, newInstrument);
        }
    );
}