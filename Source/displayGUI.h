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

/**
 * @class TrackListComponent
 * @brief Component that displays and manages a list of tracks and folders.
 *
 * Supports adding, removing, renaming, sorting, and selecting MIDI tracks.
 * Tracks can be organized into folders, and double-clicking a track triggers a callback.
 */
class TrackListComponent : public juce::Component, private juce::ListBoxModel, public juce::ComboBox::Listener, public Subject<TrackListListener>
{
public:

    /** 
     * @brief Callback triggered when a single track is removed from the list.
     * @param uuid The UUID of the removed track.
     */
    std::function<void(const juce::Uuid& uuid)> onRemoveTrack;

    /** 
     * @brief Callback triggered when multiple tracks are removed at once.
     * @param uuids A vector of UUIDs corresponding to the removed tracks.
     */
    std::function<void(const std::vector<juce::Uuid>& uuids)> onRemoveMultipleTracks;

    /** 
     * @brief Callback triggered when a track is renamed from the list.
     * @param uuid The UUID of the renamed track.
     * @param newName The new name assigned to the track.
     */
    std::function<void(const juce::Uuid& uuid, const juce::String& newName)> onRenameTrackFromList;

    /** 
     * @brief Callback called when a new TrackEntry is added and should be added to a map for lookup.
     * @param newEntry Pointer to the newly added TrackEntry.
     */
    std::function<void(TrackEntry* newEntry)> addToMapOnAdding;

    /**
     * @brief Constructor.
     * @param tracks Shared deque of currently available tracks (flat list).
     * @param groupedTracksMap Shared map of folder names to tracks.
     * @param groupedKeys Shared vector of folder names.
     * @param onTrackChosen Callback triggered when a track is double-clicked.
     */
    TrackListComponent(std::shared_ptr<std::deque<TrackEntry>> tracks,
        std::shared_ptr<std::unordered_map<juce::String, std::deque<TrackEntry>>> groupedTracksMap,
        std::shared_ptr<std::vector<juce::String>> groupedKeys,
        std::function<void(int)> onTrackChosen);

    /** @brief Lays out buttons and list box. */
    void resized() override;

    /** @brief Returns number of rows in the ListBox. */
    int getNumRows() override;

    /** @brief Handles sorting when the sort combo box is changed. */
    void comboBoxChanged(juce::ComboBox* comboBoxThatHasChanged) override;

    /**
     * @brief Draws each row in the ListBox.
     * @param rowNumber The index of the row to paint.
     * @param g Graphics context.
     * @param width Width of the row.
     * @param height Height of the row.
     * @param rowIsSelected True if the row is selected.
     */
    void paintListBoxItem(int rowNumber, juce::Graphics& g, int width, int height, bool rowIsSelected) override;

    /**
     * @brief Handles mouse click on a row.
     * @param row Row index clicked.
     * @param event Mouse event information.
     */
    void listBoxItemClicked(int row, const juce::MouseEvent& event) override;

    /** @brief Adds a MIDI track to the current list using a file chooser. */
    void addToTrackList();

    /** @brief Removes selected track(s) from the current list. */
    void removeFromTrackList();

    /** @brief Renames the selected track. */
    void renameFromTrackList();

    /** @brief Adds a new folder to organize tracks. */
    void addToFolderList();

    /** @brief Removes selected folder(s) and their tracks. */
    void removeFromFolderList();

    /** @brief Renames the selected folder. */
    void renameFromFolderList();

    /** @brief Switches view back to folder mode. */
    void backToFolderView();

    /**
     * @brief Returns a reference to all available tracks in the current view.
     * @return deque of TrackEntry objects
     */
    std::deque<TrackEntry>& getAllAvailableTracks() const;

    /**
     * @brief Extracts a display name from a MIDI track sequence.
     * @param trackSeq The MIDI sequence.
     * @return Display name combining track name and instrument if available.
     */
    juce::String extractDisplayNameFromTrack(const juce::MidiMessageSequence& trackSeq);

     /** @brief Initializes view for flat track list (TrackView). */
    void initializeTracksFromList();

    /** @brief Deallocates dynamic buttons/comboBox used in TrackView. */
    void deallocateTracksFromList();


private:

    enum class ViewMode
    {
        FolderView,
        TrackView
    };

    ViewMode viewMode = ViewMode::FolderView;

    std::shared_ptr<std::deque<TrackEntry>> availableTracks;
    std::shared_ptr<std::unordered_map<juce::String, std::deque<TrackEntry>>> groupedTracks;
    std::shared_ptr<std::vector<juce::String>> groupedTrackKeys;

    std::function<void(int)> trackChosenCallBack;

    juce::ListBox listBox;


    std::unique_ptr<juce::TextButton> addButton = nullptr;
    std::unique_ptr<juce::TextButton> removeButton = nullptr;
    std::unique_ptr<juce::TextButton> backButton = nullptr;
    std::unique_ptr<juce::TextButton> renameButton = nullptr;

    juce::TextButton addButtonFolder{ "Add folder" };
    juce::TextButton removeButtonFolder{ "Remove folder" };
    juce::TextButton renameButtonFolder{ "Rename folder" };

    std::unique_ptr<juce::ComboBox> sortComboBox = nullptr;
    juce::String currentFolderName;

    //JUCE_LEAK_DETECTOR(TrackListComponent)
};

/**
 * @brief Represents a single track in a style.
 *
 * Handles volume control, instrument selection, track naming, copy/paste operations,
 * MIDI channel assignment, and mouse interactions for track management.
 */
class Track : public juce::Component, private juce::MouseListener, public Subject<TrackListener>
{
public:
    /** @brief Callback when the track changes (volume, instrument, etc.). */
    std::function<void()> onChange;

    /** @brief Callback for requesting track selection. */
    std::function<void(std::function<void(const juce::String&, const juce::Uuid& uuid, const juce::String& type)>)> onRequestTrackSelection;
  
    /** 
     * @brief Callback to check if the track is currently playing. 
     * Used in volumeSlider.onDragEnd to decide if a MIDI message should be sent.
     */
    std::function<bool()> isPlaying;

    /** @brief Callback to sync volume across all percussion tracks. */
    std::function<void(double newVolume)> syncVolumePercussionTracks;

    /** @brief Callback triggered when the track is copied. */
    std::function<void(Track* copiedTrack)> onCopy;

    /** @brief Callback triggered when the track is pasted. */
    std::function<void(Track* toPaste)> onPaste;

    /** @brief Callback triggered to show detailed information about notes. */
    std::function<void(const juce::Uuid& uuid, int channel)> onShowInformation;

    /** @brief Default constructor. Initializes sliders, labels, and buttons. */
    Track();

    /** @brief Destructor. Clears slider callbacks to avoid dangling references. */
    ~Track();

    /** @brief Called when the component is resized to layout sliders, labels, and buttons. */
    void resized() override;

    /** @brief Draws the component outline. */
    void paint(juce::Graphics& g) override;

    /** @brief Mouse hover enter event. Shows tooltip for track name. */
    void mouseEnter(const juce::MouseEvent& ev) override;

    /** @brief Mouse hover exit event. Hides tooltip. */
    void mouseExit(const juce::MouseEvent& ev) override;

    /** @brief Returns a JSON representation of the track properties. */
    juce::DynamicObject* getJson() const;

    /**
     * @brief Shows an instant info message near the track.
     * @param text Text to display.
     */
    void showInstantInfo(const juce::String& text);

    /**
     * @brief Loads JSON from a file.
     * @param file File to load.
     * @return Parsed JSON variable or empty if failed.
     */
    juce::var loadJson(const juce::File& file);

    /** @brief Sets the volume slider value. */
    void setVolumeSlider(double value);

    /** @brief Updates the volume label and optionally notifies the track player. */
    void setVolumeLabel(const juce::String& value, bool shouldNotify=false);

    /** @brief Sets the track name label. */
    void setNameLabel(const juce::String& name);

    /** @brief Returns the track name. */
    juce::String getName() const;

    /** @brief Opens the instrument chooser dialog. */
    void openInstrumentChooser();

    /** @brief Builds the list of available GM instruments. */
    juce::StringArray instrumentListBuild();

    /** @brief Sets the instrument number and optionally notifies the track player. */
    void setInstrumentNumber(int newInstrumentNumber, bool shouldNotify=false);

    /** @brief Returns the currently assigned instrument number. */
    int getInstrumentNumber() const;

    /** @brief Sets the track UUID. */
    void setUUID(const juce::Uuid& newUUID);

    /** @brief Sets the track type (e.g., "Percussion"). */
    void setTypeOfTrack(const juce::String& newType);

    /** @brief Returns the track type. */
    juce::String getTypeOfTrack() const;

    /** @brief Returns the currently assigned UUID. */
    juce::Uuid getUsedID() const;

    /** @brief Sets the MIDI channel for the track. */
    void setChannel(int newChannel);

    /** @brief Returns the track's MIDI channel. */
    int getChannel() const;

    /** @brief Returns the current volume slider value. */
    double getVolume() const;

    /**
     * @brief Copies all properties from another track.
     * @param other Track to copy from.
     */
    void copyFrom(const Track& other);

    /**
     * @brief Pastes properties from another track.
     * @param source Track to paste from.
     */
    void pasteFrom(const Track& source);

    /** @brief Opens a modal dialog to rename the track. */
    void renameOneTrack();

    /** @brief Opens a confirmation dialog to delete the track. */
    void deleteOneTrack();

    /** @brief Copies the track for later pasting. */
    void copyOneTrack();

    /** @brief Pastes the previously copied track into this track. */
    void pasteOneTrack();

    /** @brief Shows detailed notes information for this track. */
    void showNotesInformation();

private:
    /** @brief Mouse click event. Handles left-click for selection and right-click for context menu. */
    void mouseDown(const juce::MouseEvent& event) override;
    int usedInstrumentNumber=-1;
    int channel;
    juce::String typeOfTrack;

    juce::Uuid uniqueIdentifierTrack;


    juce::Slider volumeSlider;
    juce::Label volumeLabel;
    juce::Label nameLabel;
    juce::TextButton instrumentChooserButton;
    juce::StringArray instrumentlist;
    std::unique_ptr<TrackNameToolTip> toolTipWindow=nullptr;

    //JUCE_LEAK_DETECTOR(Track)
};

/**
 * @brief Component representing the currently selected style.
 *
 * Manages playback of tracks associated with a style, allows for tempo adjustments,
 * track selection, renaming, removal, and displays track information. Handles 
 * MIDI output via a shared device and syncs playback with MultipleTrackPlayer.
 *
 * Inherits from juce::Component, juce::MouseListener, and juce::ComboBox::Listener.
 */
class CurrentStyleComponent : public juce::Component, private juce::MouseListener, private juce::ComboBox::Listener
{
public:
    /** @brief Callback invoked whenever any track changes (volume, notes, etc.). */
    std::function<void()> anyTrackChanged;

    /**
     * @brief Callback invoked to request track selection.
     * @param callback Function that receives selected track info: name, UUID, type.
     */
    std::function<void(std::function<void(const juce::String&, const juce::Uuid& uuid, const juce::String& type)>)> onRequestTrackSelectionFromTrack;
  
    /** @brief Callback to update the current track file. */
    std::function<void()> updateTrackFile;

    /** @brief Callback invoked when the keybinds tab starts. */
    std::function<void()> keybindTabStarting;

    /**
     * @brief Constructor for CurrentStyleComponent.
     * @param name Name of the style to display.
     * @param map Reference to a map of track UUIDs to TrackEntry pointers.
     * @param outputDevice Weak pointer to a shared MIDI output device.
     */
    CurrentStyleComponent(const juce::String& name, std::unordered_map<juce::Uuid, TrackEntry*>& map, std::weak_ptr<juce::MidiOutput> outputDevice);

    /** @brief Stops playback via the start/stop buttons programmatically. */
    void triggerStopClick();

    /** @brief Starts playback via the start/stop buttons programmatically. */
    void triggerStartClick();

    /** @brief Stops the track player immediately. */
    void stoppingPlayer();

    /** @brief Destructor. */
    ~CurrentStyleComponent() override;

    /** @brief Handles resizing of the component and child elements. */
    void resized() override;

    /**
     * @brief Updates the displayed style name.
     * @param newName The new name to display.
     */
    void updateName(const juce::String& newName);

    /**
     * @brief Returns the current name of the style.
     * @return The style name.
     */
    juce::String getName();

    /**
     * @brief Retrieves the internal JSON representation of this style.
     * @return Pointer to a DynamicObject representing the JSON data.
     */
    juce::DynamicObject* getJson() const;

    /**
     * @brief Loads style data from a JSON variable.
     * @param styleVar The JSON variable containing style data.
     */
    void loadJson(const juce::var& styleVar);

    /** @brief Starts playback of selected tracks based on the playSettingsTracks selection. */
    void startPlaying();

    /** @brief Stops playback of tracks. */
    void stopPlaying();

    /** @brief Returns the current playback tempo in BPM. */
    double getTempo();

    /** @brief Sets the current playback tempo.
     *  @param newTempo New BPM value.
     */
    void setTempo(double newTempo);

    /** @brief Sets the unique ID of the style for internal mapping. */
    void setStyleID(const juce::String& newID);

    /**
     * @brief Applies style changes to a single track.
     * @param track Reference to the TrackEntry to update.
     */
    void applyChangesForOneTrack(TrackEntry& track);

    /** @brief Applies style changes to all tracks in the current style. */
    void applyChangesForAllTracksCurrentStyle();

    /**
     * @brief Removes a single track from the current style.
     * @param uuid UUID of the track to remove.
     */
    void removingTrack(const juce::Uuid& uuid);

    /**
     * @brief Removes multiple tracks from the current style.
     * @param uuids Vector of UUIDs for tracks to remove.
     */
    void removingTracks(const std::vector<juce::Uuid>& uuids);

    /**
     * @brief Renames a track within the current style.
     * @param uuid UUID of the track to rename.
     * @param newName New name for the track.
     */
    void renamingTrack(const juce::Uuid& uuid, const juce::String& newName);

    /**
     * @brief Shows detailed note information for a specific track.
     * @param uuid UUID of the track.
     * @param channel MIDI channel to display.
     */
    void showingTheInformationNotesFromTrack(const juce::Uuid& uuid, int channel);

    /** @brief Returns the base tempo (original BPM of style). */

    double getBaseTempo();

    /**
     * @brief Updates the MIDI output device used by this component and the track player.
     * @param newOutput Weak pointer to the new MIDI output device.
     */
    void setDeviceOutputCurrentStyle(std::weak_ptr<juce::MidiOutput> newOutput);

    /** @brief Returns all track components associated with this style. */
    juce::OwnedArray<Track>& getAllTracks();

    /** @brief Returns a pointer to the MultipleTrackPlayer instance. */
    MultipleTrackPlayer* getTrackPlayer();

    /**
     * @brief Synchronizes volume for all percussion tracks.
     * @param newVolume New volume value (0–127).
     */
    void syncPercussionTracksVolumeChange(double newVolume);

    /**
     * @brief Applies BPM changes to all tracks before playback.
     * @param userBPM Target BPM.
     * @param whenLoad Flag indicating whether this is applied during style load.
     */
    void applyBPMchangeBeforePlayback(double userBPM, bool whenLoad=false);

    /**
     * @brief Applies BPM changes to a single track.
     * @param userBPM Target BPM.
     * @param uuid UUID of the track to update.
     */
    void applyBPMchangeForOne(double userBPM, const juce::Uuid& uuid);

    /** @brief JUCE ComboBox listener callback. */
    void comboBoxChanged(juce::ComboBox* comboBoxThatHasChanged) override;

    /** @brief Sets the selected index of the play settings combo box. */
    void comboBoxChangeIndex(int Index);

     /** @brief Normalizes all tracks (TPQN, measure offset, etc.) */
    void normalizeAllTracks();

    /**
     * @brief Normalizes a MIDI sequence to a target TPQN and beat structure.
     * @param seq Sequence to normalize.
     * @param tpqn Target ticks per quarter note.
     * @param bpm Tempo in beats per minute.
     * @param beatsPerBar Number of beats per bar (default 4).
     */
    void normalizeMeasure(juce::MidiMessageSequence& seq, double tpqn, double bpm, int beatsPerBar=4);

    /**
     * @brief Scales MIDI sequence timestamps to a target TPQN.
     * @param seq Sequence to scale.
     * @param originalTPQN Original TPQN of the sequence.
     * @param targetTPQN Desired TPQN.
     */
    void normalizeTPQN(juce::MidiMessageSequence& seq, double originalTPQN, double targetTPQN);

    /** @brief Removes all tempo meta events from a sequence. */
    void stripTempoMetaEvents(juce::MidiMessageSequence& seq);

private:
    /** @brief Mouse listener to handle track selection via clicks. */
    void mouseDown(const juce::MouseEvent& ev) override;

    /** @brief Returns the time offset of the first note in a sequence in seconds. */
    double firstNoteOffsetInSeconds(const juce::MidiMessageSequence& seq);

    juce::String name;
    juce::Label nameOfStyle;
    juce::Label selectedTrackLabel, selectedTrackKey, selectedTrackChord;
    juce::OwnedArray<Track> allTracks;
    juce::ComboBox playSettingsTracks;
    juce::TextButton startPlayingTracks;
    juce::TextButton stopPlayingTracks;

    std::weak_ptr<juce::MidiOutput> outputDevice;
    std::unique_ptr<MultipleTrackPlayer> trackPlayer=nullptr;
    std::unordered_map<juce::Uuid, TrackEntry*>& mapUuidToTrackEntry;
    Track* lastSelectedTrack = nullptr;

    juce::Slider tempoSlider;
    int oldTempo = 120.0;
    double currentTempo = 120.0;
    double baseTempo = 120.0;
    bool isPlaying = false;
    juce::String styleID;

    std::unique_ptr<Track> copiedTrack=nullptr;
    BeatBar customBeatBar;

    //JUCE_LEAK_DETECTOR(CurrentStyleComponent)
};

/**
 * @brief A clickable UI element that displays and manages a single style name.
 *
 * Handles:
 * - Selecting a style (left-click)
 * - Context menu actions: Rename / Delete (right-click)
 * - Validating name changes through external callbacks
 */
class StyleViewComponent : public juce::Component, public juce::MouseListener
{
public:

    /** @brief Notifies when style name changes. */
    std::function<void(const juce::String& oldName, const juce::String& newName)> onStyleRenamed;  

    /**
     * @brief Validates name uniqueness among sibling items.
     *
     * @return true if the name already exists.
     */
    std::function<bool(const juce::String& name)> isInListNames;

    /** @brief Requests removal of this component from lists. */
    std::function<void(const juce::String& name)> onStyleRemoveComponent;

    /**
     * @brief Creates a visual item that displays the given style name.
     *
     * @param styleName Initial string displayed in the label.
     */
    StyleViewComponent(const juce::String& styleName);

    /** @brief Adjusts internal label bounds based on text size. */
    void resized() override;

    /**
     * @brief Handles selection or popup menu based on mouse button.
     *
     * @param event Mouse click information.
     */
    void mouseUp(const juce::MouseEvent& event) override;

    /**
     * @brief Updates the visible style label text.
     *
     * @param name New display name.
     */
    void setNameLabel(const juce::String& name);

    /**
     * @brief Retrieves the current style label text.
     *
     * @return The displayed name.
     */
    juce::String getNameLabel() const;

    /**
     * @brief Prompts the user to rename this style.
     *
     * Validates:
     * - New name is not empty
     * - New name is unique (checked via `isInListNames` callback)
     */
    void changeNameLabel();

    /**
     * @brief Prompts the user to confirm and delete this style.
     *
     * Calls `onStyleRemoveComponent` if accepted.
     */
    void removeStyle();

    /** @brief Fired when the label is left-clicked. */
    std::function<void(const juce::String&)> onStyleClicked;

private:
    juce::Label label;

    //JUCE_LEAK_DETECTOR(StyleViewComponent)
};


/**
 * @brief Displays a scrollable, interactive grid of StyleViewComponent items.
 *
 * Features:
 * - Add new style button + popup input dialog
 * - Rename / delete actions propagate through callbacks
 * - Auto-grid layout when resized
 */
class StylesListComponent : public juce::Component
{
public:
    /**
     * @brief Constructs list UI and populates with initial names.
     *
     * @param stylesNamesOut Initial style names to show.
     * @param onStyleClicked Callback fired when a style is clicked.
     * @param widthSize Initial width used for layout calculation.
     */
    StylesListComponent(std::vector<juce::String> stylesNames, std::function<void(const juce::String&)> onStyleClicked, int widthSize=0);

    /** @brief Fired when a rename completes. */
    std::function<void(const juce::String& oldName, const juce::String& newName)> onStyleRename;

    /** @brief Fired after a style is created. */
    std::function<void(const juce::String& newName)> onStyleAdd;

    /** @brief Fired when a style is deleted. */
    std::function<void(const juce::String& name)> onStyleRemove;

    /** @brief Arranges add button + scroll area + style tiles. */
    void resized() override;

     /** @brief Paints top toolbar background + separator line. */
    void paint(juce::Graphics& g) override;

    /** @brief Sets new width used for internal layout. */
    void setWidthSize(const int newWidth);

    /**
     * @brief Updates displayed item positions in a 2-column grid.
     *
     * Called on resize and after list changes.
     */
    void layoutStyles();

    /**
     * @brief Refreshes UI items based on current stored names.
     */
    void repopulate();

    /**
     * @brief Shows modal input dialog for adding new styles.
     *
     * Ensures:
     * - Non-empty name
     * - Name uniqueness
     *
     * Then updates list and fires callbacks.
     */
    void addNewStyle();

    /**
     * @brief Binds all callbacks for a newly created style item.
     *
     * @param newStyle Created StyleViewComponent.
     * @param currentName Temporary name used for rename validation.
     */
    void allCallBacks(StyleViewComponent* newStyle, const juce::String& currentName);

    /**
     * @brief Removes an item from the UI list before external deletion.
     *
     * Rebuilds UI and triggers `onStyleRemove`.
     */
    void removeStyleLocally(const juce::String& name);

    /** @brief Inserts a style visually without invoking dialogs. */
    void addStyleLocally(const juce::String& newName);

    /** @brief Re-builds internal names array from component list. */
    void rebuildStyleNames();

private:
    /** @brief Fully repopulates StyleViewComponents. */
    void populate();

    juce::OwnedArray<StyleViewComponent> allStyles;
    std::function<void(const juce::String&)> onStyleClicked;
    std::vector<juce::String> stylesNames;
    int widthSize;

    juce::TextButton addButton{ "Add" };

    std::unique_ptr<juce::Component> styleItemsContainer=nullptr;
    juce::Viewport viewport;

    JUCE_LEAK_DETECTOR(StylesListComponent)
};

class MyTabbedComponent : public juce::TabbedComponent
{
public:
    MyTabbedComponent(juce::TabbedButtonBar::Orientation orientation)
        : juce::TabbedComponent(orientation) {}

    void currentTabChanged(int newCurrentTabIndex, const juce::String& newTabName) override;

    std::function<void(int, juce::String)> onTabChanged;

    //JUCE_LEAK_DETECTOR(MyTabbedComponent)
};

/**
 * @brief Listener interface for receiving playback settings updates.
 */
class DisplayListener {
public:
    /** @brief Virtual destructor for safe polymorphic cleanup. */
    virtual ~DisplayListener() = default;
  
    /**
     * @brief Notifies when playback settings have changed.
     *
     * @param settings The updated playback settings.
     */
    virtual void playBackSettingsChanged(const PlayBackSettings& settings)=0;
};


/**
 * @brief Main UI container for managing styles, track lists, and playback settings.
 *
 * The Display coordinates:
 * - Style management (tabs, JSON persistence)
 * - Track selection UI and mapping
 * - MIDI output routing and playback start/stop
 * - Global playback settings propagation
 */
class Display: public  juce::Component, public juce::ChangeListener, public TrackListListener
{
public:

    /**
     * @brief Constructs a Display and links to a MIDI output device.
     *
     * @param outputDev Weak reference to the selected MIDI output.
     * @param widthForList Optional width for the track list panel.
     */
    Display(std::weak_ptr<juce::MidiOutput> outputDev, int widthForList=0);

    /** @brief Destructor. */
    ~Display() override;

    /**
     * @brief Handles JUCE change notifications triggered by child components.
     *
     * @param source Component that triggered the change.
     */
    void changeListenerCallback(juce::ChangeBroadcaster* source) override;

    /**
     * @brief Finds the index of a style tab based on its label.
     *
     * @param name Name of the style/tab.
     * @return Index if found, otherwise -1.
     */
    int getTabIndexByName(const juce::String& name);

    /** @brief Initializes styles and JSON cache on first load. */
    void initializeAllStyles();

    /**
     * @brief Retrieves the list of styles from JSON storage.
     *
     * @return A vector of style names.
     */
    std::vector<juce::String> getAllStylesFromJson();

    /** @brief Loads all styles into the tab component from JSON. */
    void loadAllStyles();

    /**
     * @brief Updates track assignments and properties for a style in JSON.
     *
     * @param name The style to update.
     */
    void updateStyleInJson(const juce::String& name);

    /**
     * @brief Renames a style entry in JSON.
     *
     * @param oldName Current name.
     * @param newName New name.
     */
    void updateStyleNameInJson(const juce::String& oldName, const juce::String& newName);

    /**
     * @brief Adds a new blank style entry to JSON.
     *
     * @param newName Name of the new style.
     */
    void appendNewStyleInJson(const juce::String& newName);

    /**
     * @brief Removes a style and its associated track references from JSON.
     *
     * @param name Name of style to remove.
     */
    void removeStyleInJson(const juce::String& name);

    /**
     * @brief Builds a UUID lookup table for all current tracks.
     *
     * @return Map of track UUIDs to track instances.
     */
    std::unordered_map<juce::Uuid, TrackEntry*> buildTrackUuidMap();

    /** @brief Inserts missing loaded tracks into the UUID lookup map. */
    void addNewTracksToMap();

    /** @brief Layout management for all child components. */
    void resized() override;

    /**
     * @brief Gets the root JSON variant storing all styles.
     *
     * @return Reference to JSON var.
     */
    const juce::var& getJsonVar();

    /**
     * @brief Switches visible tab to the selected style.
     *
     * @param name Style to make active.
     */
    void showCurrentStyleTab(const juce::String& name);

    /**
     * @brief Opens a UI to pick tracks from grouped categories.
     *
     * @param onTrackSelected Callback invoked when user chooses a track.
     */
    void showListOfTracksToSelectFrom(std::function<void(const juce::String&, const juce::Uuid& uuid, const juce::String& type)> onTrackSelected);

    /**
     * @brief Scans a folder for available track files.
     *
     * @param folder Folder to load from.
     * @return Tracks parsed from the folder.
     */
    std::vector<TrackEntry> getAvailableTracksFromFolder(const juce::File& folder);

    /**
     * @brief Updates the active MIDI output device.
     *
     * @param devOutput New output reference.
     */
    void setDeviceOutput(std::weak_ptr<juce::MidiOutput> devOutput);

    /**
     * @brief Removes a track reference from all styles.
     *
     * @param uuid The unique track identifier.
     */
    void removeTrackFromAllStyles(const juce::Uuid& uuid);

     /**
     * @brief Removes multiple tracks from all styles.
     *
     * @param uuids List of track identifiers.
     */
    void removeTracksFromAllStyles(const std::vector<juce::Uuid>& uuids);

    /**
     * @brief Updates track name wherever referenced in JSON.
     *
     * @param uuid Track to update.
     * @param newName New label for the track.
     */
    void updateTrackNameFromAllStyles(const juce::Uuid& uuid, const juce::String& newName);

    /** @brief Updates UI state when playback is stopped. */
    void stoppingPlayer();

    /** @brief Updates UI state when playback begins. */
    void startingPlayer();

    /**
     * @brief Normalizes UI when adding a new track (TrackListListener callback).
     */
    void normalizeAddingTrackCase() override;

    /**
     * @brief Sets allowed note range for playback.
     */
    void set_min_max(int min, int max);

    /**
     * @brief Stores hardware identification data for the MIDI output.
     */
    void set_VID_PID(const juce::String& VID, const juce::String& PID);

    /** @brief Loads persisted settings from JSON storage. */
    void readSettingsFromJSON();

    /** @brief Handles navigation back to the home section. */
    void homeButtonInteraction();

    /** @brief Gets the active lowest playable MIDI note. */
    int getStartNote();

    /** @brief Gets the active highest playable MIDI note. */
    int getEndNote();


    /** @brief Get left hand bound for instrument playing */
    int getLeftBound();

    /** @brief Get right hand bound for instrument playing */
    int getRightBound();

    /**
     * @brief Registers a DisplayListener.
     *
     * @param listener Pointer to listener instance.
     */
    void addListener(DisplayListener* listener);

    /**
     * @brief Removes a registered DisplayListener.
     */
    void removeListener(DisplayListener* listener);

    /** @brief Notifies listeners that playback settings changed. */
    void callingListeners();

    /**
     * @brief Gets the number of style tabs displayed.
     *
     * @return Tab count.
     */
    int getNumTabs();

    /**
     * @brief Utility callback used by playback setting sliders/spinners.
     *
     * @param value Updated UI value.
     */
    void setNewSettingsHelperFunction(int value);

private:
    juce::HashMap<juce::String, std::unique_ptr<juce::DynamicObject>> styleDataCache;
    std::unique_ptr<StylesListComponent> stylesListComponent;
    std::unique_ptr<MyTabbedComponent> tabComp;
    std::unique_ptr<CurrentStyleComponent> currentStyleComponent;
    std::unique_ptr<PlayBackSettingsComponent> playBackSettings;
    bool created = false;
    bool createdTracksTab = false;

    int minNote, maxNote;
    PlayBackSettings settings;

    std::shared_ptr<std::deque<TrackEntry>> availableTracksFromFolder;
    std::shared_ptr<std::unordered_map<juce::String, std::deque<TrackEntry>>> groupedTracks;
    std::shared_ptr<std::vector<juce::String>> groupedTrackKeys;

    std::weak_ptr<juce::MidiOutput> outputDevice;

    std::unique_ptr<TrackListComponent> trackListComp;
    juce::var allStylesJsonVar; // this has a root object, acts like a map

    std::unordered_map<juce::Uuid, TrackEntry*> mapUuidToTrack;


    juce::ListenerList<DisplayListener> displayListeners;
    //JUCE_LEAK_DETECTOR(Display)
};
