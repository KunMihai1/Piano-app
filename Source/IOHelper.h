/*
  ==============================================================================

    TrackIOHelper.h
    Created: 17 Jul 2025 3:29:02pm
    Author:  Kisuke

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "TrackEntry.h"
#include "PlayBackSettingsCustomComponent.h"


/**
 * @brief Helper class for handling file and folder paths for the application.
 */
class IOHelper
{
public:
    /**
     * @brief Returns the application-specific folder for storing data.
     *
     * @param name Folder name
     * @return juce::File representing the folder in user application data directory.
     */
    static juce::File getFolder(const juce::String& name);

    /**
     * @brief Returns a file inside the application data folder.
     *
     * @param name File name
     * @return juce::File representing the requested file.
     */
    static juce::File getFile(const juce::String& name);

private:

};


/**
 * @brief Helper class for saving and loading MIDI tracks and sequences.
 */
class TrackIOHelper
{
public:

    /**
     * @brief Saves a set of grouped tracks to a JSON file.
     *
     * @param file File to save to
     * @param groupedTracks Map of folder names to deque of TrackEntry objects
     *
     * @details
     * Each folder contains tracks and any associated style changes.
     * Converts TrackEntry objects to a structured JSON format.
     */
    static void saveToFile(const juce::File& file, const std::unordered_map<juce::String, std::deque<TrackEntry>>& groupedTracks);

    /**
     * @brief Loads grouped tracks from a JSON file.
     *
     * @param file File to load from
     * @param groupedTracks Map to populate with TrackEntry objects
     * @param groupedTrackKeys Vector to populate with folder names in order
     *
     * @details
     * Parses JSON, reads MIDI files, populates TrackEntry objects including sequences,
     * original BPM, TPQN, percussion detection, and style changes.
     */
    static void loadFromFile(const juce::File& file,
        std::unordered_map<juce::String, std::deque<TrackEntry>>& groupedTracks,
        std::vector<juce::String>& groupedTrackKeys);


    /**
     * @brief Checks whether a MIDI sequence contains percussion notes (channel 10).
     *
     * @param sequence Pointer to a MidiMessageSequence
     * @return True if at least one note is on channel 10, false otherwise.
     */
    static bool foundPercussion(const juce::MidiMessageSequence* sequence);


    /**
     * @brief Retrieves the original BPM from a MIDI file's first tempo meta-event.
     *
     * @param midiFile MIDI file to inspect
     * @return BPM value; defaults to 120 if not found.
     */
    static double getOriginalBpmFromFile(const juce::MidiFile& midiFile);


    /**
     * @brief Converts all tick timestamps in a sequence to seconds.
     *
     * @param seq Sequence to convert
     * @param tpqn Ticks per quarter note
     * @param bpm Tempo in beats per minute
     *
     * @details Updates the time stamps in-place and sorts the sequence.
     */
    static void convertSequenceTicksToSeconds(juce::MidiMessageSequence& seq, double tpqn, double bpm);


    /**
     * @brief Applies a set of changes to a MIDI message sequence.
     *
     * @param sequence Sequence to modify
     * @param changesMap Map of event index to MidiChangeInfo
     *
     * @details Updates note number, velocity, and timestamp for note-on/off events.
     */
    static void applyChangesToASequence(juce::MidiMessageSequence& sequence, const std::unordered_map<int, MidiChangeInfo>& changesMap);


    /**
     * @brief Converts ticks to seconds using TPQN and BPM.
     */
    static double ticksToSeconds(double ticks, double tpqn, double bpm);


    /**
     * @brief Converts seconds to ticks using TPQN and BPM.
     */
    static double secondsToTicks(double seconds, double tpqn, double bpm);

private:
};


/**
 * @brief Helper class for saving and loading playback settings.
 */
class PlaybackSettingsIOHelper
{
public:
    /**
     * @brief Saves playback settings to a JSON file.
     *
     * @param file File to save to
     * @param settings PlayBackSettings object to save
     *
     * @details Uses VID and PID combination as a key for per-device settings.
     */
    static void saveToFile(const juce::File& file, const PlayBackSettings& settings);


    /**
     * @brief Loads playback settings for a specific device from a JSON file.
     *
     * @param file File to load from
     * @param VID Vendor ID of the device
     * @param PID Product ID of the device
     * @return PlayBackSettings object; returns default invalid values if not found.
     */
    static PlayBackSettings loadFromFile(const juce::File& file, const juce::String& VID, const juce::String& PID);

private:
    
};
