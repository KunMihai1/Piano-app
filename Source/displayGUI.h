/*
  ==============================================================================

    displayGUI.h
    Created: 8 Jun 2025 2:44:43am
    Author:  Kisuke

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#include "PlayBackSettingsCustomComponent.h"
#include "TrackEntry.h"
#include "TrackPlayer.h"
#include "CustomToolTip.h"
#include "SubjectInterface.h"
#include "trackListComponentListener.h"
#include "InstrumentChooser.h"
#include "CustomBeatBar.h"
#include "IOHelper.h"
#include "TrackPlayerListener.h"
#include "StyleSection.h"

/**
 * @class TrackListComponent
 * @brief UI component that manages and displays folders and MIDI tracks.
 *
 * The `TrackListComponent` provides a dynamic interface for organizing, viewing, and manipulating
 * MIDI tracks and folders. It supports hierarchical folder views, drag-and-drop style list management,
 * renaming, sorting, and asynchronous file loading via JUCE's GUI and FileChooser APIs.
 *
 * It inherits from:
 * - **juce::Component** — for GUI rendering and layout.
 * - **juce::ListBoxModel** — to supply data for the JUCE ListBox that displays folders and tracks.
 * - **juce::ComboBox::Listener** — for handling sorting selection changes.
 * - **Subject<TrackListListener>** — to notify registered listeners when tracks are added, removed, or renamed.
 *
 * This component is central to managing track data in the app and synchronizing it with
 * persistent storage and UI updates.
 *
 * @note The component maintains two primary modes of operation:
 * - `FolderView` — Displays available folders (groups of tracks).
 * - `TrackView` — Displays tracks inside a selected folder.
 *
 * @see TrackEntry, TrackIOHelper, Subject, TrackListListener
 */
class TrackListComponent : public juce::Component,
                           private juce::ListBoxModel,
                           public juce::ComboBox::Listener,
                           public Subject<TrackListListener>
{
public:
    //======================================================================
    /** @brief Callback for removing a single track. */
    std::function<void(const juce::Uuid& uuid)> onRemoveTrack;

    /** @brief Callback for removing multiple tracks at once. */
    std::function<void(const std::vector<juce::Uuid>& uuids)> onRemoveMultipleTracks;

    /** @brief Callback for renaming a track from the list. */
    std::function<void(const juce::Uuid& uuid, const juce::String& newName)> onRenameTrackFromList;

    /** @brief Callback to update external data structures when a new track is added. */
    std::function<void(TrackEntry* newEntry)> addToMapOnAdding;

    /**
     * @brief Constructs the track list component.
     *
     * @param tracks Shared pointer to the deque of available track entries.
     * @param groupedTracksMap Shared pointer to a map of folder names to their corresponding tracks.
     * @param groupedKeys Shared pointer to a list of folder names.
     * @param onTrackChosen Function called when a user double-clicks a track.
     */
    TrackListComponent(std::shared_ptr<std::deque<TrackEntry>> tracks,
                       std::shared_ptr<std::unordered_map<juce::String, std::deque<TrackEntry>>> groupedTracksMap,
                       std::shared_ptr<std::vector<juce::String>> groupedKeys,
                       std::function<void(int)> onTrackChosen);

    /** @brief Positions and resizes child components within the main area. */
    void resized() override;

    /** @brief Returns the number of rows to display in the ListBox. */
    int getNumRows() override;

    /** @brief Responds to sorting combo box changes. */
    void comboBoxChanged(juce::ComboBox* comboBoxThatHasChanged) override;

    /**
     * @brief Paints an individual item in the list box.
     * @param rowNumber The index of the item to paint.
     * @param g The graphics context.
     * @param width The width of the list row.
     * @param height The height of the list row.
     * @param rowIsSelected Whether the current row is selected.
     */
    void paintListBoxItem(int rowNumber, juce::Graphics& g, int width, int height, bool rowIsSelected) override;

    /**
     * @brief Called when a list box item is clicked (single or double click).
     * @param row The clicked row index.
     * @param event The associated mouse event.
     */
    void listBoxItemClicked(int row, const juce::MouseEvent& event) override;

    /** @brief Adds new tracks to the current folder from selected MIDI files. */
    void addToTrackList();

    /** @brief Removes selected tracks from the current folder. */
    void removeFromTrackList();

    /** @brief Renames a selected track in the list. */
    void renameFromTrackList();

    /** @brief Creates a new folder (group) for organizing tracks. */
    void addToFolderList();

    /** @brief Removes one or more selected folders and their contained tracks. */
    void removeFromFolderList();

    /** @brief Renames the currently selected folder. */
    void renameFromFolderList();

    /** @brief Returns to folder view from inside a track list. */
    void backToFolderView();

    /**
     * @brief Returns a reference to the current list of available tracks.
     * @return Reference to the track deque.
     */
    std::deque<TrackEntry>& getAllAvailableTracks() const;

    /**
     * @brief Extracts a readable display name from a given MIDI sequence.
     * @param trackSeq The MIDI sequence to extract from.
     * @return A formatted display name (e.g., "Piano (Grand)").
     */
    juce::String extractDisplayNameFromTrack(const juce::MidiMessageSequence& trackSeq);

    /** @brief Initializes the UI and buttons for viewing tracks inside a folder. */
    void initializeTracksFromList();

    /** @brief Clears track-related UI components when exiting track view. */
    void deallocateTracksFromList();

private:
    //======================================================================
    /**
     * @enum ViewMode
     * @brief Represents the current visual state of the component.
     */
    enum class ViewMode
    {
        FolderView,  /**< Folder hierarchy is displayed. */
        TrackView    /**< Individual tracks are displayed. */
    };

    /** @brief Current view mode of the component (folder or track view). */
    ViewMode viewMode = ViewMode::FolderView;

    /** @brief Shared list of available tracks (flat representation). */
    std::shared_ptr<std::deque<TrackEntry>> availableTracks;

    /** @brief Shared map of folders to their contained tracks. */
    std::shared_ptr<std::unordered_map<juce::String, std::deque<TrackEntry>>> groupedTracks;

    /** @brief Shared list of folder names (keys for the grouped map). */
    std::shared_ptr<std::vector<juce::String>> groupedTrackKeys;

    /** @brief Callback invoked when a user selects a track to play or edit. */
    std::function<void(int)> trackChosenCallBack;

    /** @brief ListBox displaying folders or tracks. */
    juce::ListBox listBox;

    // UI Controls -----------------------------------------------------------
    std::unique_ptr<juce::TextButton> addButton = nullptr;      /**< Add track button. */
    std::unique_ptr<juce::TextButton> removeButton = nullptr;   /**< Remove track button. */
    std::unique_ptr<juce::TextButton> backButton = nullptr;     /**< Back to folders button. */
    std::unique_ptr<juce::TextButton> renameButton = nullptr;   /**< Rename track button. */

    juce::TextButton addButtonFolder{ "Add folder" };           /**< Add folder button (visible in folder view). */
    juce::TextButton removeButtonFolder{ "Remove folder" };     /**< Remove folder button (visible in folder view). */
    juce::TextButton renameButtonFolder{ "Rename folder" };     /**< Rename folder button (visible in folder view). */

    std::unique_ptr<juce::ComboBox> sortComboBox = nullptr;     /**< Sorting options dropdown. */

    /** @brief Stores the name of the currently selected folder. */
    juce::String currentFolderName;
};



/**
 * @class Track
 * @brief Represents a single track in the piano application.
 *
 * The `Track` component encapsulates both the visual and logical representation
 * of a single MIDI track. It provides UI controls for volume, instrument selection,
 * and track name, while also handling user interactions such as renaming,
 * copying, pasting, deleting, and displaying note information.
 *
 * It acts as a Subject for `TrackListener`, allowing other components to react
 * to updates such as instrument or volume changes.
 *
 * @see TrackListener, Subject, InstrumentChooserComponent
 */
class Track : public juce::Component,
              private juce::MouseListener,
              public Subject<TrackListener>
{
public:
    //==============================================================================
    /** @brief Callback invoked whenever the track's state changes (e.g., rename, delete). */
    std::function<void()> onChange;

    /**
     * @brief Callback for requesting track selection.
     * 
     * This function accepts another callback which is called once a user selects a track,
     * providing the selected name, UUID, and track type.
     */
    std::function<void(std::function<void(const juce::String&, const juce::Uuid&, const juce::String&)>)> onRequestTrackSelection;

    /** @brief Returns whether the application is currently playing. */
    std::function<bool()> isPlaying;

    /** @brief Synchronizes volume changes across percussion tracks. */
    std::function<void(double newVolume)> syncVolumePercussionTracks;

    /** @brief Called when a track is copied. */
    std::function<void(Track* copiedTrack)> onCopy;

    /** @brief Called when a track is pasted. */
    std::function<void(Track* toPaste)> onPaste;

    /** @brief Displays note information for the given track UUID and channel. */
    std::function<void(const juce::Uuid& uuid, int channel)> onShowInformation;

    //==============================================================================
    /** @brief Constructs an empty Track component with default UI elements. */
    Track();

    /** @brief Destructor — safely releases resources and callbacks. */
    ~Track();

    /** @brief Lays out all UI elements within the component. */
    void resized() override;

    /** @brief Paints the outline and background of the track component. */
    void paint(juce::Graphics& g) override;

    /** @brief Handles mouse entry events (used for displaying tooltips). */
    void mouseEnter(const juce::MouseEvent& ev) override;

    /** @brief Handles mouse exit events (hides tooltips). */
    void mouseExit(const juce::MouseEvent& ev) override;

    /**
     * @brief Serializes the track's data to a JSON-style DynamicObject.
     * @return Pointer to a dynamically allocated `juce::DynamicObject` representing this track.
     */
    juce::DynamicObject* getJson() const;

    /**
     * @brief Loads track data from a JSON file.
     * @param file The file containing JSON track data.
     * @return A parsed `juce::var` object containing the track properties.
     */
    juce::var loadJson(const juce::File& file);

    /**
     * @brief Displays a temporary info message in a callout box.
     * @param text The message to display.
     */
    void showInstantInfo(const juce::String& text);

    //==============================================================================
    /** @brief Sets the volume slider's value. */
    void setVolumeSlider(double value);

    /**
     * @brief Updates the volume label.
     * @param value The new volume string.
     * @param shouldNotify Whether listeners should be notified.
     */
    void setVolumeLabel(const juce::String& value, bool shouldNotify = false);

    /** @brief Updates the track’s display name. */
    void setNameLabel(const juce::String& name);

    /** @brief Returns the track’s display name. */
    juce::String getName() const;

    /** @brief Opens the instrument chooser popup. */
    void openInstrumentChooser();

    /** @brief Builds a list of all available instrument names. */
    juce::StringArray instrumentListBuild();

    /**
     * @brief Sets the track’s MIDI instrument number.
     * @param newInstrumentNumber The new instrument number (0–127).
     * @param shouldNotify Whether listeners should be notified.
     */
    void setInstrumentNumber(int newInstrumentNumber, bool shouldNotify = false);

    /** @brief Returns the track’s current instrument number. */
    int getInstrumentNumber() const;

    /** @brief Assigns a unique identifier (UUID) to the track. */
    void setUUID(const juce::Uuid& newUUID);

    /** @brief Sets the type of this track (e.g., “melody”, “percussion”). */
    void setTypeOfTrack(const juce::String& newType);

    /** @brief Returns the type of this track. */
    juce::String getTypeOfTrack() const;

    /** @brief Returns the UUID of this track. */
    juce::Uuid getUsedID() const;

    /** @brief Sets the MIDI channel used by this track. */
    void setChannel(int newChannel);

    /** @brief Returns the MIDI channel currently used by this track. */
    int getChannel() const;

    /** @brief Returns the track’s current volume value. */
    double getVolume() const;

    //==============================================================================
    /**
     * @brief Copies all track parameters from another Track instance.
     * @param other The source track to copy from.
     */
    void copyFrom(const Track& other);

    /**
     * @brief Pastes all stored parameters into this track.
     * @param source The copied track to paste data from.
     */
    void pasteFrom(const Track& source);

    /** @brief Opens a rename dialog for this track. */
    void renameOneTrack();

    /** @brief Opens a confirmation dialog and removes the track if confirmed. */
    void deleteOneTrack();

    /** @brief Copies the track’s current state to the clipboard. */
    void copyOneTrack();

    /** @brief Pastes previously copied track data onto this one. */
    void pasteOneTrack();

    /** @brief Displays note information for the current track. */
    void showNotesInformation();

private:
    //==============================================================================
    /** @brief Handles mouse clicks on the name label for left/right-click menu actions. */
    void mouseDown(const juce::MouseEvent& event) override;

    //==============================================================================
    int usedInstrumentNumber = -1;     ///< Currently selected MIDI instrument number.
    int channel = 1;                   ///< Assigned MIDI channel.
    juce::String typeOfTrack;          ///< Type of track ("melody", "percussion", etc.).
    juce::Uuid uniqueIdentifierTrack;  ///< Unique ID for identifying this track.

    //==============================================================================
    juce::Slider volumeSlider;               ///< Vertical slider controlling track volume.
    juce::Label volumeLabel;                 ///< Displays numeric volume value.
    juce::Label nameLabel;                   ///< Displays the track’s name.
    juce::TextButton instrumentChooserButton;///< Opens instrument selection popup.
    juce::StringArray instrumentlist;        ///< List of all MIDI instrument names.
    std::unique_ptr<TrackNameToolTip> toolTipWindow = nullptr; ///< Tooltip window for displaying track name.
};



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
                              private juce::ComboBox::Listener
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
                          std::weak_ptr<juce::MidiOutput> outputDevice);

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
     * @brief Updates the current style’s name and tooltip.
     * @param newName New style name to display.
     */
    void updateName(const juce::String& newName);

    /** @brief Returns the current style’s name. */
    juce::String getName();

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
    void stopPlaying();

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
    void applyBPMchangeBeforePlayback(double userBPM, bool whenLoad = false, bool applyStyleChanges=false);

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
};



/**
 * @class StyleViewComponent
 * @brief Represents a single style item in a style list UI.
 *
 * This component displays a style name and allows the user to:
 * - Click to select the style
 * - Rename the style
 * - Delete the style
 * 
 * Supports left-click for selection and right-click for context menu actions.
 */
class StyleViewComponent : public juce::Component, public juce::MouseListener
{
public:
    //==============================================================================
    /** 
     * @brief Callback invoked when the style is renamed.
     * @param oldName Original name of the style.
     * @param newName New name after renaming.
     */
    std::function<void(const juce::String& oldName, const juce::String& newName)> onStyleRenamed;

    /** 
     * @brief Function to check if a style name already exists.
     * @param name The name to check.
     * @return True if the name exists in the list, false otherwise.
     */
    std::function<bool(const juce::String& name)> isInListNames;

    /** 
     * @brief Callback invoked when the style is removed.
     * @param name Name of the style being removed.
     */
    std::function<void(const juce::String& name)> onStyleRemoveComponent;

    /** 
     * @brief Callback invoked when the style is clicked.
     * @param name Name of the clicked style.
     */
    std::function<void(const juce::String&)> onStyleClicked;

    //==============================================================================
    /**
     * @brief Constructs a StyleViewComponent with the given style name.
     * @param styleName Name of the style to display.
     */
    StyleViewComponent(const juce::String& styleName);

    /** @brief Lays out the label inside the component. */
    void resized() override;

    /**
     * @brief Handles mouse click events.
     * 
     * Left-click triggers `onStyleClicked`.
     * Right-click opens a context menu with "Rename" and "Delete" actions.
     * 
     * @param event The mouse event details.
     */
    void mouseUp(const juce::MouseEvent& event) override;

    /**
     * @brief Updates the displayed style name.
     * @param name New name to display.
     */
    void setNameLabel(const juce::String& name);

    /** @brief Returns the current displayed style name. */
    juce::String getNameLabel() const;

    /** @brief Opens a dialog to rename the style and validates the new name. */
    void changeNameLabel();

    /** @brief Opens a confirmation dialog and removes the style if confirmed. */
    void removeStyle();

private:
    //==============================================================================
    juce::Label label;  ///< Label displaying the style name.
};



/**
 * @class StylesListComponent
 * @brief A scrollable container managing multiple `StyleViewComponent` instances.
 * 
 * This component displays a list of styles in a 2-column grid layout, allows adding, 
 * renaming, and removing styles, and handles callbacks for user interactions.
 * It uses a `juce::Viewport` to make the list scrollable.
 */
class StylesListComponent : public juce::Component
{
public:
    //==============================================================================
    /**
     * @brief Constructs a StylesListComponent with initial styles.
     * @param stylesNamesOut Vector of initial style names.
     * @param onStyleClicked Callback when a style is clicked.
     * @param widthSize Optional width size for layout calculations.
     */
    StylesListComponent(std::vector<juce::String> stylesNamesOut, 
                        std::function<void(const juce::String&)> onStyleClicked, 
                        int widthSize = 0);

    /** 
     * @brief Callback invoked when a style is renamed.
     * @param oldName Original name of the style.
     * @param newName New name of the style.
     */
    std::function<void(const juce::String& oldName, const juce::String& newName)> onStyleRename;

    /** 
     * @brief Callback invoked when a new style is added.
     * @param newName Name of the newly added style.
     */
    std::function<void(const juce::String& newName)> onStyleAdd;

    /**
     * @brief Callback invoked when a style is removed.
     * @param name Name of the removed style.
     */
    std::function<void(const juce::String& name)> onStyleRemove;

    //==============================================================================
    /** @brief Handles resizing of the component and lays out child components. */
    void resized() override;

    /** @brief Paints the background and control bar of the component. */
    void paint(juce::Graphics& g) override;

    /**
     * @brief Sets the width size used for layout calculations.
     * @param newWidth New width size.
     */
    void setWidthSize(const int newWidth);

    /** @brief Lays out all `StyleViewComponent`s in a grid. */
    void layoutStyles();

    /** @brief Clears and repopulates the list of styles. */
    void repopulate();

    /** @brief Opens a dialog to add a new style. */
    void addNewStyle();

    /**
     * @brief Assigns callbacks for a new `StyleViewComponent`.
     * @param newStyle Pointer to the style component.
     * @param currentName Name of the style.
     */
    void allCallBacks(StyleViewComponent* newStyle, const juce::String& currentName);

    /**
     * @brief Removes a style locally from the list without external prompts.
     * @param name Name of the style to remove.
     */
    void removeStyleLocally(const juce::String& name);

    /**
     * @brief Adds a style locally to the list without external prompts.
     * @param newName Name of the style to add.
     */
    void addStyleLocally(const juce::String& newName);

    /** @brief Rebuilds the internal vector of style names from the current components. */
    void rebuildStyleNames();

private:
    //==============================================================================
    /** @brief Populates the container with `StyleViewComponent`s based on `stylesNames`. */
    void populate();

    juce::OwnedArray<StyleViewComponent> allStyles;  ///< All style components.
    std::function<void(const juce::String&)> onStyleClicked;  ///< Callback when a style is clicked.
    std::vector<juce::String> stylesNames;  ///< Names of all styles.
    int widthSize;  ///< Width used for layout calculations.

    juce::TextButton addButton{ "Add" };  ///< Button to add a new style.
    std::unique_ptr<juce::Component> styleItemsContainer = nullptr;  ///< Container holding all styles.
    juce::Viewport viewport;  ///< Scrollable viewport for the style list.

    JUCE_LEAK_DETECTOR(StylesListComponent)
};


/**
 * @class MyTabbedComponent
 * @brief Custom tabbed component that allows a callback when the current tab changes.
 *
 * Extends `juce::TabbedComponent` and adds a std::function callback
 * `onTabChanged` which is called whenever the current tab is changed.
 */

class MyTabbedComponent : public juce::TabbedComponent, private juce::MouseListener
{
public:

    /**
     * @brief Constructs a MyTabbedComponent with a specified tab orientation.
     * @param orientation Orientation of the tab bar (e.g., TabsAtTop, TabsAtLeft)
     */
    MyTabbedComponent(juce::TabbedButtonBar::Orientation orientation);

    ~MyTabbedComponent();

     /**
     * @brief Called by JUCE when the current tab changes.
     * @param newCurrentTabIndex Index of the newly selected tab
     * @param newTabName Name of the newly selected tab
     */
    void currentTabChanged(int newCurrentTabIndex, const juce::String& newTabName) override;

    /** @brief Callback function triggered whenever the current tab changes */
    std::function<void(int, juce::String)> onTabChanged;

    void mouseDown(const juce::MouseEvent& event) override;

    //JUCE_LEAK_DETECTOR(MyTabbedComponent)
};

/**
 * @class DisplayListener
 * @brief Abstract interface to receive updates from the Display component.
 *
 * Classes implementing `DisplayListener` can receive notifications when
 * playback settings are changed.
 */
class DisplayListener {
public:

    /** @brief Destructor */
    virtual ~DisplayListener() = default;

    /**
     * @brief Called when playback settings change.
     * @param settings The new playback settings
     */
    virtual void playBackSettingsChanged(const PlayBackSettings& settings)=0;

    virtual void playBackSettingsTransposeChanged(int transposeValue) = 0;
};



/**
 * @class Display
 * @brief Main UI component for managing styles, tracks, and playback settings.
 *
 * The `Display` class handles the visual representation of styles, the currently
 * selected style, and associated tracks. It provides functionality to add, remove,
 * and rename styles, manage track assignments, and interact with playback settings.
 * This class also persists style and track data in JSON format.
 */
class Display : public juce::Component,
                public juce::ChangeListener,
                public TrackListListener
{
public:
    //==============================================================================
    /**
     * @brief Constructs a Display component.
     * @param outputDev Optional MIDI output device.
     * @param widthForList Width of the style list component.
     */
    Display(std::weak_ptr<juce::MidiOutput> outputDev, int widthForList = 0);

    /** @brief Destructor */
    ~Display() override;

    /** @brief Callback from `ChangeBroadcaster` */
    void changeListenerCallback(juce::ChangeBroadcaster* source) override;

    /**
     * @brief Gets the tab index for a tab by its name.
     * @param name Name of the tab.
     * @return Index of the tab, or -1 if not found.
     */
    int getTabIndexByName(const juce::String& name);

    /** @brief Initializes all default styles in JSON if missing */
    void initializeAllStyles();

    /** @brief Returns all style names currently in JSON */
    std::vector<juce::String> getAllStylesFromJson();

    /** @brief Loads all styles from JSON into memory */
    void loadAllStyles();

    /** @brief Updates the current style in JSON */
    void updateStyleInJson(const juce::String& name);

    /** @brief Updates a style's name in JSON */
    void updateStyleNameInJson(const juce::String& oldName, const juce::String& newName);

    /** @brief Appends a new style to JSON */
    void appendNewStyleInJson(const juce::String& newName);

    /** @brief Removes a style from JSON */
    void removeStyleInJson(const juce::String& name);

    /** @brief Builds a map of Track UUIDs to `TrackEntry` pointers */
    std::unordered_map<juce::Uuid, TrackEntry*> buildTrackUuidMap();

    /** @brief Handles resizing of the component */
    void resized() override;

    /** @brief Returns the root JSON variable representing all styles */
    const juce::var& getJsonVar();

    /** @brief Displays the currently selected style tab */
    void showCurrentStyleTab(const juce::String& name);

    /**
     * @brief Shows a list of tracks to select from and returns the selected track via callback
     * @param onTrackSelected Callback when a track is selected
     */
    void showListOfTracksToSelectFrom(std::function<void(const juce::String&, const juce::Uuid&, const juce::String& type)> onTrackSelected);

    /** @brief Returns a list of available tracks from a folder */
    std::vector<TrackEntry> getAvailableTracksFromFolder(const juce::File& folder);

    /** @brief Sets the MIDI output device */
    void setDeviceOutput(std::weak_ptr<juce::MidiOutput> devOutput);

    /** @brief Removes a track from all styles */
    void removeTrackFromAllStyles(const juce::Uuid& uuid);

    /** @brief Removes multiple tracks from all styles */
    void removeTracksFromAllStyles(const std::vector<juce::Uuid>& uuids);

    /** @brief Updates a track name in all styles */
    void updateTrackNameFromAllStyles(const juce::Uuid& uuid, const juce::String& newName);

    /** @brief Stops playback */
    void stoppingPlayer();

    /** @brief Starts playback */
    void startingPlayer();

    /** @brief Updates UI before any loading operation */
    void updateUIbeforeAnyLoadingCase() override;

    /** @brief Sets minimum and maximum MIDI note boundaries */
    void set_min_max(int min, int max);

    /** @brief Sets VID and PID for playback settings */
    void set_VID_PID(const juce::String& VID, const juce::String& PID);

    /** @brief Gets the VID of the current input device */
    juce::String getVID();

    /** @brief Gets the PID of the current input device*/
    juce::String getPID();

    /** @brief Loads playback settings from JSON */
    void readSettingsFromJSON();

    /** @brief Handles home button interactions */
    void homeButtonInteraction();

    /** @brief get the start note of playing range */
    int getStartNote();

    /** @brief get the end note of playing range */
    int getEndNote();

    /** @brief get the left max bound of playing range left hand */
    int getLeftBound();

    /** @brief get the right min bound of playing range rigt hand */
    int getRightBound();

    /** @brief adds display listener */
    void addListener(DisplayListener* listener);

    /** @brief removes display listener */
    void removeListener(DisplayListener* listener);

    /** @brief calling the listener's function that they inherit */
    void callingListeners();

    /** @brief get total number of tabs of the tabbed component */
    int getNumTabs();

    /** @brief sets all the settings including min,max note, playing range on left/right hand */
    void setNewSettingsHelperFunction(int value);

    /** @brief Checks if a certain tab exists in the tabbed component*/
    bool existsTab(const juce::String& name);

private:
    //==============================================================================
    juce::HashMap<juce::String, std::unique_ptr<juce::DynamicObject>> styleDataCache; ///< Cached style data
    std::unique_ptr<StylesListComponent> stylesListComponent; ///< UI component for style list
    std::unique_ptr<MyTabbedComponent> tabComp; ///< Tabbed component for styles & tracks
    std::unique_ptr<CurrentStyleComponent> currentStyleComponent; ///< UI for current style
    std::unique_ptr<PlayBackSettingsComponent> playBackSettings; ///< Playback settings UI
    bool created = false; ///< Flag for style tab creation
    bool createdTracksTab = false; ///< Flag for tracks tab creation
    bool createdPlaybackSettingsTab = false;

    int minNote, maxNote;  ///< min and max notes of playing range
    PlayBackSettings settings; ///< Playback settings

    std::shared_ptr<std::deque<TrackEntry>> availableTracksFromFolder; ///< Tracks loaded from folder
    std::shared_ptr<std::unordered_map<juce::String, std::deque<TrackEntry>>> groupedTracks; ///< Tracks grouped by folder
    std::shared_ptr<std::vector<juce::String>> groupedTrackKeys; ///< Keys for grouped tracks

    std::unordered_map<juce::String, std::vector<StyleSection>> sectionsPerStyle;

    std::weak_ptr<juce::MidiOutput> outputDevice; ///< MIDI output device

    std::unique_ptr<TrackListComponent> trackListComp; ///< Track selection component
    juce::var allStylesJsonVar; ///< Root JSON object storing all styles
    std::unordered_map<juce::Uuid, TrackEntry*> mapUuidToTrack; ///< Map of track UUIDs to entries

    juce::ListenerList<DisplayListener> displayListeners; ///< Registered listeners
};
