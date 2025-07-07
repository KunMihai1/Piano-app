/*
  ==============================================================================

    displayGUI.h
    Created: 8 Jun 2025 2:44:43am
    Author:  Kisuke

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#include "TrackEntry.h"
#include "TrackPlayer.h"

class TrackListComponent : public juce::Component, private juce::ListBoxModel, public juce::ComboBox::Listener
{
public:

    TrackListComponent(std::vector<TrackEntry>& tracks, std::function<void(int)> onTrackChosen);

    void resized() override;

    int getNumRows() override;

    void comboBoxChanged(juce::ComboBox* comboBoxThatHasChanged) override;

    void paintListBoxItem(int rowNumber, juce::Graphics& g, int width, int height, bool rowIsSelected) override;

    void listBoxItemClicked(int row, const juce::MouseEvent& event) override;

    void addToTrackList();
    
    void removeFromTrackList();

    void saveToFile(const juce::File& fileToSave);

    void loadFromFile(const juce::File& fileToLoad);

    std::vector<TrackEntry>& getAllAvailableTracks() const;

    juce::String extractDisplayNameFromTrack(const juce::MidiMessageSequence& trackSeq);

    std::unordered_map<juce::String, TrackEntry> buildTrackNameMap();

    double getInitialTempoBPM(const juce::MidiFile& midiFile);

private:

    juce::ListBox listBox;
    std::vector<TrackEntry>& availableTracks;
    std::function<void(int)> trackChosenCallBack;

    juce::TextButton addButton{ "Add" };
    juce::TextButton removeButton{ "Remove" };
    juce::ComboBox sortComboBox;

};

class Track : public juce::Component, private juce::MouseListener
{
public:
    std::function<void()> onChange;
    std::function<void(std::function<void(const juce::String&)>)> onRequestTrackSelection;

    Track();
    ~Track();

    void resized() override;
    
    void paint(juce::Graphics& g) override;

    juce::DynamicObject* getJson() const;


    juce::var loadJson(const juce::File& file);

    void setVolumeSlider(double value);
    void setVolumeLabel(const juce::String& value);
    void setNameLabel(const juce::String& name);
    juce::String getName();

private:
    void mouseDown(const juce::MouseEvent& event) override;

    juce::Slider volumeSlider;
    juce::Label volumeLabel;
    juce::Label nameLabel;
};

class CurrentStyleComponent : public juce::Component
{
public:
    std::function<void()> anyTrackChanged;
    std::function<void(std::function<void(const juce::String&)>)> onRequestTrackSelectionFromTrack;

    CurrentStyleComponent(const juce::String& name, std::unordered_map<juce::String, TrackEntry>& map, juce::MidiOutput* outputDevice = nullptr);

    void resized() override;

    void updateName(const juce::String& newName);

    juce::String getName();

    juce::DynamicObject* getJson() const;

    void loadJson(const juce::var& styleVar);

    void startPlaying();

    void stopPlaying();


private:
    juce::String name;
    juce::Label nameOfStyle;
    juce::Label selectedTrackLabel, selectedTrackKey, selectedTrackChord;
    juce::OwnedArray<Track> allTracks;
    juce::ComboBox playSettingsTracks;
    juce::TextButton startPlayingTracks;
    juce::TextButton stopPlayingTracks;

    juce::MidiOutput* outputDevice = nullptr;
    std::unique_ptr<MultipleTrackPlayer> trackPlayer=nullptr;
    std::unordered_map<juce::String, TrackEntry>& mapNameToTrackEntry;
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
};

class MyTabbedComponent : public juce::TabbedComponent
{
public:
    MyTabbedComponent(juce::TabbedButtonBar::Orientation orientation)
        : juce::TabbedComponent(orientation) {}

    void currentTabChanged(int newCurrentTabIndex, const juce::String& newTabName) override;

    std::function<void(int, juce::String)> onTabChanged;
};

class Display: public  juce::Component, public juce::ChangeListener
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
    void showListOfTracksToSelectFrom(std::function<void(const juce::String&)> onTrackSelected);

    void createUserTracksFolder();
    std::vector<TrackEntry> getAvailableTracksFromFolder(const juce::File& folder);

    void setDeviceOutput(juce::MidiOutput* devOutput = nullptr);

private:
    juce::HashMap<juce::String, std::unique_ptr<juce::DynamicObject>> styleDataCache;
    std::unique_ptr<MyTabbedComponent> tabComp;
    std::unique_ptr<CurrentStyleComponent> currentStyleComponent;
    bool created = false;
    bool createdTracksTab = false;
    std::vector<TrackEntry> availableTracksFromFolder;
    juce::MidiOutput* outputDevice = nullptr;

    std::unique_ptr<TrackListComponent> trackListComp;
    juce::var allStylesJsonVar;

    std::unordered_map<juce::String, TrackEntry> mapNameToTrack;
};
