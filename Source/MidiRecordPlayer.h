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
 * @brief Represents a MIDI event and its timestamp relative to recording start.
 */
struct RecordedEvent {
    juce::MidiMessage message;
    double timeFromStart;
};


/**
 * @class MidiRecordPlayer
 * @brief Manages MIDI recording, playback, and file operations.
 *
 * Supports live recording from a MIDI device, playback of recorded events,
 * playback of MIDI files, and saving/loading recordings to/from MIDI files.
 * Provides callbacks for notifying external components when events occur.
 */
class MidiRecordPlayer : private juce::Timer, public MidiHandlerListener
{
public:
    std::function<void()> notifyFunction;
    std::function<void()> applyPresetFunction;

    /** 
     * @brief Default constructor. Initializes internal state.
     */
    MidiRecordPlayer();

    /**
     * @brief Constructs a player associated with a MIDI output device.
     * @param out Weak pointer to a JUCE MIDI output device.
     */
    MidiRecordPlayer(std::weak_ptr<juce::MidiOutput> out);


    /**
     * @brief Starts recording MIDI events.
     *
     * Clears any previous recording, applies presets, and begins timestamping incoming MIDI messages.
     */
    void startRecording();

    /**
     * @brief Stops recording MIDI events.
     * @return True if recording was active and successfully stopped, false otherwise.
     */
    bool stopRecording();


    /**
     * @brief Starts playback of recorded MIDI events.
     * @return True if playback successfully started, false if no events are available.
     */
    bool startPlayBack();

   /**@brief Stops playback of recorded events and sends "All Notes Off" to the MIDI output. */
    void stopPlayBack();

    /**
     * @brief Starts playback of MIDI events loaded from a file.
     * Stops live recording or playback if active.
     */
    void startRecordingFilePlaying();

    /**@brief Stops playback of MIDI events loaded from a file and sends "All Notes Off". */
    void stopRecordingFilePlaying();

    /**
     * @brief Handles incoming MIDI messages and stores them if recording is active.
     * @param message MIDI message to handle.
     */
    void handleIncomingMessage(const juce::MidiMessage& message) override;

    void timerCallback() override;

    /**
     * @brief Sets the MIDI output device used for playback.
     * @param outputDev Weak pointer to the output device.
     */
    void setOutputDevice(std::weak_ptr<juce::MidiOutput> outputDev);

    /**
     * @brief Sets the program (instrument) number for left or right hand.
     * @param newProgram Program number to set.
     * @param choice "left" or "right" to select the hand.
     */
    void setProgarmNumber(int newProgram, const juce::String& choice="");

    /**
     * @brief Sets the initial program number (preset) to apply at recording start.
     * @param value Program number to apply.
     */
    void setInitialProgram(int value);

    /**
     * @brief Sets reverb value for playback.
     * @param value Reverb amount (0–127).
     */
    void setReverb(int value);

    /**
     * @brief Checks if the recorder is currently active.
     * @return True if currently recording, false otherwise.
     */
    bool getIsRecording();

    /**
     * @brief Checks if playback is currently active.
     * @return True if currently playing back, false otherwise.
     */
    bool getIsPlaying();

    /**
     * @brief Saves recorded MIDI events to a file.
     * @param fileToSaveTo File to save the recording.
     * @param errorMsg Will be populated with an error description if saving fails.
     * @param tempo Tempo in BPM to use for the MIDI file.
     * @return True on success, false on failure.
     */
    bool saveRecordingToFile(const juce::File& fileToSaveTo, juce::String& errorMsg, double tempo = 120.0);

    /**
     * @brief Loads MIDI events from a file for playback.
     * @param fileToParse File containing the MIDI data.
     * @param errorMsg Will be populated with an error description if parsing fails.
     * @return True on success, false on failure.
     */
    bool parseRecordingFromFile(const juce::File& fileToParse, juce::String& errorMsg);

    /**
     * @brief Returns the number of MIDI events currently recorded in memory.
     * @return Number of recorded events.
     */
    int getSizeRecorded();

    /**
     * @brief Returns the program (instrument) number assigned to the left hand.
     * @return Program number for left-hand playback.
     */
    int getProgramLeftHand();

    /**
     * @brief Returns the program (instrument) number assigned to the right hand.
     * @return Program number for right-hand playback.
     */
    int getProgramRightHand();

    std::vector<RecordedEvent>& getAllRecordedEvents();



private:

    int initialProgram = 0;
    int reverb = 50;
    bool programChanged = false;
    bool isRecording = false;
    bool isPlaying = false;
    bool isPlayingFile = false;

    double recordStartTime=0;
    double playBackStartTime = 0;
    int nextEventIndex = 0;
    int nextEventFileIndex = 0;

    std::weak_ptr<juce::MidiOutput> midiOutputDevice;
    std::vector<RecordedEvent> allEventsPlayed;
    std::vector<RecordedEvent> allEventsPlayedFile;
    int programLeftHand,programRightHand;
};
