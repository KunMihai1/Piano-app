/*
  ==============================================================================

    TrackEntry.h
    Created: 7 Jul 2025 9:19:32pm
    Author:  Kisuke

  ==============================================================================
*/

#pragma once

struct TrackEntry
{
    juce::File file;
    int trackIndex = 0;
    juce::String displayName;
    juce::MidiMessageSequence sequence;

    juce::String getDisplayName() const
    {
        return displayName.isNotEmpty()
            ? displayName
            : file.getFileNameWithoutExtension();
    }
};