/*
  ==============================================================================

    TrackEntry.h
    Created: 7 Jul 2025 9:19:32pm
    Author:  Kisuke

  ==============================================================================
*/

/**
 * @file TrackEntry.h
 * @brief Defines structures for MIDI track information and related metadata.
 * 
 * This file includes definitions for TrackEntry, MidiChangeInfo, TrackType, 
 * and helper functions for TrackType conversions.
 */

#pragma once


/**
 * @enum TrackType
 * @brief Represents the type of MIDI track.
 */
enum class TrackType {Melodic,Percussion};


/**
 * @struct MidiChangeInfo
 * @brief Stores information about a MIDI event change.
 */
struct MidiChangeInfo
{
    int oldNumber = -1;
    double oldTimeStamp = 0.0;
    int oldVelocity = -1;

    int newNumber = -1;
    double newTimeStamp = 0.0;
    int newVelocity = -1;
};


struct TrackSection
{
    juce::String name;
    int startIndex = 0;
    int endIndex = 0;
};

/**
 * @struct TrackEntry
 * @brief Stores all relevant information for a MIDI track.
 */
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

    std::vector<TrackSection> sections;
    std::unordered_map<juce::String, std::unordered_map<int, MidiChangeInfo>> styleChangesMap;


    double originalTPQN=960.0;
    double normalizedTPQN=960.0;

    bool hasBeenNormalized = false;

    double trackStartOffsetInSeconds = 0.0;

    juce::MidiMessageSequence sequence;
    juce::MidiMessageSequence originalSequenceTicks;
    TrackType type = TrackType::Melodic;


    /**
     * @brief Returns the display name, or filename if empty.
     * @return Display name string.
     */
    juce::String getDisplayName() const
    {
        return displayName.isNotEmpty()
            ? displayName
            : file.getFileNameWithoutExtension();
    }

    /**
     * @brief Returns the unique ID string for this track.
     * @return Unique identifier string.
     */
    juce::String getUniqueID() const
    {
        return uuid.toString();
    }

    /**
     * @brief Generates a new UUID.
     * @return Newly generated UUID.
     */
    static juce::Uuid generateUUID()
    {
        return juce::Uuid();
    }

};


/**
 * @namespace TrackTypeConversion
 * @brief Helper functions for converting TrackType to/from string.
 */
namespace TrackTypeConversion
{
    /**
     * @brief Converts a TrackType to string.
     * @param type TrackType value.
     * @return String representation ("melodic" or "percussion").
     */
    inline juce::String toString(TrackType type)
    {
        if (type == TrackType::Melodic) return "melodic";
        else if (type == TrackType::Percussion) return "percussion";
        return "unknown";
    }

    /**
     * @brief Converts a string to TrackType.
     * @param str String representation of track type.
     * @return Corresponding TrackType value.
     */
    inline TrackType fromString(const juce::String& str)
    {
        if (str.compareIgnoreCase("percussion") == 0)
            return TrackType::Percussion;
        else return TrackType::Melodic;
    }
}
