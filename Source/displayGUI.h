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

class TrackListComponent : public juce::Component, private juce::ListBoxModel, public juce::ComboBox::Listener, public Subject<TrackListListener>
{
public:

    std::function<void(const juce::Uuid& uuid)> onRemoveTrack;
    std::function<void(const std::vector<juce::Uuid>& uuids)> onRemoveMultipleTracks;
    std::function<void(const juce::Uuid& uuid, const juce::String& newName)> onRenameTrackFromList;
    std::function<void(TrackEntry* newEntry)> addToMapOnAdding;

    TrackListComponent(std::shared_ptr<std::deque<TrackEntry>> tracks,
        std::shared_ptr<std::unordered_map<juce::String, std::deque<TrackEntry>>> groupedTracksMap,
        std::shared_ptr<std::vector<juce::String>> groupedKeys,
        std::function<void(int)> onTrackChosen);

    void resized() override;

    int getNumRows() override;

    void comboBoxChanged(juce::ComboBox* comboBoxThatHasChanged) override;

    void paintListBoxItem(int rowNumber, juce::Graphics& g, int width, int height, bool rowIsSelected) override;

    void listBoxItemClicked(int row, const juce::MouseEvent& event) override;

    void addToTrackList();
    
    void removeFromTrackList();

    void renameFromTrackList();

    void addToFolderList();

    void removeFromFolderList();

    void renameFromFolderList();

    void backToFolderView();

    std::deque<TrackEntry>& getAllAvailableTracks() const;

    juce::String extractDisplayNameFromTrack(const juce::MidiMessageSequence& trackSeq);

    void initializeTracksFromList();

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

    void normalizeAllTracks();

    void normalizeMeasure(juce::MidiMessageSequence& seq, double tpqn, double bpm, int beatsPerBar=4);

    void normalizeTPQN(juce::MidiMessageSequence& seq, double originalTPQN, double targetTPQN);

    void stripTempoMetaEvents(juce::MidiMessageSequence& seq);

private:
    void mouseDown(const juce::MouseEvent& ev) override;

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
