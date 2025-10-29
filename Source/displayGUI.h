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

class Track : public juce::Component, private juce::MouseListener, public Subject<TrackListener>
{
public:
    std::function<void()> onChange;
    std::function<void(std::function<void(const juce::String&, const juce::Uuid& uuid, const juce::String& type)>)> onRequestTrackSelection;
    std::function<bool()> isPlaying;
    std::function<void(double newVolume)> syncVolumePercussionTracks;
    std::function<void(Track* copiedTrack)> onCopy;
    std::function<void(Track* toPaste)> onPaste;
    std::function<void(const juce::Uuid& uuid, int channel)> onShowInformation;

    Track();
    ~Track();

    void resized() override;
    
    void paint(juce::Graphics& g) override;

    void mouseEnter(const juce::MouseEvent& ev) override;

    void mouseExit(const juce::MouseEvent& ev) override;

    juce::DynamicObject* getJson() const;

    void showInstantInfo(const juce::String& text);

    juce::var loadJson(const juce::File& file);

    void setVolumeSlider(double value);
    void setVolumeLabel(const juce::String& value, bool shouldNotify=false);
    void setNameLabel(const juce::String& name);
    juce::String getName() const;
    void openInstrumentChooser();
    juce::StringArray instrumentListBuild();
    void setInstrumentNumber(int newInstrumentNumber, bool shouldNotify=false);
    int getInstrumentNumber() const;
    void setUUID(const juce::Uuid& newUUID);
    void setTypeOfTrack(const juce::String& newType);
    juce::String getTypeOfTrack() const;
    juce::Uuid getUsedID() const;

    void setChannel(int newChannel);
    int getChannel() const;

    double getVolume() const;

    void copyFrom(const Track& other);
    
    void pasteFrom(const Track& source);

    void renameOneTrack();

    void deleteOneTrack();

    void copyOneTrack();

    void pasteOneTrack();

    void showNotesInformation();

private:
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

class CurrentStyleComponent : public juce::Component, private juce::MouseListener, private juce::ComboBox::Listener
{
public:
    std::function<void()> anyTrackChanged;
    std::function<void(std::function<void(const juce::String&, const juce::Uuid& uuid, const juce::String& type)>)> onRequestTrackSelectionFromTrack;
    std::function<void()> updateTrackFile;
    std::function<void()> keybindTabStarting;

    CurrentStyleComponent(const juce::String& name, std::unordered_map<juce::Uuid, TrackEntry*>& map, std::weak_ptr<juce::MidiOutput> outputDevice);

    void triggerStopClick();

    void triggerStartClick();

    void stoppingPlayer();
    
    ~CurrentStyleComponent() override;

    void resized() override;

    void updateName(const juce::String& newName);

    juce::String getName();

    juce::DynamicObject* getJson() const;

    void loadJson(const juce::var& styleVar);

    void startPlaying();

    void stopPlaying();

    double getTempo();

    void setTempo(double newTempo);

    void setStyleID(const juce::String& newID);

    void applyChangesForOneTrack(TrackEntry& track);

    void applyChangesForAllTracksCurrentStyle();

    void removingTrack(const juce::Uuid& uuid);

    void removingTracks(const std::vector<juce::Uuid>& uuids);

    void renamingTrack(const juce::Uuid& uuid, const juce::String& newName);

    void showingTheInformationNotesFromTrack(const juce::Uuid& uuid, int channel);

    double getBaseTempo();

    void setDeviceOutputCurrentStyle(std::weak_ptr<juce::MidiOutput> newOutput);

    juce::OwnedArray<Track>& getAllTracks();

    MultipleTrackPlayer* getTrackPlayer();

    void syncPercussionTracksVolumeChange(double newVolume);

    void applyBPMchangeBeforePlayback(double userBPM, bool whenLoad=false);

    void applyBPMchangeForOne(double userBPM, const juce::Uuid& uuid);

    void comboBoxChanged(juce::ComboBox* comboBoxThatHasChanged) override;

    void comboBoxChangeIndex(int Index);

private:
    void mouseDown(const juce::MouseEvent& ev) override;

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

class StyleViewComponent : public juce::Component, public juce::MouseListener
{
public:
    std::function<void(const juce::String& oldName, const juce::String& newName)> onStyleRenamed;
    std::function<bool(const juce::String& name)> isInListNames;
    std::function<void(const juce::String& name)> onStyleRemoveComponent;

    StyleViewComponent(const juce::String& styleName);

    void resized() override;

    void mouseUp(const juce::MouseEvent& event) override;

    void setNameLabel(const juce::String& name);

    juce::String getNameLabel() const;

    void changeNameLabel();

    void removeStyle();

    std::function<void(const juce::String&)> onStyleClicked;

private:
    juce::Label label;

    //JUCE_LEAK_DETECTOR(StyleViewComponent)
};

class StylesListComponent : public juce::Component
{
public:
    StylesListComponent(std::vector<juce::String> stylesNames, std::function<void(const juce::String&)> onStyleClicked, int widthSize=0);

    std::function<void(const juce::String& oldName, const juce::String& newName)> onStyleRename;
    std::function<void(const juce::String& newName)> onStyleAdd;
    std::function<void(const juce::String& name)> onStyleRemove;

    void resized() override;
    void paint(juce::Graphics& g) override;

    void setWidthSize(const int newWidth);

    void layoutStyles();

    void repopulate();

    void addNewStyle();

    void allCallBacks(StyleViewComponent* newStyle, const juce::String& currentName);

    void removeStyleLocally(const juce::String& name);

    void addStyleLocally(const juce::String& newName);

    void rebuildStyleNames();

private:
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

class DisplayListener {
public:
    virtual ~DisplayListener() = default;
    virtual void playBackSettingsChanged(const PlayBackSettings& settings)=0;
};

class Display: public  juce::Component, public juce::ChangeListener, public TrackListListener
{
public:

    Display(std::weak_ptr<juce::MidiOutput> outputDev, int widthForList=0);
    ~Display() override;

    void changeListenerCallback(juce::ChangeBroadcaster* source) override;

    int getTabIndexByName(const juce::String& name);

    void initializeAllStyles();
    std::vector<juce::String> getAllStylesFromJson();
    void loadAllStyles();
    void updateStyleInJson(const juce::String& name);
    void updateStyleNameInJson(const juce::String& oldName, const juce::String& newName);
    void appendNewStyleInJson(const juce::String& newName);
    void removeStyleInJson(const juce::String& name);

    std::unordered_map<juce::Uuid, TrackEntry*> buildTrackUuidMap();

    void addNewTracksToMap();

    void resized() override;
    const juce::var& getJsonVar();

    void showCurrentStyleTab(const juce::String& name);
    void showListOfTracksToSelectFrom(std::function<void(const juce::String&, const juce::Uuid& uuid, const juce::String& type)> onTrackSelected);

    std::vector<TrackEntry> getAvailableTracksFromFolder(const juce::File& folder);

    void setDeviceOutput(std::weak_ptr<juce::MidiOutput> devOutput);

    void removeTrackFromAllStyles(const juce::Uuid& uuid);

    void removeTracksFromAllStyles(const std::vector<juce::Uuid>& uuids);

    void updateTrackNameFromAllStyles(const juce::Uuid& uuid, const juce::String& newName);

    void stoppingPlayer();

    void startingPlayer();

    void updateUIbeforeAnyLoadingCase() override;

    void set_min_max(int min, int max);

    void set_VID_PID(const juce::String& VID, const juce::String& PID);

    void readSettingsFromJSON();

    void homeButtonInteraction();

    int getStartNote();

    int getEndNote();

    int getLeftBound();

    int getRightBound();

    void addListener(DisplayListener* listener);

    void removeListener(DisplayListener* listener);

    void callingListeners();

    int getNumTabs();

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
