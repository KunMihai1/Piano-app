/*
  ==============================================================================

    displayGUI.cpp
    Created: 8 Jun 2025 2:44:43am
    Author:  Kisuke

  ==============================================================================
*/

#include "displayGUI.h"
#include "InstrumentChooser.h"


void convertTicksToSeconds(juce::MidiFile& midiFile, double bpm = 120.0)
{
    int tpqn = midiFile.getTimeFormat();
    if (tpqn <= 0)
        tpqn = 960;

    const double defaultTempoBPM = bpm; //k so elapsed time need to match this btw TODO
    const double secondsPerQuarterNote = 60.0 / defaultTempoBPM;
    const double secondsPerTick = secondsPerQuarterNote / double(tpqn);

    for (int t = 0; t < midiFile.getNumTracks(); ++t)
    {
        if (auto* seq = midiFile.getTrack(t))
        {
            for (int e = 0; e < seq->getNumEvents(); ++e)
            {
                auto& msg = seq->getEventPointer(e)->message;
                double ticks = msg.getTimeStamp();
                msg.setTimeStamp(ticks * secondsPerTick);
            }
        }
    }
}

Display::Display(int widthForList, juce::MidiOutput* outputDev) : outputDevice{ outputDev }
{
    availableTracksFromFolder = std::make_shared<std::vector<TrackEntry>>();
    groupedTracks = std::make_shared<std::unordered_map<juce::String, std::vector<TrackEntry>>>();
    groupedTrackKeys = std::make_shared<std::vector<juce::String>>();

    tabComp = std::make_unique<MyTabbedComponent>(juce::TabbedButtonBar::TabsAtLeft);
    addAndMakeVisible(tabComp.get());

    initializeAllStyles();
    loadAllStyles();

    std::vector<juce::String> stylesNames = getAllStylesFromJson();

    auto* list = new StylesListComponent{ stylesNames, [this](const juce::String& name) {
        showCurrentStyleTab(name); },widthForList
    };

    list->onStyleRename = [this](const juce::String& oldName, const juce::String& newName)
    {
        updateStyleNameInJson(oldName, newName);
    };

    auto* scrollableView = new juce::Viewport();
    scrollableView->setScrollBarsShown(true, false); // only vertical
    scrollableView->setViewedComponent(list, true);
    tabComp->addTab("Styles", juce::Colour::fromRGB(10, 15, 10), scrollableView, true);

    list->resized();
    scrollableView->getViewedComponent()->resized();

    tabComp->onTabChanged = [this](int newIndex, juce::String tabName)
    {
        if (createdTracksTab && tabName != "My tracks")
        {
            if (trackListComp)
            {
                trackListComp->removeListener(this);
            }
            int myTracksIndex = tabComp->getNumTabs() - 1;
            tabComp->removeTab(myTracksIndex);

            //removeChildComponent(trackListComp.get());
            trackListComp.reset();
            createdTracksTab = false;
            
        }
    };

    auto appDataFolder = juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
        .getChildFile("Piano Synth2");

    auto jsonFile = appDataFolder.getChildFile("myTracks.json");
    if (!jsonFile.exists())
    {
        juce::var emptyArray = juce::Array<juce::var>{};
        juce::String jsonString = juce::JSON::toString(emptyArray);

        jsonFile.replaceWithText(jsonString);
    }

    trackListComp = std::make_unique<TrackListComponent>(
        availableTracksFromFolder,
        groupedTracks,
        groupedTrackKeys,
        [](int) {}
    );
    trackListComp->loadFromFile(jsonFile);

    mapNameToTrack = trackListComp->buildTrackNameMap();
}

Display::~Display()
{

}

void Display::changeListenerCallback(juce::ChangeBroadcaster* source)
{
    auto* tabbedCast = dynamic_cast<juce::TabbedComponent*>(source);
    if (tabbedCast == tabComp.get())
    {
        int currentIndex = tabComp->getCurrentTabIndex();
        int tracksTabIndex = getTabIndexByName("My tracks");

        if (tracksTabIndex >= 0 && currentIndex != tracksTabIndex)
        {
            tabComp->removeTab(tracksTabIndex);
        }
    }
}

int Display::getTabIndexByName(const juce::String& name)
{
    auto& tabBar = tabComp->getTabbedButtonBar();

    for (int i = 0; i < tabComp->getNumTabs(); ++i)
    {
        if (tabBar.getTabButton(i)->getName() == name)
            return i;
    }
    return -1;
}

void Display::showCurrentStyleTab(const juce::String& name)
{
    if (!created)
    {
        currentStyleComponent = std::make_unique<CurrentStyleComponent>(name, mapNameToTrack, outputDevice);
        currentStyleComponent->onRequestTrackSelectionFromTrack = [this](std::function<void(const juce::String&, const juce::Uuid& uuid, const juce::String& type)> trackChosenCallback)
        {
            showListOfTracksToSelectFrom(trackChosenCallback);
        };
        tabComp->addTab(name, juce::Colour::fromRGB(10, 15, 10), currentStyleComponent.get(), true); //release, get potential issue
        created = true;
    }
    else
    {
        for (auto& track : currentStyleComponent->getAllTracks())
            track->removeListener(currentStyleComponent->getTrackPlayer());

        int index = tabComp->getNumTabs() - 1;
        tabComp->getTabbedButtonBar().setTabName(index, name);
        currentStyleComponent->updateName(name);
    }
    currentStyleComponent->anyTrackChanged = [this, name]()
    {
        updateStyleInJson(name);
    };

    if (allStylesJsonVar.isObject())
    {
        auto stylesArray = allStylesJsonVar["styles"];
        if (stylesArray.isArray())
        {
            for (const auto& style : *stylesArray.getArray())
            {
                if (auto* obj = style.getDynamicObject())
                {
                    if (obj->getProperty("name").toString() == name)
                    {
                        currentStyleComponent->loadJson(style);
                        break;
                    }
                }
            }
        }
    }

    tabComp->setCurrentTabIndex(tabComp->getNumTabs() - 1);
}

void Display::showListOfTracksToSelectFrom(std::function<void(const juce::String&, const juce::Uuid& uuid, const juce::String& type)> onTrackSelected)
{
    trackListComp = std::make_unique<TrackListComponent>(availableTracksFromFolder, groupedTracks, groupedTrackKeys,
        [this, onTrackSelected](int index)
        {
            auto& track = (*availableTracksFromFolder)[index];
            juce::String typeString = TrackTypeConversion::toString(track.type);

            onTrackSelected(track.getDisplayName(), track.getUniqueID(), typeString);

            if (mapNameToTrack.empty())
                mapNameToTrack = trackListComp->buildTrackNameMap();

            if (trackListComp)
                trackListComp->removeListener(this);
            int in = tabComp->getNumTabs() - 2;
            if (in >= 0)
            {
                updateStyleInJson(currentStyleComponent->getName());
                tabComp->setCurrentTabIndex(in);
            }
        });
    trackListComp->addListener(this);

    trackListComp->onRemoveTrack = [this](const juce::Uuid& uuid)
    {
        removeTrackFromAllStyles(uuid);
    };

    trackListComp->onRemoveMultipleTracks = [this](const std::vector<juce::Uuid>& uuids)
    {
        removeTracksFromAllStyles(uuids);
    };
    tabComp->addTab("My tracks", juce::Colour::fromRGB(10, 15, 10), trackListComp.get(), false);
    tabComp->setCurrentTabIndex(tabComp->getNumTabs() - 1);

    createdTracksTab = true;
}

void Display::createUserTracksFolder()
{
    auto userTracksFolder = juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
        .getChildFile("Piano Synth2")
        .getChildFile("UserTracks");

    if (!userTracksFolder.exists())
    {
        userTracksFolder.createDirectory();
    }
}

std::vector<TrackEntry> Display::getAvailableTracksFromFolder(const juce::File& folder)
{
    std::vector<TrackEntry> tracks;

    if (!folder.exists() || !folder.isDirectory())
        return tracks;

    juce::DirectoryIterator iter(folder, false, "*.mid", juce::File::findFiles);

    while (iter.next())
    {
        juce::File trackFile = iter.getFile();
        TrackEntry tr;
        tr.file = trackFile;
        tracks.push_back(tr);
    }

    return tracks;
}

void Display::setDeviceOutput(juce::MidiOutput* devOutput)
{
    this->outputDevice = devOutput;
}

void Display::stoppingPlayer()
{
    if (currentStyleComponent)
        this->currentStyleComponent->stoppingPlayer();
}

void Display::updateUIbeforeAnyLoadingCase()
{
    DBG("sUNT IN CAZUL ASTA");
    //if (this->mapNameToTrack.empty())
    //    mapNameToTrack = trackListComp->buildTrackNameMap();
}

void Display::removeTrackFromAllStyles(const juce::Uuid& uuid)
{
    if (!allStylesJsonVar.isObject())
        return;

    auto stylesArray = allStylesJsonVar["styles"];

    if (!stylesArray.isArray())
        return;

    for (auto& styleVariable : *stylesArray.getArray())
    {
        auto* styleObj = styleVariable.getDynamicObject();

        if (styleObj == nullptr)
            continue;

        juce::var trackVariable = styleObj->getProperty("tracks");

        if (!trackVariable.isArray())
            continue;

        auto* trackArray = trackVariable.getArray();

        for (auto& trackVar : *trackArray)
        {
            auto* trackObj = trackVar.getDynamicObject();

            if (trackObj == nullptr)
                continue;

            juce::String uuidString = trackObj->getProperty("uuid").toString();
            if (uuidString == uuid.toString())
            {
                trackObj->setProperty("uuid", "noID");
                trackObj->setProperty("name", "None");
                trackObj->setProperty("instrumentNumber", -1);
            }
        }
    }

    if (currentStyleComponent)
    {
        currentStyleComponent->removingTrack(uuid);
    }

    juce::File appDataFolder = juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
        .getChildFile("Piano Synth2");

    juce::File jsonFile = appDataFolder.getChildFile("allStyles.json");

    juce::String jsonString = juce::JSON::toString(allStylesJsonVar);
    jsonFile.replaceWithText(jsonString);
}

void Display::removeTracksFromAllStyles(const std::vector<juce::Uuid>& uuids)
{
    if (!allStylesJsonVar.isObject())
        return;

    auto stylesArray = allStylesJsonVar["styles"];

    if (!stylesArray.isArray())
        return;

    std::unordered_set<juce::String> uuidSet;
    for (const auto& uuid : uuids)
        uuidSet.insert(uuid.toString());

    for (auto& styleVariable : *stylesArray.getArray())
    {
        auto styleObj = styleVariable.getDynamicObject();
        if (styleObj == nullptr)
            continue;

        juce::var trackVariable = styleObj->getProperty("tracks");

        if (!trackVariable.isArray())
            continue;

        for (auto& trackVar : *trackVariable.getArray())
        {
            auto trackObj = trackVar.getDynamicObject();
            if (trackObj == nullptr)
                continue;

            juce::String uuidString = trackObj->getProperty("uuid").toString();
            if (uuidSet.count(uuidString))
            {
                trackObj->setProperty("uuid", "noID");
                trackObj->setProperty("name", "None");
                trackObj->setProperty("instrumentNumber", -1);
            }
        }
    }

    if (currentStyleComponent)
    {
        currentStyleComponent->removingTracks(uuids);
    }

    juce::File appDataFolder = juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
        .getChildFile("Piano Synth2");

    juce::File jsonFile = appDataFolder.getChildFile("allStyles.json");

    juce::String jsonString = juce::JSON::toString(allStylesJsonVar);
    jsonFile.replaceWithText(jsonString);
}

void Display::initializeAllStyles()
{
    juce::File appDataFolder = juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
        .getChildFile("Piano Synth2");

    if (!appDataFolder.exists())
        appDataFolder.createDirectory();

    auto file = appDataFolder.getChildFile("allStyles.json");

    if (file.exists())
        return;

    juce::Array<juce::var> stylesArray;

    for (int i = 0; i < 10; ++i)
    {
        auto* styleObj = new juce::DynamicObject{};
        juce::String styleName = "Style " + juce::String(i + 1);
        styleObj->setProperty("name", styleName);
        styleObj->setProperty("BPM", 120.0f);

        juce::Array<juce::var> tracksArray;
        for (int t = 0; t < 8; ++t)
        {
            auto* trackObj = new juce::DynamicObject{};
            trackObj->setProperty("name", "None");
            trackObj->setProperty("type", "None");
            trackObj->setProperty("volume", 50.0f);
            trackObj->setProperty("instrumentNumber", -1);
            trackObj->setProperty("uuid", "noID");
            tracksArray.add(trackObj);
        }
        styleObj->setProperty("tracks", tracksArray);
        stylesArray.add(styleObj);
    }

    auto* rootObj = new juce::DynamicObject{};
    rootObj->setProperty("styles", stylesArray);

    juce::var jsonVar(rootObj);  // var now owns rootObj pointer

    juce::String jsonString = juce::JSON::toString(jsonVar);

    file.replaceWithText(jsonString);
}

std::vector<juce::String> Display::getAllStylesFromJson()
{
    std::vector<juce::String> allStylesNames;

    if (!allStylesJsonVar.isObject())
        return allStylesNames;

    auto stylesArray = allStylesJsonVar["styles"];
    if (!stylesArray.isArray())
        return allStylesNames;

    for (auto& styleVar : *stylesArray.getArray())
    {
        auto* obj = styleVar.getDynamicObject();
        if (obj == nullptr)
            continue;

        juce::String name = obj->getProperty("name").toString();
        if (name.isNotEmpty())
            allStylesNames.push_back(name);
    }

    return allStylesNames;
}

void Display::loadAllStyles()
{
    juce::File appDataFolder = juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
        .getChildFile("Piano Synth2");

    auto file = appDataFolder.getChildFile("allStyles.json");

    if (!file.exists())
    {
        allStylesJsonVar = juce::var();
        return;
    }

    juce::String jsonString = file.loadFileAsString();
    allStylesJsonVar = juce::JSON::parse(jsonString);
}

void Display::updateStyleInJson(const juce::String& name)
{
    if (!allStylesJsonVar.isObject())
        return;

    auto stylesArray = allStylesJsonVar["styles"];

    if (!stylesArray.isArray())
        return;

    for (int i = 0; i < stylesArray.getArray()->size(); ++i)
    {
        auto& styleVar = stylesArray.getArray()->getReference(i);
        if (auto* obj = styleVar.getDynamicObject())
        {
            if (obj->getProperty("name").toString() == name)
            {
                styleVar = juce::var(currentStyleComponent->getJson());
                break;
            }
        }
    }

    juce::File appDataFolder = juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
        .getChildFile("Piano Synth2");
    auto file = appDataFolder.getChildFile("allStyles.json");

    juce::String jsonString = juce::JSON::toString(allStylesJsonVar);
    file.replaceWithText(jsonString);
}

void Display::updateStyleNameInJson(const juce::String& oldName, const juce::String& newName)
{
    if (!allStylesJsonVar.isObject())
        return;

    auto stylesArray = allStylesJsonVar["styles"];

    if (!stylesArray.isArray())
        return;

    for (int i = 0; i < stylesArray.getArray()->size(); i++)
    {
        auto& styleVar = stylesArray.getArray()->getReference(i);
        auto* obj = styleVar.getDynamicObject();
        if (obj == nullptr)
            continue;

        if (obj->getProperty("name").toString() == oldName)
        {
            obj->setProperty("name", newName);
            break;
        }
    }

    juce::File appDataFolder = juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
        .getChildFile("Piano Synth2");
    juce::File jsonFile = appDataFolder.getChildFile("allStyles.json");

    juce::String jsonString = juce::JSON::toString(allStylesJsonVar);
    jsonFile.replaceWithText(jsonString);
}

void Display::resized()
{
    tabComp->setBounds(getLocalBounds());

}

const juce::var& Display::getJsonVar()
{
    return allStylesJsonVar;
}

StyleViewComponent::StyleViewComponent(const juce::String& styleName)
{
    label.setFont(juce::Font(18.0f, juce::Font::bold));
    label.setText(styleName, juce::dontSendNotification);
    label.setColour(juce::Label::textColourId, juce::Colours::cyan);
    label.setInterceptsMouseClicks(true, false);
    label.setMouseCursor(juce::MouseCursor::PointingHandCursor);

    addAndMakeVisible(label);

    label.addMouseListener(this, false);

}

void StyleViewComponent::resized()
{
    auto font = label.getFont();
    int textWidth = static_cast<int>(font.getStringWidth(label.getText()));
    int textHeight = static_cast<int>(font.getHeight());

    label.setBounds(10, 10, textWidth + 10, textHeight + 6);
}

void StyleViewComponent::mouseUp(const juce::MouseEvent& event)
{
    if (event.eventComponent == &label && onStyleClicked)
    {
        if (event.mods.isLeftButtonDown())
        {
            if(onStyleClicked)
                onStyleClicked(label.getText());
        }
        else if (event.mods.isRightButtonDown())
        {
            juce::PopupMenu menu;
            menu.addItem("Rename", [this]() {
                changeNameLabel();
                });

            menu.showMenuAsync(juce::PopupMenu::Options().withTargetComponent(this));
        }
    }
}

void StyleViewComponent::setNameLabel(const juce::String& name)
{
    label.setText(name, juce::dontSendNotification);
}

void StyleViewComponent::changeNameLabel()
{
    auto window = new juce::AlertWindow{ "Rename track","Enter a name for your track",juce::AlertWindow::NoIcon };

    window->addTextEditor("nameEditor", label.getText(), "Style Name:");
    window->addButton("OK", 1, juce::KeyPress(juce::KeyPress::returnKey));
    window->addButton("Cancel", 0, juce::KeyPress(juce::KeyPress::escapeKey));

    window->enterModalState(true, juce::ModalCallbackFunction::create([this, window](int result)
        {
            std::unique_ptr<juce::AlertWindow> cleanup{ window };
            if (result != 1)
                return;

            juce::String oldName = label.getText();
            juce::String theNewName = window->getTextEditor("nameEditor")->getText().trim();

            if (theNewName.isEmpty())
            {
                juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::InfoIcon, "Rename style", "You can't put an empty name!");
                return;
            }

            if (isInListNames)
            {
                if (isInListNames(theNewName))
                {
                    juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::InfoIcon, "Rename style", "The name already exists!");
                    return;
                }
            }

            setNameLabel(theNewName);

            if (onStyleRenamed)
                onStyleRenamed(oldName, theNewName);

            
        }
    ));
}

StylesListComponent::StylesListComponent(std::vector<juce::String>& stylesNames, std::function<void(const juce::String&)> onStyleClicked, int widthSize) : stylesNames{stylesNames}, onStyleClicked{onStyleClicked}
{
    addAndMakeVisible(addButton);
    addAndMakeVisible(removeButton);

    addButton.setMouseCursor(juce::MouseCursor::PointingHandCursor);
    removeButton.setMouseCursor(juce::MouseCursor::PointingHandCursor);

    addButton.onClick = [this]()
    {

    };

    removeButton.onClick = [this]()
    {

    };  

    if (stylesNames.empty())
        nrOfStyles = 10;
    else nrOfStyles = stylesNames.size();
    this->widthSize = widthSize;
    populate();
}

void StylesListComponent::resized()
{

    this->widthSize = getWidth();


    const int itemWidth = getWidth() / 2 - 10;   // Half of parent initial parent width - with spacing
    const int itemHeight = 50;
    const int columns = 2;
    const int spacing = 10;

    int x = spacing;
    int y = spacing;
    int count = 0;

    for (auto* style : allStyles)
    {
        style->setBounds(x, y, itemWidth, itemHeight);

        count++;
        if (count % columns == 0)
        {
            x = spacing;
            y += itemHeight + spacing;
        }
        else
        {
            x += itemWidth + spacing;
        }
    }
}

void StylesListComponent::setWidthSize(const int newWidth)
{
    this->widthSize = newWidth;
}

void StylesListComponent::populate()
{
    const int itemHeight = 50;
    const int spacing = 10;

    for (int i = 0; i < nrOfStyles; i++)
    {
        juce::String name;
        if (i < stylesNames.size())
            name = stylesNames[i];
        else name = "Style "+juce::String(i);
        auto* newStyle = new StyleViewComponent{name};
        newStyle->onStyleClicked = this->onStyleClicked;
        newStyle->onStyleRenamed = [this](const juce::String& oldName, const juce::String& newName)
        {
            onStyleRename(oldName, newName);
        };

        juce::String currentName = name;
        newStyle->isInListNames = [this,currentName](const juce::String& newName)
        {
            bool isIn=false;
            for (auto& nameInList : stylesNames)
            {
                if (nameInList == newName && newName!=currentName)
                {
                    isIn = true;
                    break;
                }
            }
                    

            return isIn;
        };
        allStyles.add(newStyle);
        addAndMakeVisible(newStyle);
    }

    // Very narrow width; Viewport will stretch it later
    const int totalHeight = nrOfStyles * (itemHeight + spacing) + spacing;
    setSize(widthSize, totalHeight / 2);  // key fix: force vertical scrolling
}


void CurrentStyleComponent::startPlaying()
{
    int selectedID = playSettingsTracks.getSelectedId();
    stopPlaying();

    std::vector<TrackEntry> selectedTracks;

    if (outputDevice == nullptr)
    {
        DBG("Output is null!");
        return;
    }


    if (selectedID == 1)
    {
        for (const auto& tr : allTracks)
        {
            auto it = mapNameToTrackEntry.find(tr->getUsedID());
            if (it != mapNameToTrackEntry.end())
            {
                DBG("asta e bun ba");
                it->second.instrumentAssociated = tr->getInstrumentNumber();
                it->second.volumeAssociated = tr->getVolume();
                selectedTracks.push_back(it->second);
            }
        }
    }
    else if (selectedID == 2)
    {
        if (lastSelectedTrack)
        {
            auto it = mapNameToTrackEntry.find(lastSelectedTrack->getUsedID());
            if (it != mapNameToTrackEntry.end())
                selectedTracks.push_back(it->second);
        }
    }
    if (selectedTracks.empty())
    {
        juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::WarningIcon, "Playing tracks", "There are no tracks to play");
        return;
    }
    trackPlayer->setTracks(selectedTracks);
    trackPlayer->applyBPMchangeBeforePlayback(currentTempo);

    trackPlayer->syncPlaybackSettings();
    trackPlayer->start();
}

CurrentStyleComponent::CurrentStyleComponent(const juce::String& name, std::unordered_map<juce::Uuid, TrackEntry>& map, juce::MidiOutput* outputDevice) : name(name), mapNameToTrackEntry(map), outputDevice(outputDevice)
{
    addMouseListener(this, true);
    nameOfStyle.setText(name, juce::dontSendNotification);
    nameOfStyle.setTooltip(name);
    addAndMakeVisible(nameOfStyle);

    playSettingsTracks.setMouseCursor(juce::MouseCursor::PointingHandCursor);
    playSettingsTracks.addItem("All tracks", 1);
    playSettingsTracks.addItem("Solo track(last selected)", 2);
    playSettingsTracks.setSelectedId(1);

    addAndMakeVisible(playSettingsTracks);
    addAndMakeVisible(startPlayingTracks);
    addAndMakeVisible(stopPlayingTracks);
    addAndMakeVisible(elapsedTimeLabel);
    elapsedTimeLabel.setText("00:00", juce::dontSendNotification);

    juce::Colour textColour = juce::Colour::fromRGB(0, 255, 0)
        .withAlpha(0.8f);
    elapsedTimeLabel.setColour(juce::Label::textColourId, textColour);

    trackPlayer = std::make_unique<MultipleTrackPlayer>(outputDevice);

    trackPlayer->onElapsedUpdate = [this](double ElapsedTime)
    {
        setElapsedTime(ElapsedTime);
    };

    startPlayingTracks.onClick = [this]
    {
        isPlaying = true;
        startPlaying();
    };

    stopPlayingTracks.onClick = [this]
    {
        stopPlaying();
    };

    for (int i = 0; i < 8; i++)
    {
        auto* newTrack = new Track{};
        newTrack->onChange = [this]()
        {
            if (anyTrackChanged)
                anyTrackChanged();

        };

        newTrack->onCopy = [this](Track* copied)
        {
            copiedTrack = std::make_unique<Track>();
            copiedTrack->copyFrom(*copied);
        };

        newTrack->onPaste = [this](Track* toPaste)
        {
            if (copiedTrack == nullptr)
            {
                juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::InfoIcon, "Pasting a track", "First you have to copy!");
                return;
            }

            toPaste->pasteFrom(*copiedTrack.get());
            
        };

        newTrack->syncVolumePercussionTracks = [this](double newVolume) {
            syncPercussionTracksVolumeChange(newVolume);
        };

        newTrack->onRequestTrackSelection = [this](std::function<void(const juce::String&, const juce::Uuid& uuid, const juce::String& type)> trackChosenCallback)
        {
            if (onRequestTrackSelectionFromTrack)
                onRequestTrackSelectionFromTrack(trackChosenCallback);
        };

        newTrack->isPlaying = [this]() {
            return isPlaying;
        };

        addAndMakeVisible(newTrack);
        allTracks.add(newTrack);
    }

    tempoSlider.setRange(60.0, 200.0, 1.0);
    tempoSlider.setSkewFactorFromMidPoint(120.0);
    tempoSlider.setValue(currentTempo);
    tempoSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    tempoSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 50, 20);

    tempoSlider.setColour(juce::Slider::trackColourId, juce::Colours::cornflowerblue);
    tempoSlider.setColour(juce::Slider::backgroundColourId, juce::Colours::darkgrey);
    tempoSlider.setMouseCursor(juce::MouseCursor::LeftRightResizeCursor);
    tempoSlider.setColour(juce::Slider::thumbColourId, juce::Colours::orange);
    tempoSlider.textFromValueFunction = [](double value)
    {
        return juce::String(static_cast<int>(value)) + " BPM";
    };
    tempoSlider.setValue(tempoSlider.getValue(), juce::sendNotificationSync);
    tempoSlider.updateText();

    tempoSlider.onValueChange = [this]()
    {
        oldTempo = currentTempo;
        currentTempo = tempoSlider.getValue();
    };

    tempoSlider.onDragEnd = [this]()
    {
        if (anyTrackChanged)
            anyTrackChanged();
    };

    addAndMakeVisible(tempoSlider);
}

void CurrentStyleComponent::stopPlaying()
{
    if (trackPlayer)
        trackPlayer->stop();
}

double CurrentStyleComponent::getTempo()
{
    return currentTempo;
}

double CurrentStyleComponent::getBaseTempo()
{
    return baseTempo;
}

void CurrentStyleComponent::removingTrack(const juce::Uuid& uuid)
{
    for (auto& track : allTracks)
    {
        if (track->getUsedID() == uuid)
        {
            track->setNameLabel("None");
            track->setUUID(juce::Uuid{ "noID" });
        }
    }
}

void CurrentStyleComponent::removingTracks(const std::vector<juce::Uuid>& uuids)
{
    for (auto& track : allTracks)
    {
        auto trackUUID = track->getUsedID();
        for (auto& uuid : uuids)
        {
            if (trackUUID == uuid)
            {
                track->setNameLabel("None");
                track->setUUID(juce::Uuid{ "noID" });
                break;
            }
        }
    }
}

void CurrentStyleComponent::setElapsedTime(double newTimeToSet)
{
    int totalSeconds = static_cast<int>(newTimeToSet);

    int minutes = totalSeconds / 60;
    int seconds = totalSeconds % 60;
    juce::String formattedTime = juce::String::formatted("%02d:%02d", minutes, seconds);

    this->elapsedTimeLabel.setText(formattedTime, juce::dontSendNotification);
}

juce::OwnedArray<Track>& CurrentStyleComponent::getAllTracks()
{
    return allTracks;
}

MultipleTrackPlayer* CurrentStyleComponent::getTrackPlayer()
{
    return trackPlayer.get();
}

void CurrentStyleComponent::syncPercussionTracksVolumeChange(double newVolume)
{
    for (auto& track : allTracks)
    {
        if (track->getTypeOfTrack() == TrackTypeConversion::toString(TrackType::Percussion))
        {
            if (track->getVolume() != newVolume)
                track->setVolumeSlider(newVolume);
        }
    }
}

void CurrentStyleComponent::mouseDown(const juce::MouseEvent& ev)
{
    auto* clickedComponent = ev.originalComponent;

    while (clickedComponent != nullptr)
    {
        auto* clickedTrack = dynamic_cast<Track*>(clickedComponent);
        if (allTracks.contains(clickedTrack))
        {
            lastSelectedTrack = clickedTrack;
            return;
        }
        clickedComponent = clickedComponent->getParentComponent();
    }
}

void CurrentStyleComponent::setTempo(double newTempo)
{
    oldTempo = currentTempo;
    this->currentTempo = newTempo;
    this->tempoSlider.setValue(newTempo);
}


void CurrentStyleComponent::resized()
{
    int labelWidth = 50;
    int playSettingsWidth = getWidth() / 6 + 30;
    int heightFirst = 20;
    int startStopWidth = 35;
    playSettingsTracks.setBounds(getWidth() - playSettingsWidth, 0, playSettingsWidth, heightFirst);
    playSettingsTracks.setMouseCursor(juce::MouseCursor::PointingHandCursor);

    nameOfStyle.setBounds((getWidth() - labelWidth) / 2 - 40, 0, getWidth() / 6, heightFirst);
    startPlayingTracks.setBounds(0, 0, startStopWidth, heightFirst);
    startPlayingTracks.setButtonText("Start");
    startPlayingTracks.setMouseCursor(juce::MouseCursor::PointingHandCursor);

    stopPlayingTracks.setBounds(startPlayingTracks.getWidth() + 5, 0, startStopWidth, heightFirst);
    stopPlayingTracks.setButtonText("Stop");
    stopPlayingTracks.setMouseCursor(juce::MouseCursor::PointingHandCursor);

    elapsedTimeLabel.setBounds(stopPlayingTracks.getX() + stopPlayingTracks.getWidth(), 0, 50, heightFirst);

    //nameOfStyle.setBounds(stopPlayingTracks.getX() + stopPlayingTracks.getWidth() + 5, 0, getWidth() / 6, heightFirst);

    int sliderWidth = playSettingsTracks.getX() - (nameOfStyle.getX() + nameOfStyle.getWidth()) + 10;
    tempoSlider.setBounds(nameOfStyle.getX() + nameOfStyle.getWidth() - 15, 0, sliderWidth, 18);

    float width = getWidth() / 8.0f, height = 80.0f;
    float initialX = 0, y = getHeight() - height;

    for (int i = 0; i < allTracks.size(); i++)
    {
        allTracks[i]->setBounds(juce::Rectangle<int>(
            static_cast<int>(i * width),
            static_cast<int>(y),
            static_cast<int>(width),
            static_cast<int>(height)
            ));
    }
}

void CurrentStyleComponent::updateName(const juce::String& newName)
{
    name = newName;
    nameOfStyle.setText(name, juce::dontSendNotification);
}

CurrentStyleComponent::~CurrentStyleComponent()
{
    if (trackPlayer)
    {
        for (auto& track : allTracks)
            track->removeListener(trackPlayer.get());
    }
}

juce::String CurrentStyleComponent::getName()
{
    return name;
}

void CurrentStyleComponent::stoppingPlayer()
{
    if (trackPlayer)
    {
        this->trackPlayer->stop();
        trackPlayer.reset();
    }
}

juce::DynamicObject* CurrentStyleComponent::getJson() const
{
    auto* styleObj = new juce::DynamicObject{};

    styleObj->setProperty("name", name);
    styleObj->setProperty("BPM", currentTempo);

    juce::Array<juce::var> tracksArray;

    for (auto* track : allTracks)
    {
        tracksArray.add(juce::var(track->getJson()));
    }

    styleObj->setProperty("tracks", tracksArray);

    return styleObj;
}

void CurrentStyleComponent::loadJson(const juce::var& styleVar)
{
    if (!styleVar.isObject())
        return;


    auto* obj = styleVar.getDynamicObject();
    if (!obj)
        return;

    updateName(obj->getProperty("name").toString());
    setTempo(static_cast<double>(obj->getProperty("BPM")));

    auto tracksVar = obj->getProperty("tracks");

    if (!tracksVar.isArray())
        return;

    auto* tracksArray = tracksVar.getArray();
    int current = 2;

    for (int i = 0; i < allTracks.size() && i < tracksArray->size(); ++i)
    {
        auto trackVar = tracksArray->getReference(i);
        if (!trackVar.isObject())
            continue;

        auto* trackObj = trackVar.getDynamicObject();
        if (!trackObj)
            continue;

        auto name = trackObj->getProperty("name").toString();
        double volume = static_cast<double>(trackObj->getProperty("volume"));
        int instrumentNumber = static_cast<int>(trackObj->getProperty("instrumentNumber"));

        auto uuidSTR = trackObj->getProperty("uuid");

        juce::Uuid uuid{ uuidSTR };
        allTracks[i]->addListener(trackPlayer.get());
        auto type = trackObj->getProperty("type");
        allTracks[i]->setTypeOfTrack(type);

        if (type == "percussion")
        {
            allTracks[i]->setChannel(10);
        }
        else
        {
            allTracks[i]->setChannel(current);
            current++;
        }
        allTracks[i]->setNameLabel(name);
        allTracks[i]->setVolumeSlider(volume);
        allTracks[i]->setVolumeLabel(juce::String(volume));
        allTracks[i]->setInstrumentNumber(instrumentNumber);
        allTracks[i]->setUUID(uuid);
    }
}

Track::Track()
{
    volumeSlider.setSliderStyle(juce::Slider::LinearVertical);
    volumeSlider.setRange(0, 127, 1); //changed from 100 to 127 recently
    volumeSlider.onValueChange = [this]()
    {
        volumeLabel.setText(juce::String(volumeSlider.getValue()), juce::dontSendNotification);
    };

    volumeSlider.onDragEnd = [this]()
    {
        if (getTypeOfTrack() == TrackTypeConversion::toString(TrackType::Percussion) && syncVolumePercussionTracks)
            syncVolumePercussionTracks(volumeSlider.getValue());

        if (onChange)
            onChange();

        if (static_cast<int>(volumeSlider.getValue()) != -1 && isPlaying())
            notify(channel, static_cast<int>(volumeSlider.getValue()), -1);
    };

    addAndMakeVisible(volumeSlider);


    volumeLabel.setText(juce::String(volumeSlider.getValue()), juce::dontSendNotification);
    volumeLabel.setColour(juce::Label::textColourId, juce::Colours::crimson);
    addAndMakeVisible(volumeLabel);

    nameLabel.setText("None", juce::dontSendNotification);
    //nameLabel.setTooltip(nameLabel.getText());
    nameLabel.setColour(juce::Label::textColourId, juce::Colours::white);

    nameLabel.setInterceptsMouseClicks(true, false);
    nameLabel.addMouseListener(this, false);
    nameLabel.setMouseCursor(juce::MouseCursor::PointingHandCursor);

    addAndMakeVisible(nameLabel);

    instrumentChooserButton.setButtonText("Instrument");

    instrumentChooserButton.onClick = [this] {
        if (channel >= 2 && channel <= 16)
        {
            if (channel != 10)
                openInstrumentChooser();
            else
            {
                juce::String toShow = "Since this is a track for percussion, the instrument change won't take effect!\nIf you want to change anything, you must change the original notes!";
                showInstantInfo(toShow);
            }
        }
    };
    instrumentChooserButton.setMouseCursor(juce::MouseCursor::PointingHandCursor);
    addAndMakeVisible(instrumentChooserButton);

    instrumentlist = instrumentListBuild();


}

Track::~Track()
{
    volumeSlider.onDragEnd = nullptr;
    volumeSlider.onValueChange = nullptr; //in case of alt f4 it will crash otherwise since these listeners will propagate the onChange
}

void Track::resized()
{
    auto heightOfSlider = getHeight() / 2 + 20;
    auto startY = (getHeight() - heightOfSlider) / 2;
    volumeSlider.setBounds(0, startY - 10, 15, heightOfSlider);
    volumeLabel.setBounds(20, 0, 30, 40);
    nameLabel.setBounds((getWidth() - 40) / 2, volumeLabel.getHeight() + volumeLabel.getY() + 20, getWidth(), 20);


    int x = volumeSlider.getX() + volumeSlider.getWidth() + 10;
    int maxWidth = getWidth() - x - 5;
    int desiredWidth = 40;
    int actualWidth = juce::jmin(desiredWidth, maxWidth);
    instrumentChooserButton.setBounds(x, nameLabel.getY() - 25, maxWidth - 2, 20);
}

void Track::paint(juce::Graphics& g)
{
    g.setColour(juce::Colours::lightgrey);
    g.drawRect(getLocalBounds(), 1);
}

void Track::mouseEnter(const juce::MouseEvent& ev)
{
    if (ev.eventComponent == &nameLabel && toolTipWindow == nullptr)
    {
        toolTipWindow = std::make_unique<TrackNameToolTip>(nameLabel.getText());
        toolTipWindow->setSize(150, 24);

        auto labelBounds = nameLabel.getScreenBounds();
        int x = labelBounds.getX() + 4;
        int y = labelBounds.getBottom() + 4;

        toolTipWindow->setTopLeftPosition(x, y);
        toolTipWindow->addToDesktop(0);
        toolTipWindow->toFront(true);
        toolTipWindow->setVisible(true);
    }
}

void Track::mouseExit(const juce::MouseEvent& ev)
{
    if (toolTipWindow != nullptr)
    {
        toolTipWindow->setVisible(false);
        toolTipWindow = nullptr;
    }
}

juce::String Track::getName() const
{
    return this->nameLabel.getText();
}

juce::DynamicObject* Track::getJson() const
{

    auto* obj = new juce::DynamicObject{};

    obj->setProperty("name", nameLabel.getText());
    obj->setProperty("type", typeOfTrack);
    obj->setProperty("volume", volumeSlider.getValue());
    obj->setProperty("instrumentNumber", usedInstrumentNumber);
    obj->setProperty("uuid", uniqueIdentifierTrack.toString());

    return obj;
}

void Track::showInstantInfo(const juce::String& text)
{
    auto label = std::make_unique<juce::Label>();
    label->setText(text, juce::dontSendNotification);
    label->setJustificationType(juce::Justification::centred);
    label->setColour(juce::Label::backgroundColourId, juce::Colours::lightyellow);
    label->setColour(juce::Label::textColourId, juce::Colours::black);
    label->setBorderSize(juce::BorderSize<int>(2));
    label->setSize(300, 100);

    auto* display = findParentComponentOfClass<Display>();
    if (display != nullptr)
    {
        juce::CallOutBox::launchAsynchronously(std::move(label), display->getScreenBounds(), nullptr);
    }
}

juce::var Track::loadJson(const juce::File& file)
{
    if (file.exists())
        return juce::var{};

    juce::String jsonString = file.loadFileAsString();
    juce::var jsonVar = juce::JSON::parse(jsonString);

    if (jsonVar.isUndefined())
        return juce::var{};

    return jsonVar;
}

void Track::openInstrumentChooser()
{
    std::unique_ptr<InstrumentChooserComponent> chooser = std::make_unique<InstrumentChooserComponent>(instrumentlist);
    chooser->instrumentSelectedFunction = [this](int instrumentIndex, const juce::String& name)
    {
        setInstrumentNumber(instrumentIndex, true);
        nameLabel.setText(name, juce::dontSendNotification);
        if (onChange)
            onChange();

    };

    auto* display = findParentComponentOfClass<Display>();
    auto bounds = display->getScreenBounds();

    chooser->setSize(bounds.getWidth(), bounds.getHeight());

    juce::CallOutBox::launchAsynchronously(std::move(chooser), bounds, nullptr);
}

juce::StringArray Track::instrumentListBuild()
{
    juce::StringArray instrumentListLocal = {
    "Acoustic Grand Piano",
    "Bright Acoustic Piano",
    "Electric Grand Piano",
    "Honky-tonk Piano",
    "Electric Piano 1",
    "Electric Piano 2",
    "Harpsichord",
    "Clavi",
    "Celesta",
    "Glockenspiel",
    "Music Box",
    "Vibraphone",
    "Marimba",
    "Xylophone",
    "Tubular Bells",
    "Dulcimer",
    "Drawbar Organ",
    "Percussive Organ",
    "Rock Organ",
    "Church Organ",
    "Reed Organ",
    "Accordion",
    "Harmonica",
    "Tango Accordion",
    "Acoustic Guitar (nylon)",
    "Acoustic Guitar (steel)",
    "Electric Guitar (jazz)",
    "Electric Guitar (clean)",
    "Electric Guitar (muted)",
    "Overdriven Guitar",
    "Distortion Guitar",
    "Guitar harmonics",
    "Acoustic Bass",
    "Electric Bass (finger)",
    "Electric Bass (pick)",
    "Fretless Bass",
    "Slap Bass 1",
    "Slap Bass 2",
    "Synth Bass 1",
    "Synth Bass 2",
    "Violin",
    "Viola",
    "Cello",
    "Contrabass",
    "Tremolo Strings",
    "Pizzicato Strings",
    "Orchestral Harp",
    "Timpani",
    "String Ensemble 1",
    "String Ensemble 2",
    "SynthStrings 1",
    "SynthStrings 2",
    "Choir Aahs",
    "Voice Oohs",
    "Synth Voice",
    "Orchestra Hit",
    "Trumpet",
    "Trombone",
    "Tuba",
    "Muted Trumpet",
    "French Horn",
    "Brass Section",
    "Synth Brass 1",
    "Synth Brass 2",
    "Soprano Sax",
    "Alto Sax",
    "Tenor Sax",
    "Baritone Sax",
    "Oboe",
    "English Horn",
    "Bassoon",
    "Clarinet",
    "Piccolo",
    "Flute",
    "Recorder",
    "Pan Flute",
    "Blown Bottle",
    "Shakuhachi",
    "Whistle",
    "Ocarina",
    "Lead 1 (square)",
    "Lead 2 (sawtooth)",
    "Lead 3 (calliope)",
    "Lead 4 (chiff)",
    "Lead 5 (charang)",
    "Lead 6 (voice)",
    "Lead 7 (fifths)",
    "Lead 8 (bass + lead)",
    "Pad 1 (new age)",
    "Pad 2 (warm)",
    "Pad 3 (polysynth)",
    "Pad 4 (choir)",
    "Pad 5 (bowed)",
    "Pad 6 (metallic)",
    "Pad 7 (halo)",
    "Pad 8 (sweep)",
    "FX 1 (rain)",
    "FX 2 (soundtrack)",
    "FX 3 (crystal)",
    "FX 4 (atmosphere)",
    "FX 5 (brightness)",
    "FX 6 (goblins)",
    "FX 7 (echoes)",
    "FX 8 (sci-fi)",
    "Sitar",
    "Banjo",
    "Shamisen",
    "Koto",
    "Kalimba",
    "Bag pipe",
    "Fiddle",
    "Shanai",
    "Tinkle Bell",
    "Agogo",
    "Steel Drums",
    "Woodblock",
    "Taiko Drum",
    "Melodic Tom",
    "Synth Drum",
    "Reverse Cymbal",
    "Guitar Fret Noise",
    "Breath Noise",
    "Seashore",
    "Bird Tweet",
    "Telephone Ring",
    "Helicopter",
    "Applause",
    "Gunshot"
    };
    return instrumentListLocal;
}

void Track::setInstrumentNumber(int newInstrumentNumber, bool shouldNotify)
{
    this->usedInstrumentNumber = newInstrumentNumber;
    if (usedInstrumentNumber != -1 && shouldNotify)
        notify(channel, -1, usedInstrumentNumber);
}

int Track::getInstrumentNumber() const
{
    return usedInstrumentNumber;
}

void Track::setVolumeSlider(double value)
{
    this->volumeSlider.setValue(value);
}

void Track::setVolumeLabel(const juce::String& value, bool shouldNotify)
{
    volumeLabel.setText(value, juce::dontSendNotification);
    if (value.getIntValue() != -1 && shouldNotify)
        notify(channel, value.getIntValue(), -1);
}

void Track::setNameLabel(const juce::String& name)
{
    nameLabel.setText(name, juce::dontSendNotification);
    //nameLabel.setTooltip(name);
}

void Track::mouseDown(const juce::MouseEvent& event)
{
    if (event.eventComponent == &nameLabel)
    {
        if (event.mods.isLeftButtonDown())
        {
            if (onRequestTrackSelection)
            {
                onRequestTrackSelection([this](const juce::String& selectedTrack, const juce::Uuid& uuid, const juce::String& type)
                    {
                        setNameLabel(selectedTrack);
                        setUUID(uuid);
                        setTypeOfTrack(type);
                    }
                );
            }
        }
        else if(event.mods.isRightButtonDown()){
            juce::PopupMenu menu;
            menu.addItem("Rename", [this]() {
                renameOneTrack();
                });
            menu.addItem("Delete", [this]() { 
                deleteOneTrack();
                });
            menu.addItem("Copy", [this]() { 
                copyOneTrack();
                });

            menu.addItem("Paste", [this]() {
                pasteOneTrack();
                });

            menu.showMenuAsync(juce::PopupMenu::Options().withTargetComponent(&nameLabel));
        }
    }
}

void Track::setUUID(const juce::Uuid& newUUID)
{
    this->uniqueIdentifierTrack = newUUID;
}

void Track::setTypeOfTrack(const juce::String& newType)
{
    this->typeOfTrack = newType;
}

juce::String Track::getTypeOfTrack() const
{
    return this->typeOfTrack;
}

juce::Uuid Track::getUsedID() const
{
    return uniqueIdentifierTrack;
}

void Track::setChannel(int newChannel)
{
    this->channel = newChannel;
}

int Track::getChannel() const
{
    return this->channel;
}

double Track::getVolume() const
{
    return this->volumeSlider.getValue();
}

void Track::copyFrom(const Track& other)
{
    setNameLabel(other.getName());
    setInstrumentNumber(other.getInstrumentNumber());
    setTypeOfTrack(other.getTypeOfTrack());
    setVolumeSlider(other.getVolume());
    setVolumeLabel(juce::String(other.getVolume()));
    setUUID(other.getUsedID());
}

void Track::pasteFrom(const Track& source)
{
    copyFrom(source);
}

void Track::renameOneTrack()
{
    auto window = new juce::AlertWindow{ "Rename track","Enter a name for your track",juce::AlertWindow::NoIcon};

    window->addTextEditor("nameEditor", nameLabel.getText(), "Track Name:");
    window->addButton("OK", 1, juce::KeyPress(juce::KeyPress::returnKey));
    window->addButton("Cancel", 0, juce::KeyPress(juce::KeyPress::escapeKey));

    window->enterModalState(true, juce::ModalCallbackFunction::create([this, window](int result)
        {
            std::unique_ptr<juce::AlertWindow> cleanup{ window };
            if (result != 1)
                return;

            juce::String newNameToPut=window->getTextEditor("nameEditor")->getText().trim();
            setNameLabel(newNameToPut);
            if (onChange)
                onChange();
        }
    ));
}

void Track::deleteOneTrack()
{
    juce::AlertWindow::showOkCancelBox(
        juce::AlertWindow::WarningIcon,
        "Confirm Remove",
        "Are you sure you want to remove the selected track?",
        "Remove",
        "Cancel",
        this,
        juce::ModalCallbackFunction::create(
            [this](int result)
            {
                if (result == 0)
                    return;

                setNameLabel("None");
                setTypeOfTrack("None");
                setInstrumentNumber(-1);
                setUUID(juce::Uuid{ "noID" });

                if (onChange)
                    onChange();
            }
            ));
}

void Track::copyOneTrack()
{
    if (onCopy)
        onCopy(this);

    juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::InfoIcon, "Track info", "The track has been succesfully copied!");
}

void Track::pasteOneTrack()
{
    if (onPaste)
        onPaste(this);

    if (onChange)
        onChange();
}

TrackListComponent::TrackListComponent(std::shared_ptr<std::vector<TrackEntry>> tracks,
    std::shared_ptr<std::unordered_map<juce::String, std::vector<TrackEntry>>> groupedTracksMap,
    std::shared_ptr<std::vector<juce::String>> groupedKeys,
    std::function<void(int)> onTrackChosen) : availableTracks{ tracks },
    groupedTracks{ groupedTracksMap },
    groupedTrackKeys{ groupedKeys },
    trackChosenCallBack{ onTrackChosen }
{

    addAndMakeVisible(listBox);
    listBox.setMultipleSelectionEnabled(true);
    listBox.setModel(this);
    listBox.updateContent();
    addAndMakeVisible(addButtonFolder);
    addAndMakeVisible(removeButtonFolder);

    addButtonFolder.setMouseCursor(juce::MouseCursor::PointingHandCursor);
    removeButtonFolder.setMouseCursor(juce::MouseCursor::PointingHandCursor);

    addButtonFolder.onClick = [this] {
        addToFolderList();
    };

    removeButtonFolder.onClick = [this] {
        removeFromFolderList();
    };
}

void TrackListComponent::resized()
{
    auto area = getLocalBounds();

    int controlBarHeight = 36;
    auto controlBarArea = area.removeFromTop(controlBarHeight);

    const int backWidth = 100;
    const int smallButtonWidth = 100;
    const int comboWidth = 150;
    const int spacing = 5;

    if (addButtonFolder.isVisible())
        addButtonFolder.setBounds(controlBarArea.removeFromLeft(smallButtonWidth).reduced(spacing));

    if (removeButtonFolder.isVisible())
        removeButtonFolder.setBounds(controlBarArea.removeFromLeft(smallButtonWidth).reduced(spacing));

    if (backButton)
        backButton->setBounds(controlBarArea.removeFromLeft(80).reduced(5));

    if (sortComboBox)
        sortComboBox->setBounds(controlBarArea.removeFromLeft(120).reduced(5));

    if (addButton)
        addButton->setBounds(controlBarArea.removeFromLeft(80).reduced(5));

    if (removeButton)
        removeButton->setBounds(controlBarArea.removeFromLeft(80).reduced(5));

    listBox.setBounds(area);
}

int TrackListComponent::getNumRows()
{
    if (viewMode == ViewMode::TrackView)
        return (int)availableTracks->size();
    else return (int)groupedTrackKeys->size();
}

void TrackListComponent::comboBoxChanged(juce::ComboBox* comboBoxThatHasChanged)
{
    DBG("Size 15" + juce::String(availableTracks->size()));
    if (comboBoxThatHasChanged == sortComboBox.get())
    {
        int id = sortComboBox->getSelectedId();
        if (id == 1)
        {
            std::sort(availableTracks->begin(), availableTracks->end(), [](const TrackEntry& first, const TrackEntry& second)
                {
                    return first.getDisplayName().toLowerCase() < second.getDisplayName().toLowerCase();
                });
        }
        else if (id == 2)
        {
            std::sort(availableTracks->begin(), availableTracks->end(), [](const TrackEntry& first, const TrackEntry& second)
                {
                    return first.file.getLastModificationTime() > second.file.getLastModificationTime();
                });
        }
        else if (id == 3)
        {
            std::sort(availableTracks->begin(), availableTracks->end(), [](const TrackEntry& first, const TrackEntry& second)
                {
                    return first.file.getLastModificationTime() < second.file.getLastModificationTime();
                });
        }

        listBox.updateContent();
        listBox.repaint();
    }
}

void TrackListComponent::paintListBoxItem(int rowNumber, juce::Graphics& g, int width, int height, bool rowIsSelected)
{
    if (rowIsSelected)
        g.fillAll(juce::Colours::lightblue);
    else
        g.fillAll(juce::Colours::white);

    g.setColour(juce::Colours::black);

    juce::String label;

    if (viewMode == ViewMode::TrackView)
    {
        if (juce::isPositiveAndBelow(rowNumber, availableTracks->size()))
            label = (*availableTracks)[rowNumber].getDisplayName();
    }
    else if (viewMode == ViewMode::FolderView)
    {
        if (juce::isPositiveAndBelow(rowNumber, groupedTrackKeys->size()))
            label = (*groupedTrackKeys)[rowNumber];
    }

    g.drawText(label, 5, 0, width - 10, height, juce::Justification::centredLeft);

    g.setColour(juce::Colours::lightgrey);
    g.drawRect(0, 0, width, height, 1);
}

void TrackListComponent::listBoxItemClicked(int row, const juce::MouseEvent& event)
{
    if (event.getNumberOfClicks() == 2)
    {
        if (viewMode == ViewMode::TrackView && juce::isPositiveAndBelow(row, availableTracks->size()))
        {
            if (trackChosenCallBack)
                trackChosenCallBack(row);
        }
        else if (viewMode == ViewMode::FolderView)
        {
            if (juce::isPositiveAndBelow(row, groupedTrackKeys->size()))
            {
                juce::String folderKey = (*groupedTrackKeys)[row];
                currentFolderName = folderKey;
                if (groupedTracks->find(folderKey) != groupedTracks->end())
                {
                    *availableTracks = (*groupedTracks)[folderKey];
                    initializeTracksFromList();
                    resized();


                    listBox.updateContent();
                    listBox.repaint();
                }
            }
        }
    }
    else
    {

    }
}

void TrackListComponent::addToTrackList()
{
    auto chooser = std::make_shared<juce::FileChooser>("Select a MIDI file to add", juce::File{}, "*.mid");

    chooser->launchAsync(
        juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
        [this, chooser](const juce::FileChooser& fc)
        {
            auto file = fc.getResult();
            if (file.existsAsFile())
            {
                juce::FileInputStream inputStream(file);
                if (!inputStream.openedOk())
                {
                    juce::AlertWindow::showMessageBoxAsync(
                        juce::AlertWindow::WarningIcon,
                        "Failed parsing",
                        "Could not open the selected file");
                    return;
                }

                juce::MidiFile midiFile;
                if (!midiFile.readFrom(inputStream))
                {
                    juce::AlertWindow::showMessageBoxAsync(
                        juce::AlertWindow::WarningIcon,
                        "Failed parsing",
                        "Could not read the selected file");
                    return;
                }

                double originalBpm = getOriginalBpmFromFile(midiFile);
                convertTicksToSeconds(midiFile, originalBpm);

                int totalTracks = midiFile.getNumTracks();
                int addedTracks = 0;
                int duplicateTracks = 0;

                for (int i = 0; i < totalTracks; ++i)
                {
                    bool duplicate = false;
                    for (const auto& trackEntry : *availableTracks)
                    {
                        if (trackEntry.file == file && trackEntry.trackIndex == i)
                        {
                            duplicate = true;
                            break;
                        }
                    }
                    if (duplicate)
                    {
                        ++duplicateTracks;
                        continue;
                    }

                    auto* trackSequence = midiFile.getTrack(i);
                    juce::String displayName = extractDisplayNameFromTrack(*trackSequence);

                    TrackEntry newEntry;
                    newEntry.file = file;
                    newEntry.trackIndex = i;
                    newEntry.displayName = displayName;
                    newEntry.originalSequenceTicks = *trackSequence;

                    newEntry.sequence = *trackSequence;
                    newEntry.originalBPM = originalBpm;

                    newEntry.folderName = currentFolderName;
                    DBG("Folder name is:" + newEntry.folderName);

                    if (foundPercussion(trackSequence))
                        newEntry.type = TrackType::Percussion;
                    else newEntry.type = TrackType::Melodic;

                    newEntry.uuid = TrackEntry::generateUUID();

                    availableTracks->push_back(newEntry);
                    (*groupedTracks)[currentFolderName].push_back(newEntry);
                    ++addedTracks;
                }

                listBox.updateContent();
                listBox.repaint();
                auto appDataFolder = juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
                    .getChildFile("Piano Synth2");
                auto jsonFile = appDataFolder.getChildFile("myTracks.json");

                saveToFile(jsonFile);

                int id = sortComboBox->getSelectedId();
                sortComboBox->setSelectedId(id);
                comboBoxChanged(sortComboBox.get());

                notifyAddingToTrackList();

                juce::AlertWindow::showMessageBoxAsync(
                    juce::AlertWindow::InfoIcon,
                    "Add Tracks",
                    "Selected MIDI file has " + juce::String(totalTracks) + " tracks.\n"
                    + juce::String(addedTracks) + " tracks added.\n"
                    + juce::String(duplicateTracks) + " tracks were already in the list."
                );
            }
        }
    );

}

void TrackListComponent::removeFromTrackList()
{
    juce::SparseSet<int> selectedRows = listBox.getSelectedRows();

    if (selectedRows.isEmpty())
    {
        juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::WarningIcon, "Remove Track(s)", "Please select at least one track to remove");
        return;
    }



    juce::AlertWindow::showOkCancelBox(
        juce::AlertWindow::WarningIcon,
        "Confirm Remove",
        "Are you sure you want to remove the selected track(s)?",
        "Remove",
        "Cancel",
        this,
        juce::ModalCallbackFunction::create(
            [this, selectedRows](int result)
            {
                if (result == 0) // Cancel
                    return;


                for (int i = selectedRows.size() - 1; i >= 0; --i)
                {
                    int rowIndex = selectedRows[i];
                    auto& track = (*availableTracks)[rowIndex];
                    juce::Uuid uuid = track.getUniqueID();
                    juce::String folderName = track.folderName;

                    auto& folderVector = (*groupedTracks)[folderName];
                    folderVector.erase(std::remove_if(folderVector.begin(), folderVector.end(), [uuid](const TrackEntry& tr)
                        {
                            return uuid == tr.getUniqueID();

                        }));
                    availableTracks->erase(availableTracks->begin() + rowIndex);

                    if (onRemoveTrack) onRemoveTrack(uuid);
                }

                listBox.deselectAllRows();
                listBox.updateContent();
                listBox.repaint();

                auto appDataFolder = juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
                    .getChildFile("Piano Synth2");
                auto jsonFile = appDataFolder.getChildFile("myTracks.json");
                saveToFile(jsonFile);
            }
        )
    );
}


void TrackListComponent::addToFolderList()
{
    auto* window = new juce::AlertWindow("Create Folder",
        "Enter a name for the new folder:",
        juce::AlertWindow::NoIcon);

    window->addTextEditor("folderName", "");
    window->addButton("OK", 1, juce::KeyPress(juce::KeyPress::returnKey));
    window->addButton("Cancel", 0, juce::KeyPress(juce::KeyPress::escapeKey));

    window->enterModalState(true, juce::ModalCallbackFunction::create([this, window](int result)
        {
            std::unique_ptr<juce::AlertWindow> cleanup(window);
            if (result != 1)
                return;

            juce::String folderName = window->getTextEditor("folderName")->getText().trim();
            if (folderName.isEmpty())
            {
                juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::WarningIcon,
                    "Invalid Name",
                    "Folder name cannot be empty.");
                return;
            }

            if (groupedTracks->count(folderName) > 0)
            {
                juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::WarningIcon,
                    "Duplicate Name",
                    "A folder with this name already exists.");
                return;
            }

            groupedTrackKeys->push_back(folderName);
            (*groupedTracks)[folderName] = {};
            currentFolderName = folderName;

            viewMode = ViewMode::FolderView;
            listBox.updateContent();
            listBox.repaint();

            auto appDataFolder = juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
                .getChildFile("Piano Synth2");
            auto jsonFile = appDataFolder.getChildFile("myTracks.json");
            saveToFile(jsonFile);

        }));
}

void TrackListComponent::removeFromFolderList()
{
    juce::SparseSet<int> selectedRows = listBox.getSelectedRows();

    if (selectedRows.isEmpty())
    {
        juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::WarningIcon, "Remove Folder(s)", "Please select at least one folder to remove");
        return;
    }

    juce::AlertWindow::showOkCancelBox(
        juce::AlertWindow::WarningIcon,
        "Confirm Remove",
        "Are you sure you want to remove the selected folder(s)?",
        "Remove",
        "Cancel",
        this,
        juce::ModalCallbackFunction::create(
            [this, selectedRows](int result)
            {
                if (result == 0) // Cancel
                    return;

                for (int i = selectedRows.size() - 1; i >= 0; i--)
                {
                    int rowIndex = selectedRows[i];
                    juce::String folderName = (*groupedTrackKeys)[rowIndex];

                    auto& tracks = (*groupedTracks)[folderName];
                    std::vector<juce::Uuid> uuidsToRemove;

                    for (auto& track : tracks)
                        uuidsToRemove.push_back(track.getUniqueID());

                    if (onRemoveMultipleTracks)
                        onRemoveMultipleTracks(uuidsToRemove);

                    tracks.clear();

                    groupedTracks->erase(folderName);

                    auto it = std::find(groupedTrackKeys->begin(), groupedTrackKeys->end(), folderName);
                    if (it != groupedTrackKeys->end())
                        groupedTrackKeys->erase(it);

                    listBox.deselectAllRows();
                    listBox.updateContent();
                    listBox.repaint();

                    auto appDataFolder = juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
                        .getChildFile("Piano Synth2");
                    auto jsonFile = appDataFolder.getChildFile("myTracks.json");
                    saveToFile(jsonFile);
                }
            }
        )
    );
}

void TrackListComponent::backToFolderView()
{
    viewMode = ViewMode::FolderView;
    deallocateTracksFromList();
    addButtonFolder.setVisible(true);
    removeButtonFolder.setVisible(true);
    listBox.updateContent();
    listBox.repaint();
}

void TrackListComponent::saveToFile(const juce::File& fileToSave)
{
    const auto& grouped = groupedTracks;

    juce::Array<juce::var> foldersArray;

    for (auto& [folderName, tracks] : *grouped)
    {
        auto* folderObj = new juce::DynamicObject{};
        folderObj->setProperty("folderName", folderName);

        if (!tracks.empty())
            folderObj->setProperty("filePath", tracks[0].file.getFullPathName());
        else
            folderObj->setProperty("filePath", juce::String());

        juce::Array<juce::var> trackArray;

        for (auto& tr : tracks)
        {
            auto* trackObject = new juce::DynamicObject{};

            trackObject->setProperty("trackIndex", tr.trackIndex);
            trackObject->setProperty("displayName", tr.displayName);
            trackObject->setProperty("uuid", tr.uuid.toString());

            trackArray.add(juce::var(trackObject));
        }
        folderObj->setProperty("Tracks", trackArray);
        foldersArray.add(juce::var(folderObj));

    }

    juce::var jsonVar(foldersArray);
    juce::String jsonString = juce::JSON::toString(jsonVar);
    fileToSave.replaceWithText(jsonString);
}

//the flat list availableTracks doesn't need to be built rn, because we have grouped tracks and that's structured, but for future if
//i wanna change anything, like to have idk, something with all the tracks, i won't need to iterate again through every structured folder name to get all the available tracks
void TrackListComponent::loadFromFile(const juce::File& fileToLoad)

{
    if (!fileToLoad.existsAsFile())
        return;

    juce::String jsonString = fileToLoad.loadFileAsString();
    juce::var jsonVar = juce::JSON::parse(jsonString);

    if (!jsonVar.isArray())
        return;


    availableTracks->clear();
    groupedTrackKeys->clear();
    groupedTracks->clear();

    juce::Array<juce::var>* foldersArray = jsonVar.getArray();

    for (auto& item : *foldersArray)
    {
        auto* folderObj = item.getDynamicObject();
        if (folderObj == nullptr)
            continue;

        juce::String filePath = folderObj->getProperty("filePath").toString();
        juce::String folderName = folderObj->getProperty("folderName").toString();

        if (filePath.isEmpty())
        {
            // Empty folder, add folder name and an empty vector of tracks
            groupedTrackKeys->push_back(folderName);
            (*groupedTracks)[folderName] = {};
            continue; // move on to next folder
        }

        juce::File file{ filePath };

        if (!file.existsAsFile())
            continue;

        groupedTrackKeys->push_back(folderName);
        (*groupedTracks)[folderName] = {};

        juce::var tracksVar = folderObj->getProperty("Tracks");

        if (!tracksVar.isArray())
            continue;

        juce::Array<juce::var>* trackArray = tracksVar.getArray();

        juce::FileInputStream inputStream(file);
        if (!inputStream.openedOk())
            continue;

        juce::MidiFile midiFile;
        if (!midiFile.readFrom(inputStream))
            continue;


        double originalBpm = getOriginalBpmFromFile(midiFile);

        convertTicksToSeconds(midiFile, originalBpm);

        for (auto& trackItem : *trackArray)
        {
            auto* trackObj = trackItem.getDynamicObject();
            if (trackObj == nullptr)
                continue;

            int trackIndex = (int)trackObj->getProperty("trackIndex");
            juce::String displayName = trackObj->getProperty("displayName").toString();

            if (trackIndex >= midiFile.getNumTracks())
                continue;

            auto* sequence = midiFile.getTrack(trackIndex);
            if (sequence == nullptr)
                continue;

            juce::String uuidString = trackObj->getProperty("uuid").toString();

            TrackEntry tr;
            tr.file = file;
            tr.trackIndex = trackIndex;
            tr.displayName = displayName;
            tr.sequence = *sequence;
            tr.originalBPM = originalBpm;
            tr.folderName = folderName;

            if (foundPercussion(sequence))
                tr.type = TrackType::Percussion;
            else tr.type = TrackType::Melodic;

            if (uuidString.isNotEmpty())
                tr.uuid = juce::Uuid(uuidString);
            else tr.uuid = TrackEntry::generateUUID();

            //availableTracks->push_back(tr);

            (*groupedTracks)[folderName].push_back(tr);
        }
    }
    viewMode = ViewMode::FolderView;
    listBox.updateContent();
    listBox.repaint();
}

std::vector<TrackEntry>& TrackListComponent::getAllAvailableTracks() const
{
    return *availableTracks;
}

juce::String TrackListComponent::extractDisplayNameFromTrack(const juce::MidiMessageSequence& trackSeq)
{
    juce::String trackName;
    juce::String instrumentName;

    for (int i = 0; i < trackSeq.getNumEvents(); i++)
    {
        const auto& event = trackSeq.getEventPointer(i)->message;

        if (event.isTrackNameEvent())
        {
            trackName = event.getTextFromTextMetaEvent();
        }
        else if (event.isMetaEvent() && event.getMetaEventType() == 0x04)
        {
            instrumentName = event.getTextFromTextMetaEvent();
        }
    }

    if (trackName.isNotEmpty() && instrumentName.isNotEmpty())
        return trackName + " (" + instrumentName + ")";
    else if (trackName.isNotEmpty())
        return trackName;
    else if (instrumentName.isNotEmpty())
        return instrumentName;

    return "Unnamed Track";
}

std::unordered_map<juce::Uuid, TrackEntry> TrackListComponent::buildTrackNameMap()
{
    std::unordered_map<juce::Uuid, TrackEntry> map;

    for (const auto& [folderName, tracks] : *groupedTracks)
    {
        DBG("yuppiii");
        for (const auto& tr : tracks)
        {
            map[tr.getUniqueID()] = tr;
        }
    }

    return map;
}

double TrackListComponent::getOriginalBpmFromFile(const juce::MidiFile& midiFile)
{
    if (midiFile.getNumTracks() == 0)
        return 120.0;

    auto* firstTrack = midiFile.getTrack(0);
    if (firstTrack == nullptr)
        return 120.0;

    for (int i = 0; i < firstTrack->getNumEvents(); ++i)
    {
        const auto& msg = firstTrack->getEventPointer(i)->message;
        if (msg.isTempoMetaEvent())
        {
            double bpm;
            if (msg.getTempoSecondsPerQuarterNote() > 0.0)
                bpm = 60.0 / msg.getTempoSecondsPerQuarterNote();
            else bpm = 120.0;
            return bpm;
        }
    }
    return 120.0;
}

void TrackListComponent::initializeTracksFromList()
{
    viewMode = ViewMode::TrackView;
    addButtonFolder.setVisible(false);
    removeButtonFolder.setVisible(false);

    backButton = std::make_unique<juce::TextButton>("Back");
    addButton = std::make_unique<juce::TextButton>("Add");
    removeButton = std::make_unique<juce::TextButton>("Remove");
    sortComboBox = std::make_unique<juce::ComboBox>();

    sortComboBox->addListener(this);

    backButton->setMouseCursor(juce::MouseCursor::PointingHandCursor);
    addButton->setMouseCursor(juce::MouseCursor::PointingHandCursor);
    removeButton->setMouseCursor(juce::MouseCursor::PointingHandCursor);


    backButton->onClick = [this]() {
        backToFolderView();
    };

    addButton->onClick = [this]() {
        addToTrackList();
    };

    removeButton->onClick = [this]() {
        removeFromTrackList();
    };

    addAndMakeVisible(backButton.get());
    addAndMakeVisible(addButton.get());
    addAndMakeVisible(removeButton.get());
    addAndMakeVisible(sortComboBox.get());

    sortComboBox->addItem("Sort by Name(ascending)", 1);
    sortComboBox->addItem("Sort by Last Modified (Newest)", 2);
    sortComboBox->addItem("Sort by Last Modified (Oldest)", 3);

    sortComboBox->setSelectedId(1);
}

void TrackListComponent::deallocateTracksFromList()
{
    backButton = nullptr;
    addButton = nullptr;
    removeButton = nullptr;
    sortComboBox = nullptr;
}

bool TrackListComponent::foundPercussion(const juce::MidiMessageSequence* sequence)
{
    bool perc = false;
    for (int i = 0; i < sequence->getNumEvents(); ++i)
    {
        auto msg = sequence->getEventPointer(i)->message;
        if (msg.isNoteOnOrOff() && msg.getChannel() == 10)
        {
            perc = true;
            break;
        }
    }
    return perc;
}

void MyTabbedComponent::currentTabChanged(int newCurrentTabIndex, const juce::String& newTabName)
{
    if (onTabChanged)
        onTabChanged(newCurrentTabIndex, newTabName);
}