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
    juce::Uuid uuid;
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

    juce::String getUniqueID() const
    {
        return uuid.toString();
    }

    static juce::Uuid generateUUID()
    {
        return juce::Uuid();
    }

};