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
#include "TrackListener.h"
#include "TrackPlayerListener.h"
#include "SubjectInterface.h"

class MultipleTrackPlayer: private juce::HighResolutionTimer, public TrackListener, public Subject<TrackPlayerListener>
{
public:
    struct TrackPlaybackData {
        int filteredSequenceIndex = -1;
        int nextEventIndex = 0;
    };

    std::function<void(double)> onElapsedUpdate;


    MultipleTrackPlayer(std::weak_ptr<juce::MidiOutput> out);
    
    void changingMidPlaySettings(int volume, int instrument);

    void setTracks(const std::vector<TrackEntry>& newTracks);

    void setCurrentBPM(int newBPM);

    void stop();

    void start();

    void updatePlaybackSettings(int channel, int newVolume=-1, int newInstrument=-1) override;

    ~MultipleTrackPlayer();

    void applyBPMchangeDuringPlayback(double newBPM);

    int findNextEventIndex(const juce::MidiMessageSequence& seq, double currentTime);

    void syncPlaybackSettings();

    void setDeviceOutputTrackPlayer(std::weak_ptr<juce::MidiOutput> newOutput);

    void setTimeSignatureDenominator(int newDenominator);

    void setTimeSignatureNumerator(int newNumerator);

private:
    void hiResTimerCallback() override;
    std::vector<juce::MidiMessageSequence> filteredSequences;
    int baseChannelTrack = 2;
    std::vector<TrackEntry> currentTracks;

    std::weak_ptr<juce::MidiOutput> outputDevice;
    std::vector<TrackPlaybackData> tracks;
    std::vector<int> eventIndices;
    double startTime = 0.0;
    double currentElapsedTime;

    double currentBPM = 120.0;

    double lastKnownSequenceTime = 0.0;

    int timeSignatureDenominator = 4;
    int timeSignatureNumerator = 4;
    
};