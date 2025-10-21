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

class IOHelper
{
public:
    static juce::File getFolder(const juce::String& name);
    static juce::File getFile(const juce::String& name);

private:

};

class TrackIOHelper
{
public:
    static void saveToFile(const juce::File& file, const std::unordered_map<juce::String, std::deque<TrackEntry>>& groupedTracks);

    static void loadFromFile(const juce::File& file,
        std::unordered_map<juce::String, std::deque<TrackEntry>>& groupedTracks,
        std::vector<juce::String>& groupedTrackKeys);

    static bool foundPercussion(const juce::MidiMessageSequence* sequence);

    static double getOriginalBpmFromFile(const juce::MidiFile& midiFile);

    static void convertTicksToSeconds(juce::MidiFile& midiFile, double bpm);

    static void applyChangesToASequence(juce::MidiMessageSequence& sequence, const std::unordered_map<int, MidiChangeInfo>& changesMap);

private:
};

class PlaybackSettingsIOHelper
{
public:
    static void saveToFile(const juce::File& file, const PlayBackSettings& settings);

    static PlayBackSettings loadFromFile(const juce::File& file, const juce::String& VID, const juce::String& PID);

private:
    
};