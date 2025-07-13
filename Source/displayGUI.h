/*
  ==============================================================================

    displayGUI.h
    Created: 8 Jun 2025 2:44:43am
    Author:  Kisuke

    TODO

    -remove from a track the current related file to it(making it none, not removing the actual file from the system)

    -make update names available (for track, folder, style)

    -make the timer to be instead show beat bar

    -when selecting a track from the list of 8, you can see all the notes listed from that uploaded track, aka the midi message sequence, and if possible, to put also the time stamps? in seconds

    -modify the starting time for each track

    -normalize the measure

    -normalize the tpqn

    -make bpm change mid play(not only before pressing play)

    -make it so that the the current style can be played by pressing a note(lowest) and end while highest note

    -making the sets so that you can select what to play on left hand and on right hand

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#include "TrackEntry.h"
#include "TrackPlayer.h"
#include "CustomToolTip.h"
#include "SubjectInterface.h"
#include "trackListComponentListener.h"
#include "InstrumentChooser.h"

class TrackListComponent : public juce::Component, private juce::ListBoxModel, public juce::ComboBox::Listener, public Subject<TrackListListener>
{
public:

    std::function<void(const juce::Uuid& uuid)> onRemoveTrack;
    std::function<void(const std::vector<juce::Uuid>& uuids)> onRemoveMultipleTracks;

    TrackListComponent(std::shared_ptr<std::vector<TrackEntry>> tracks,
        std::shared_ptr<std::unordered_map<juce::String, std::vector<TrackEntry>>> groupedTracksMap,
        std::shared_ptr<std::vector<juce::String>> groupedKeys,
        std::function<void(int)> onTrackChosen);

    void resized() override;

    int getNumRows() override;

    void comboBoxChanged(juce::ComboBox* comboBoxThatHasChanged) override;

    void paintListBoxItem(int rowNumber, juce::Graphics& g, int width, int height, bool rowIsSelected) override;

    void listBoxItemClicked(int row, const juce::MouseEvent& event) override;

    void addToTrackList();
    
    void removeFromTrackList();

    void addToFolderList();

    void removeFromFolderList();

    void backToFolderView();

    void saveToFile(const juce::File& fileToSave);

    void loadFromFile(const juce::File& fileToLoad);

    std::vector<TrackEntry>& getAllAvailableTracks() const;

    juce::String extractDisplayNameFromTrack(const juce::MidiMessageSequence& trackSeq);

    std::unordered_map<juce::Uuid, TrackEntry> buildTrackNameMap();

    double getOriginalBpmFromFile(const juce::MidiFile& file);

    void initializeTracksFromList();

    void deallocateTracksFromList();

    bool foundPercussion(const juce::MidiMessageSequence* sequence);


private:

    enum class ViewMode
    {
        FolderView,
        TrackView
    };

    ViewMode viewMode = ViewMode::FolderView;

    std::shared_ptr<std::vector<TrackEntry>> availableTracks;
    std::shared_ptr<std::unordered_map<juce::String, std::vector<TrackEntry>>> groupedTracks;
    std::shared_ptr<std::vector<juce::String>> groupedTrackKeys;

    std::function<void(int)> trackChosenCallBack;

    juce::ListBox listBox;


    std::unique_ptr<juce::TextButton> addButton = nullptr;
    std::unique_ptr<juce::TextButton> removeButton = nullptr;
    std::unique_ptr<juce::TextButton> backButton = nullptr;

    juce::TextButton addButtonFolder{ "Add folder" };
    juce::TextButton removeButtonFolder{ "Remove folder" };

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

class CurrentStyleComponent : public juce::Component, private juce::MouseListener
{
public:
    std::function<void()> anyTrackChanged;
    std::function<void(std::function<void(const juce::String&, const juce::Uuid& uuid, const juce::String& type)>)> onRequestTrackSelectionFromTrack;

    CurrentStyleComponent(const juce::String& name, std::unordered_map<juce::Uuid, TrackEntry>& map, juce::MidiOutput* outputDevice = nullptr);

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

    void removingTrack(const juce::Uuid& uuid);

    void removingTracks(const std::vector<juce::Uuid>& uuids);

    void setElapsedTime(double newElapsedTime);

    double getBaseTempo();

    juce::OwnedArray<Track>& getAllTracks();

    MultipleTrackPlayer* getTrackPlayer();

    void syncPercussionTracksVolumeChange(double newVolume);

private:
    void mouseDown(const juce::MouseEvent& ev) override;

    juce::String name;
    juce::Label nameOfStyle;
    juce::Label selectedTrackLabel, selectedTrackKey, selectedTrackChord;
    juce::Label elapsedTimeLabel;
    juce::OwnedArray<Track> allTracks;
    juce::ComboBox playSettingsTracks;
    juce::TextButton startPlayingTracks;
    juce::TextButton stopPlayingTracks;

    juce::MidiOutput* outputDevice = nullptr;
    std::unique_ptr<MultipleTrackPlayer> trackPlayer=nullptr;
    std::unordered_map<juce::Uuid, TrackEntry>& mapNameToTrackEntry;
    Track* lastSelectedTrack = nullptr;

    juce::Slider tempoSlider;
    int oldTempo = 120.0;
    double currentTempo = 120.0;
    double baseTempo = 120.0;
    bool isPlaying = false;

    std::unique_ptr<Track> copiedTrack=nullptr;

    //JUCE_LEAK_DETECTOR(CurrentStyleComponent)
};

class StyleViewComponent : public juce::Component, public juce::MouseListener
{
public:
    StyleViewComponent(const juce::String& styleName);

    void resized() override;

    void mouseUp(const juce::MouseEvent& event) override;

    std::function<void(const juce::String&)> onStyleClicked;

private:
    juce::Label label;

    //JUCE_LEAK_DETECTOR(StyleViewComponent)
};

class StylesListComponent : public juce::Component
{
public:
    StylesListComponent(int nrOfStyles, std::function<void(const juce::String&)> onStyleClicked, int widthSize=0);

    void resized() override;
    void setWidthSize(const int newWidth);

private:
    void populate();

    juce::OwnedArray<StyleViewComponent> allStyles;
    std::function<void(const juce::String&)> onStyleClicked;

    int nrOfStyles;
    int widthSize;

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

class Display: public  juce::Component, public juce::ChangeListener, public TrackListListener
{
public:
    Display(int widthForList=0, juce::MidiOutput* outputDev=nullptr);
    ~Display() override;

    void changeListenerCallback(juce::ChangeBroadcaster* source) override;

    int getTabIndexByName(const juce::String& name);

    void initializeAllStyles();
    void loadAllStyles();
    void updateStyleInJson(const juce::String& name);

    void resized() override;
    const juce::var& getJsonVar();

    void showCurrentStyleTab(const juce::String& name);
    void showListOfTracksToSelectFrom(std::function<void(const juce::String&, const juce::Uuid& uuid, const juce::String& type)> onTrackSelected);

    void createUserTracksFolder();
    std::vector<TrackEntry> getAvailableTracksFromFolder(const juce::File& folder);

    void setDeviceOutput(juce::MidiOutput* devOutput = nullptr);

    void removeTrackFromAllStyles(const juce::Uuid& uuid);

    void removeTracksFromAllStyles(const std::vector<juce::Uuid>& uuids);

    void stoppingPlayer();

    void updateAllStylesInJson();

    void updateUIbeforeAnyLoadingCase() override;

private:
    juce::HashMap<juce::String, std::unique_ptr<juce::DynamicObject>> styleDataCache;
    std::unique_ptr<MyTabbedComponent> tabComp;
    std::unique_ptr<CurrentStyleComponent> currentStyleComponent;
    bool created = false;
    bool createdTracksTab = false;

    std::shared_ptr<std::vector<TrackEntry>> availableTracksFromFolder;
    std::shared_ptr<std::unordered_map<juce::String, std::vector<TrackEntry>>> groupedTracks;
    std::shared_ptr<std::vector<juce::String>> groupedTrackKeys;

    juce::MidiOutput* outputDevice = nullptr;

    std::unique_ptr<TrackListComponent> trackListComp;
    juce::var allStylesJsonVar;

    std::unordered_map<juce::Uuid, TrackEntry> mapNameToTrack;

    //JUCE_LEAK_DETECTOR(Display)
};
