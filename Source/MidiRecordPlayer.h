/*
  ==============================================================================

    MidiRecordPlayer.h
    Created: 25 May 2025 1:21:59am
    Author:  Kisuke

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "MidiHandler.h"
#include "MidiHandlerAbstractSubject.h"

/**
 * @struct RecordedEvent
 * @brief Stores a MIDI message with its timestamp relative to recording start.
 */
struct RecordedEvent {
    juce::MidiMessage message;   /**< The MIDI message */
    double timeFromStart;        /**< Time in seconds since recording started */
};

/**
 * @class MidiRecordPlayer
 * @brief Handles recording and playback of MIDI messages.
 *
 * This class can record incoming MIDI messages, play them back in real-time,
 * and save or load recordings from MIDI files. It also supports sending
 * recorded messages to a MIDI output device.
 */
class MidiRecordPlayer : private juce::Timer, public MidiHandlerListener
{
public:
    /** Callback invoked when playback stops or recording state changes */
    std::function<void()> notifyFunction;

    /** Callback to apply presets when starting recording */
    std::function<void()> applyPresetFunction;

    /** @brief Default constructor  */
    MidiRecordPlayer();

    /** @brief Constructor with an output device linked with the record player */
    MidiRecordPlayer(std::weak_ptr<juce::MidiOutput> out);

    /** @brief Starts recording incoming MIDI messages */
    void startRecording();

    /** @brief Stops recording; returns true if recording was active */
    bool stopRecording();

    /** @brief Starts playback of the recorded sequence; returns true if successful */
    bool startPlayBack();

    /** @brief Stops playback */
    void stopPlayBack();

    /** @brief Starts playback of a recording loaded from file */
    void startRecordingFilePlaying();

    /** @brief Stops playback of recording from file */
    void stopRecordingFilePlaying();

    /** @brief Handles incoming MIDI messages while recording */
    void handleIncomingMessage(const juce::MidiMessage& message) override;

    /** @brief Timer callback to send MIDI messages at correct playback times */
    void timerCallback() override;

    /** @brief Sets the MIDI output device */
    void setOutputDevice(std::weak_ptr<juce::MidiOutput> outputDev);

    /** @brief Sets program number for a hand ("left" or "right") */
    void setProgarmNumber(int newProgram, const juce::String& choice="");

    /** @brief Sets initial program number to be applied at recording start */
    void setInitialProgram(int value);

    /** @brief Sets reverb level */
    void setReverb(int value);

    /** @brief Returns true if currently recording */
    bool getIsRecording();

    /** @brief Returns true if currently playing */
    bool getIsPlaying();

    /** @brief Saves the recorded sequence to a MIDI file */
    bool saveRecordingToFile(const juce::File& fileToSaveTo, juce::String& errorMsg, double tempo = 120.0);

    /** @brief Loads a MIDI recording from a file */
    bool parseRecordingFromFile(const juce::File& fileToParse, juce::String& errorMsg);

    /** @brief Returns the number of recorded events */
    int getSizeRecorded();

    /** @brief Returns the left-hand program number */
    int getProgramLeftHand();

    /** @brief Returns the right-hand program number */
    int getProgramRightHand();

    /** @brief Returns all recorded events */
    std::vector<RecordedEvent>& getAllRecordedEvents();

    /** @brief Returns a message mapped to a different channel due to the program changes of a single channel that may affect the recording */
    juce::MidiMessage remapChannel(const juce::MidiMessage& message);

private:
    int initialProgram = 0;                      /**< Initial program number */
    int reverb = 50;                             /**< Reverb level */
    bool programChanged = false;                 /**< Whether program changed since recording */
    bool isRecording = false;                    /**< Recording state flag */
    bool isPlaying = false;                      /**< Playback state flag */
    bool isPlayingFile = false;                  /**< File playback state flag */

    double recordStartTime = 0;                  /**< Time when recording started */
    double playBackStartTime = 0;               /**< Time when playback started */
    int nextEventIndex = 0;                      /**< Next event index during playback */
    int nextEventFileIndex = 0;                  /**< Next event index for file playback */

    juce::SpinLock lock;

    std::weak_ptr<juce::MidiOutput> midiOutputDevice; /**< MIDI output device */
    std::vector<RecordedEvent> allEventsPlayed;       /**< Recorded events from live input */
    std::vector<RecordedEvent> allEventsPlayedFile;   /**< Events loaded from file */
    int programLeftHand, programRightHand;            /**< Hand program numbers */
};
