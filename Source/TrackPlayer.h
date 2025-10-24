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


/**
 * @class MultipleTrackPlayer
 * @brief Handles playback of multiple MIDI tracks with per-track settings.
 *
 * The player can start, stop, and update playback of multiple tracks, adjusting
 * BPM, volume, instruments, and time signatures dynamically. It works with a
 * juce::MidiOutput device.
 */
class MultipleTrackPlayer: private juce::HighResolutionTimer, public TrackListener, public Subject<TrackPlayerListener>
{
public:
    struct TrackPlaybackData {
        int filteredSequenceIndex = -1;
        int nextEventIndex = 0;
    };

    /**
     * @brief Callback triggered whenever the playback position is updated.
     *
     * This is used by external components, such as a custom beat display, to synchronize
     * visual feedback with the current playback. The callback receives the current elapsed
     * playback time in beats (adjusted for the time signature).
     */
    std::function<void(double)> onElapsedUpdate;



    /**
   * @brief Constructs a MultipleTrackPlayer.
   * @param out Weak pointer to the MIDI output device.
   */
    MultipleTrackPlayer(std::weak_ptr<juce::MidiOutput> out);
    
    void changingMidPlaySettings(int volume, int instrument);

    /**
     * @brief Sets the tracks to be played and filters MIDI sequences for playback.
     * @param newTracks Vector of TrackEntry objects representing tracks.
     */
    void setTracks(const std::vector<TrackEntry>& newTracks);


    /**
     * @brief Updates the current BPM for playback.
     * @param newBPM New beats per minute.
     */
    void setCurrentBPM(int newBPM);

    /**
     * @brief Stops all playback and resets timers.
     */
    void stop();

    /**
     * @brief Starts playback of all tracks.
     */
    void start();

    /**
     * @brief Updates volume and instrument settings for a specific MIDI channel.
     * @param channel MIDI channel (1-16).
     * @param newVolume New volume value (0-127). -1 if unchanged.
     * @param newInstrument New instrument/program number. -1 if unchanged.
     */
    void updatePlaybackSettings(int channel, int newVolume=-1, int newInstrument=-1) override;

    /**
     * @brief Destructor for MultipleTrackPlayer.
     */
    ~MultipleTrackPlayer();

    /**
     * @brief Applies a BPM change during playback, updating timestamps accordingly.
     * @param newBPM New beats per minute.
     */
    void applyBPMchangeDuringPlayback(double newBPM);

    /**
     * @brief Finds the index of the next event in a sequence based on the current time.
     * @param seq MIDI sequence to search.
     * @param currentTime Current playback time in seconds.
     * @return Index of the next event to play.
     */
    int findNextEventIndex(const juce::MidiMessageSequence& seq, double currentTime);

    
    /**
     * @brief Syncs playback settings for all tracks (volume, instrument).
     */
    void syncPlaybackSettings();

    /**
     * @brief Sets a new MIDI output device.
     * @param newOutput Weak pointer to a new juce::MidiOutput device.
     */
    void setDeviceOutputTrackPlayer(std::weak_ptr<juce::MidiOutput> newOutput);

    /**
     * @brief Sets the denominator of the time signature.
     * @param newDenominator Denominator value (e.g., 4 for 4/4).
     */
    void setTimeSignatureDenominator(int newDenominator);

    /**
     * @brief High-resolution timer callback used to send MIDI events at precise times.
     *
     * This function handles playback progression, sending MIDI events, updating
     * elapsed time, and notifying listeners of the current playback position.
     */
    void setTimeSignatureNumerator(int newNumerator);

private:
    /**
     * @brief High-resolution timer callback used to send MIDI events at precise times.
     *
     * This function handles playback progression, sending MIDI events, updating
     * elapsed time, and notifying listeners of the current playback position.
     */
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
