/*
  ==============================================================================

    TrackEntry.h
    Created: 7 Jul 2025 9:19:32pm
    Author:  Kisuke

  ==============================================================================
*/

#pragma once

enum class TrackType {Melodic,Percussion};

struct TrackEntry
{
    juce::String folderName;
    juce::Uuid uuid;
    juce::File file;
    int trackIndex = 0;
    juce::String displayName;
    int instrumentAssociated=-1;
    double volumeAssociated = 0.0f;
    double originalBPM = 0.0f;

    juce::MidiMessageSequence sequence;
    juce::MidiMessageSequence originalSequenceTicks;
    TrackType type = TrackType::Melodic;

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

namespace TrackTypeConversion
{
    inline juce::String toString(TrackType type)
    {
        if (type == TrackType::Melodic) return "melodic";
        else if (type == TrackType::Percussion) return "percussion";
        return "unknown";
    }

    inline TrackType fromString(const juce::String& str)
    {
        if (str.compareIgnoreCase("percussion") == 0)
            return TrackType::Percussion;
        else return TrackType::Melodic;
    }
}
