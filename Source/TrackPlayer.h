/*
  ==============================================================================

    TrackPlayer.h
    Created: 7 Jul 2025 9:02:50pm
    Author:  Kisuke

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "TrackEntry.h"

class MultipleTrackPlayer: private juce::HighResolutionTimer
{
public:
    struct TrackPlaybackData {
        const juce::MidiMessageSequence* sequence=nullptr;
        int nextEventIndex=0;
    };

    MultipleTrackPlayer(juce::MidiOutput* out);
    
    void setTracks(const std::vector<TrackEntry>& newTracks);

    void stop();

    void start();

    

private:
    void hiResTimerCallback() override;

    juce::MidiOutput* outputDevice=nullptr;
    std::vector<TrackPlaybackData> tracks;
    std::vector<int> eventIndices;
    double startTime = 0.0;
};