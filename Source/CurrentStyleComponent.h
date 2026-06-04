#pragma once
#include <JuceHeader.h>
#include "Track.h"
#include "TrackPlayer.h"
#include "TrackPlayerListener.h"
#include "StyleSection.h"
#include "CustomBeatBar.h"

/**
 * @class CurrentStyleComponent
 * @brief Manages and displays all tracks within a musical style.
 *
 * The `CurrentStyleComponent` acts as a container and controller for multiple
 * `Track` components. It manages playback, tempo, instrument synchronization,
 * and communication with a `MultipleTrackPlayer`.
 *
 * It provides control over all tracks simultaneously or individually, and
 * handles serialization, deserialization, and UI updates for the current style.
 *
 * Key features include:
 * - Managing up to 8 tracks per style
 * - Syncing tempo (BPM) changes across all tracks
 * - Controlling playback (start/stop)
 * - Managing track selections, renaming, and deletions
 * - Handling JSON import/export of style data
 */
class CurrentStyleComponent : public juce::Component,
                              private juce::MouseListener,
                              private juce::ComboBox::Listener,
                              public TrackPlayerListenerModifyStateObjects
{
public:

    //==============================================================================

    std::function<bool(const juce::String& name)> tabExsitsCallback;

    /** @brief Called whenever any track within the style is modified. */
    std::function<void()> anyTrackChanged;

    /**
     * @brief Callback for requesting a track selection from another component.
     *
     * Passes a callback which will be called with the chosen track name, UUID, and type.
     */
    std::function<void(std::function<void(const juce::String&, const juce::Uuid&, const juce::String&)>)> onRequestTrackSelectionFromTrack;

    /** @brief Callback to trigger track file updates after changes. */
    std::function<void()> updateTrackFile;

    /** @brief Callback triggered when switching to the keybinds tab. */
    std::function<void()> keybindTabStarting;

    //==============================================================================
    /**
     * @brief Constructs the CurrentStyleComponent.
     *
     * @param name Name of the style.
     * @param map Reference to the UUID-to-TrackEntry map used for synchronization.
     * @param outputDevice Weak pointer to the MIDI output device used for playback.
     */
    CurrentStyleComponent(const juce::String& name,
                          std::unordered_map<juce::Uuid, TrackEntry*>& map,
                          std::weak_ptr<juce::MidiOutput> outputDevice,
                          std::weak_ptr<std::unordered_map<juce::String, std::unordered_map<juce::String,StyleSection>>> styleSectionsMap);

    /** @brief Destructor. Cleans up listeners and internal references. */
    ~CurrentStyleComponent() override;

    //==============================================================================
    /** @brief Triggers the stop button click programmatically. */
    void triggerStopClick();

    /** @brief Triggers the start button click programmatically. */
    void triggerStartClick();

    /** @brief Stops the player immediately and resets internal states. */
    void stoppingPlayer();

    //==============================================================================
    /** @brief Resizes all child components (tracks, sliders, buttons, labels). */
    void resized() override;

    /**
     * @brief Updates the current style's name and tooltip.
     * @param newName New style name to display.
     */
    void updateName(const juce::String& newName);

    void updateObjects() override;

    /** @brief Returns the current style's name. */
    juce::String getName();

    juce::String getStyleID();

    /**
     * @brief Serializes the current style to a JUCE DynamicObject.
     * @return Pointer to a dynamically allocated object representing this style.
     */
    juce::DynamicObject* getJson() const;

    /**
     * @brief Loads style data from a JSON-style `juce::var` object.
     * @param styleVar A `juce::var` representing serialized style data.
     */
    void loadJson(const juce::var& styleVar);

    //==============================================================================
    /** @brief Starts playback of selected tracks based on play mode (all/solo). */
    void startPlaying();

    /** @brief Stops playback of all tracks. */
    void stopPlaying(bool shouldModify=true);

    /** @brief Returns the current playback tempo (BPM). */
    double getTempo();

    /** @brief Returns the base tempo used for scaling sequences. */
    double getBaseTempo();

    /**
     * @brief Sets a new playback tempo (BPM).
     * @param newTempo The new tempo in beats per minute.
     */
    void setTempo(double newTempo);

    /**
     * @brief Sets a unique style identifier for this component.
     * @param newID New style ID string.
     */
    void setStyleID(const juce::String& newID);

    /**
     * @brief Applies style-based modifications to a single track.
     * @param track Reference to a `TrackEntry` object.
     */
    void applyChangesForOneTrack(TrackEntry& track);

    /** @brief Applies style modifications to all tracks in the current style. */
    void applyChangesForAllTracksCurrentStyle();

    /**
     * @brief Removes a single track by UUID, resetting it visually and logically.
     * @param uuid UUID of the track to remove.
     */
    void removingTrack(const juce::Uuid& uuid);

    /**
     * @brief Removes multiple tracks by their UUIDs.
     * @param uuids List of track UUIDs to remove.
     */
    void removingTracks(const std::vector<juce::Uuid>& uuids);

    /**
     * @brief Renames a track by UUID.
     * @param uuid UUID of the track to rename.
     * @param newName New name for the track.
     */
    void renamingTrack(const juce::Uuid& uuid, const juce::String& newName);

    /**
     * @brief Displays note information for a specific track.
     * @param uuid Track UUID.
     * @param channel MIDI channel associated with the track.
     */
    void showingTheInformationNotesFromTrack(const juce::Uuid& uuid, int channel);

    /**
     * @brief Sets the MIDI output device for the current style.
     * @param newOutput New weak pointer to a MIDI output device.
     */
    void setDeviceOutputCurrentStyle(std::weak_ptr<juce::MidiOutput> newOutput);

    /** @brief Returns all tracks currently contained in this style. */
    juce::OwnedArray<Track>& getAllTracks();

    /** @brief Returns a pointer to the internal track player instance. */
    MultipleTrackPlayer* getTrackPlayer();

    void setMidiInjectCallback(std::function<void(const juce::MidiMessage&)> cb);

    struct TrackChannelInstrument { int channel; int instrument; };
    std::vector<TrackChannelInstrument> getTrackChannelInstruments() const;

    /**
     * @brief Synchronizes volume changes across all percussion tracks.
     * @param newVolume The new volume value to apply.
     */
    void syncPercussionTracksVolumeChange(double newVolume);

    /**
     * @brief Applies BPM changes globally before playback starts.
     * @param userBPM The new user-defined tempo.
     * @param whenLoad Whether this was triggered during loading.
     */
    void applyBPMchangeBeforePlayback(double userBPM, bool applyStyleChanges=false);

    /**
     * @brief Applies BPM scaling to a specific track only.
     * @param userBPM The new BPM.
     * @param uuid UUID of the target track.
     */
    void applyBPMchangeForOne(double userBPM, const juce::Uuid& uuid);



    //==============================================================================
    /**
     * @brief Handles combo box changes for playback options.
     * @param comboBoxThatHasChanged Pointer to the combo box triggering the event.
     */
    void comboBoxChanged(juce::ComboBox* comboBoxThatHasChanged) override;

    /**
     * @brief Changes the combo box selection index manually.
     * @param Index The new index to set.
     */
    void comboBoxChangeIndex(int Index);

    bool getIsPlaying();

    void handleIntroCurrentStyle(const juce::String& name, const std::unordered_map<juce::String,StyleSection>& section);

    void handleEndingCurrentStyle(const juce::String& name, const std::unordered_map<juce::String, StyleSection>& section);

    void handleVarCurrentStyle(const juce::String& name, const std::unordered_map<juce::String, StyleSection>& section);

    void handleFillCurrentStyle(const juce::String& name, const std::unordered_map<juce::String, StyleSection>& section);

    void handleBreakCurrentStyle(const juce::String& name, const std::unordered_map<juce::String, StyleSection>& section);

private:
    //==============================================================================
    /** @brief Handles mouse clicks for selecting tracks. */
    void mouseDown(const juce::MouseEvent& ev) override;

    //==============================================================================
    juce::String name;                                      ///< Name of the current style.
    juce::Label nameOfStyle;                                ///< Label displaying the style name.
    juce::Label selectedTrackLabel, selectedTrackKey, selectedTrackChord; ///< Info labels.
    juce::OwnedArray<Track> allTracks;                      ///< Container for all track components.
    juce::ComboBox playSettingsTracks;                      ///< Combo box for selecting play mode.
    juce::TextButton startPlayingTracks;                    ///< Button to start playback.
    juce::TextButton stopPlayingTracks;                     ///< Button to stop playback.

    std::weak_ptr<juce::MidiOutput> outputDevice;           ///< MIDI output device used for playback.
    std::unique_ptr<MultipleTrackPlayer> trackPlayer = nullptr; ///< Handles multi-track playback.
    std::unordered_map<juce::Uuid, TrackEntry*>& mapUuidToTrackEntry; ///< Global map of all track entries.
    Track* lastSelectedTrack = nullptr;                     ///< Last track selected by the user.
    std::weak_ptr<std::vector<StyleSection>> sections;

    juce::Slider tempoSlider;                               ///< Slider for controlling BPM.
    int oldTempo = 120.0;                                   ///< Previously stored tempo.
    double currentTempo = 120.0;                            ///< Current tempo in BPM.
    double baseTempo = 120.0;                               ///< Base tempo for playback ratio.
    bool isPlaying = false;                                 ///< Indicates whether playback is active.
    juce::String styleID;                                   ///< Unique ID of the current style.

    std::unique_ptr<Track> copiedTrack = nullptr;           ///< Track object used for copy/paste operations.
    BeatBar customBeatBar;                                  ///< Visual component displaying beat progression.

    std::weak_ptr<std::unordered_map<juce::String, std::unordered_map<juce::String,StyleSection>>> styleSectionsMap;
};
