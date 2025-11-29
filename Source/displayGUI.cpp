/*
  ==============================================================================

    displayGUI.cpp
    Created: 8 Jun 2025 2:44:43am
    Author:  Kisuke

  ==============================================================================
*/

#include "displayGUI.h"
#include "InstrumentChooser.h"
#include "CustomTableContainer.h"

Display::Display(std::weak_ptr<juce::MidiOutput> outputDev, int widthForList) : outputDevice{outputDev}
{
    availableTracksFromFolder = std::make_shared<std::deque<TrackEntry>>();
    groupedTracks = std::make_shared<std::unordered_map<juce::String, std::deque<TrackEntry>>>();
    groupedTrackKeys = std::make_shared<std::vector<juce::String>>();

    tabComp = std::make_unique<MyTabbedComponent>(juce::TabbedButtonBar::TabsAtLeft);
    

    addAndMakeVisible(tabComp.get());

    initializeAllStyles();
    loadAllStyles();

    auto appDataFolder = IOHelper::getFolder("Piano Synth2");

    auto jsonFile = IOHelper::getFile("myTracks.json");


    if (!jsonFile.exists())
    {
        juce::var emptyArray = juce::Array<juce::var>{};
        juce::String jsonString = juce::JSON::toString(emptyArray);

        jsonFile.replaceWithText(jsonString);
    }

    auto jsonFileSections = IOHelper::getFile("mySections.json");

    if (!jsonFileSections.exists())
    {
        auto* emptyObj = new juce::DynamicObject();
        juce::var jsonVar(emptyObj);

        juce::String jsonString = juce::JSON::toString(jsonVar);
        jsonFileSections.replaceWithText(jsonString);
    }

    TrackIOHelper::loadFromFile(jsonFile, *groupedTracks, *groupedTrackKeys);
    

    mapUuidToTrack = buildTrackUuidMap();

    std::vector<juce::String> stylesNames = getAllStylesFromJson();
    DBG("Size of names is:" + juce::String(stylesNames.size()));

    auto list = std::make_unique<StylesListComponent>(stylesNames,
        [this](const juce::String& name) {
            showCurrentStyleTab(name);
        },
        widthForList
    );

    list->onStyleRename = [this](const juce::String& oldName, const juce::String& newName)
    {
        int index = tabComp->getCurrentTabIndex();
        if (index>=0 && index + 1 < tabComp->getNumTabs())
        {
            tabComp->setTabName(index + 1, newName);

            currentStyleComponent->updateName(newName);
        }

        updateStyleNameInJson(oldName, newName);
    };

    list->onStyleAdd = [this](const juce::String& newName)
    {
        appendNewStyleInJson(newName);
    };

    list->onStyleRemove = [this](const juce::String& name)
    {
        if (tabComp)
        {
            int currentTabIndex = tabComp->getCurrentTabIndex();
            if (currentTabIndex + 1 <= tabComp->getNumTabs())
            {
                juce::String tabName = tabComp->getTabNames()[currentTabIndex + 1];
                if (tabName == name)
                {
                    tabComp->removeTab(currentTabIndex + 1);
                    currentStyleComponent.reset();
                    created = false;
                }
            }
        }
        removeStyleInJson(name);
    };

    //auto* scrollableView = new juce::Viewport();
    //scrollableView->setScrollBarsShown(true, false); // only vertical
    //scrollableView->setViewedComponent(list, true);
    tabComp->addTab("Styles", juce::Colour::fromRGB(10, 15, 10), list.get(), false);

    stylesListComponent = std::move(list);
    //list->resized();
    //scrollableView->getViewedComponent()->resized();

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

            trackListComp.reset();
            createdTracksTab = false;
            return;
        }

        if (createdPlaybackSettingsTab && newIndex==0)
        {
            int mySettingsIndex = tabComp->getNumTabs() - 1;
            tabComp->removeTab(mySettingsIndex);

            playBackSettings.reset();
            createdPlaybackSettingsTab = false;
            return;
        }

        
    };
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
        currentStyleComponent = std::make_unique<CurrentStyleComponent>(name, mapUuidToTrack, outputDevice);
        currentStyleComponent->onRequestTrackSelectionFromTrack = [this](std::function<void(const juce::String&, const juce::Uuid& uuid, const juce::String& type)> trackChosenCallback)
        {
            showListOfTracksToSelectFrom(trackChosenCallback);
        };

        currentStyleComponent->updateTrackFile = [this]()
        {
            auto appDataFolder = IOHelper::getFolder("Piano Synth2");

            auto jsonFile = IOHelper::getFile("myTracks.json");
            TrackIOHelper::saveToFile(jsonFile, *groupedTracks);
        };

        currentStyleComponent->keybindTabStarting = [this]()
        {

            int existingIndex = getTabIndexByName("Playback Settings");

            if (existingIndex >= 0)
            {
                tabComp->setCurrentTabIndex(existingIndex);
                return;
            }


            playBackSettings = std::make_unique<PlayBackSettingsComponent>(minNote, maxNote, this->settings);
            playBackSettings->setBounds(getLocalBounds());

            playBackSettings->onChangingSettings = [this](PlayBackSettings newSettings)
            {
                //this->settings = newSettings; no need of this anymore since the PlaybackSettings class holds a reference to the settings;
                displayListeners.call(&DisplayListener::playBackSettingsChanged, this->settings);
            };

            playBackSettings->onChangingTranspose = [this](int transposeValue)
            {
                displayListeners.call(&DisplayListener::playBackSettingsTransposeChanged, transposeValue);
            };

            createdPlaybackSettingsTab = true;
            tabComp->addTab("Playback Settings", juce::Colour::fromRGB(10, 15, 10), playBackSettings.get(), false);

            existingIndex = tabComp->getNumTabs() - 1;

            
            tabComp->setCurrentTabIndex(existingIndex);
        };

        tabComp->addTab(name, juce::Colour::fromRGB(10, 15, 10), currentStyleComponent.get(), false);
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

    currentStyleComponent->tabExsitsCallback = [this](const juce::String& name)
    {
        return this->existsTab(name);
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

    double bpmToUse = currentStyleComponent->getTempo();
    DBG("BPM TO USE:" + juce::String(bpmToUse));
    currentStyleComponent->applyBPMchangeBeforePlayback(bpmToUse);
    currentStyleComponent->applyChangesForAllTracksCurrentStyle();
}

void Display::showListOfTracksToSelectFrom(std::function<void(const juce::String&, const juce::Uuid& uuid, const juce::String& type)> onTrackSelected)
{
    trackListComp = std::make_unique<TrackListComponent>(availableTracksFromFolder, groupedTracks, groupedTrackKeys,
        [this, onTrackSelected](int index)
        {
            auto& track = (*availableTracksFromFolder)[index];
            juce::String typeString = TrackTypeConversion::toString(track.type);

            onTrackSelected(track.getDisplayName(), track.getUniqueID(), typeString);

            mapUuidToTrack[track.getUniqueID()] = &track;
            
            if (currentStyleComponent)
                currentStyleComponent->applyBPMchangeForOne(currentStyleComponent->getTempo(), track.getUniqueID());

            //if (mapNameToTrack.empty())
              //  mapNameToTrack = buildTrackNameMap();

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
    
    trackListComp->onRenameTrackFromList = [this](const juce::Uuid& uuid, const juce::String& newName)
    {
        updateTrackNameFromAllStyles(uuid, newName);
    };


    trackListComp->addToMapOnAdding= [this](TrackEntry* newEntry)
    {
        mapUuidToTrack[newEntry->getUniqueID()] = newEntry;
    };

    tabComp->addTab("My tracks", juce::Colour::fromRGB(10, 15, 10), trackListComp.get(), false);
    tabComp->setCurrentTabIndex(tabComp->getNumTabs() - 1);

    createdTracksTab = true;
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

void Display::setDeviceOutput(std::weak_ptr<juce::MidiOutput> devOutput)
{
    this->outputDevice = devOutput;
    if (currentStyleComponent)
        currentStyleComponent->setDeviceOutputCurrentStyle(devOutput);
}

void Display::stoppingPlayer()
{
    if (currentStyleComponent)
        this->currentStyleComponent->triggerStopClick();
}

void Display::startingPlayer()
{
    if (currentStyleComponent)
        this->currentStyleComponent->triggerStartClick();
}

void Display::updateUIbeforeAnyLoadingCase()
{
    DBG("sUNT IN CAZUL ASTA");
    //if (this->mapNameToTrack.empty())
    //    mapNameToTrack = trackListComp->buildTrackNameMap();
}

void Display::set_min_max(int min, int max)
{
    if (min < 21)
        this->minNote = 21;
    else this->minNote = min;

    if (max > 108)
        this->maxNote = 108;
    else this->maxNote = max;
    if (playBackSettings)
        this->playBackSettings->setLowestHighest(minNote, maxNote);
}

void Display::set_VID_PID(const juce::String& VID, const juce::String& PID)
{
    this->settings.VID = VID;
    this->settings.PID = PID;
    DBG("In setter:" + this->settings.VID + " " + this->settings.PID);
}

juce::String Display::getVID()
{
    return this->settings.VID;
}

juce::String Display::getPID()
{
    return this->settings.PID;
}

void Display::readSettingsFromJSON()
{
    PlayBackSettings settingsLoaded = PlaybackSettingsIOHelper::loadFromFile(IOHelper::getFile("playbackSettings.json"), this->settings.VID, this->settings.PID);
    this->settings = settingsLoaded;
}

void Display::homeButtonInteraction()
{
    tabComp->removeTab(tabComp->getNumTabs() - 1);
    tabComp->setCurrentTabIndex(tabComp->getNumTabs() - 1);
    this->currentStyleComponent->comboBoxChangeIndex(0);
    this->playBackSettings.reset(); 
}

int Display::getStartNote()
{
    return this->settings.startNote;
}

int Display::getEndNote()
{
    return this->settings.endNote;
}

int Display::getLeftBound()
{
    return this->settings.leftHandBound;
}

int Display::getRightBound()
{
    return this->settings.rightHandBound;
}

void Display::addListener(DisplayListener* listener)
{
    displayListeners.add(listener);
}

void Display::removeListener(DisplayListener* listener)
{
    displayListeners.remove(listener);
}

int Display::getNumTabs()
{
    return this->tabComp->getNumTabs();
}

void Display::setNewSettingsHelperFunction(int value)
{
    if(settings.startNote!=-1)
        settings.startNote += value;

    if(settings.endNote!=-1)
        settings.endNote += value;

    if(settings.leftHandBound!=-1)
        settings.leftHandBound += value;

    if(settings.rightHandBound!=-1)
        settings.rightHandBound += value;

    if (settings.startNote < 21 && settings.startNote!=-1)
        settings.startNote = 21;

    if (settings.endNote > 108 && settings.endNote!=-1)
        settings.endNote = 108;

    if (settings.leftHandBound < 21)
        settings.leftHandBound = 21;

    if (settings.rightHandBound > 108)
        settings.rightHandBound = 108;

    if(playBackSettings)
        this->playBackSettings->setNewSettings(settings);
    else displayListeners.call(&DisplayListener::playBackSettingsChanged, this->settings);
}

bool Display::existsTab(const juce::String& name)
{
    for (auto& tabName : tabComp->getTabNames())
    {
        if (tabName == name)
            return true;
    }
    return false;
}

void Display::callingListeners()
{
    displayListeners.call(&DisplayListener::playBackSettingsChanged, this->settings);
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
                trackObj->setProperty("type", "None");
            }
        }
    }

    if (currentStyleComponent)
    {
        currentStyleComponent->removingTrack(uuid);
    }

    auto appDataFolder = IOHelper::getFolder("Piano Synth2");

    auto jsonFile = IOHelper::getFile("allStyles.json");

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
        auto* styleObj = styleVariable.getDynamicObject();
        if (styleObj == nullptr)
            continue;

        juce::var trackVariable = styleObj->getProperty("tracks");

        if (!trackVariable.isArray())
            continue;

        for (auto& trackVar : *trackVariable.getArray())
        {
            auto* trackObj = trackVar.getDynamicObject();
            if (trackObj == nullptr)
                continue;

            juce::String uuidString = trackObj->getProperty("uuid").toString();
            if (uuidSet.count(uuidString))
            {
                trackObj->setProperty("uuid", "noID");
                trackObj->setProperty("name", "None");
                trackObj->setProperty("instrumentNumber", -1);
                trackObj->setProperty("type", "None");
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

void Display::updateTrackNameFromAllStyles(const juce::Uuid& uuid, const juce::String& newName)
{
    if (!allStylesJsonVar.isObject())
        return;

    auto stylesArray = allStylesJsonVar["styles"];

    if (!stylesArray.isArray())
        return;

    for (auto& styleVar : *stylesArray.getArray())
    {
        auto* styleObj = styleVar.getDynamicObject();

        if (styleObj == nullptr)
            continue;

        juce::var trackVariable = styleObj->getProperty("tracks");

        if (!trackVariable.isArray())
            continue;

        for (auto& trackVar : *trackVariable.getArray())
        {
            auto* trackObj = trackVar.getDynamicObject();
            if (trackObj == nullptr)
                continue;

            juce::String uuidString = trackObj->getProperty("uuid").toString();
            if (uuidString == uuid.toString())
            {
                trackObj->setProperty("name", newName);
            }
        }
    }

    if (currentStyleComponent)
    {
        currentStyleComponent->renamingTrack(uuid, newName);
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
        styleObj->setProperty("StyleID", "style_" + juce::String(i+1));

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
    auto appDataFolder = IOHelper::getFolder("Piano Synth2");

    auto file= IOHelper::getFile("allStyles.json");

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

    auto appDataFolder = IOHelper::getFolder("Piano Synth2");

    auto file = IOHelper::getFile("allStyles.json");

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

    auto appDataFolder = IOHelper::getFolder("Piano Synth2");

    auto jsonFile = IOHelper::getFile("allStyles.json");

    juce::String jsonString = juce::JSON::toString(allStylesJsonVar);
    jsonFile.replaceWithText(jsonString);
}

void Display::appendNewStyleInJson(const juce::String& newName)
{
    if (!allStylesJsonVar.isObject())
        return;

   
    auto* obj = allStylesJsonVar.getDynamicObject();
    if (obj == nullptr)
        return;


    juce::var stylesVar = obj->getProperty("styles");

    if (!stylesVar.isArray())
        return;


    auto* stylesArray = stylesVar.getArray();

    if (stylesArray == nullptr)
        return;

    const int numExistingStyles = stylesArray->size();
    const juce::String styleID = "style_" + juce::String(numExistingStyles);


    auto* newStyle = new juce::DynamicObject();
    newStyle->setProperty("name", newName);
    newStyle->setProperty("BPM", 120.0f);
    newStyle->setProperty("StyleID", styleID);
    juce::Array<juce::var> tracksArray;
    for (int i = 0; i < 8; ++i)
    {
        auto* track = new juce::DynamicObject();
        track->setProperty("name", "None");
        track->setProperty("type", "None");
        track->setProperty("volume", 50.0f);
        track->setProperty("instrumentNumber", -1);
        track->setProperty("uuid", "noID");
        tracksArray.add(track);
    }
    newStyle->setProperty("tracks", tracksArray);
    stylesArray->add(newStyle);

    auto appDataFolder = IOHelper::getFolder("Piano Synth2");

    auto jsonFile = IOHelper::getFile("allStyles.json");

    juce::String jsonString = juce::JSON::toString(allStylesJsonVar);
    jsonFile.replaceWithText(jsonString);
}

void Display::removeStyleInJson(const juce::String& name)
{
    if (!allStylesJsonVar.isObject())
        return;

    auto* obj = allStylesJsonVar.getDynamicObject();
    if (obj == nullptr)
        return;

    juce::var styles = obj->getProperty("styles");
    if (!styles.isArray())
        return;

    auto* stylesArray = styles.getArray();

    for (int i = 0; i < stylesArray->size(); i++)
    {
        auto& style = stylesArray->getReference(i);
        auto* styleObj = style.getDynamicObject();
        if (styleObj == nullptr)
            continue;

        juce::String styleName = styleObj->getProperty("name").toString();
        if (styleName == name)
        {
            stylesArray->remove(i);
            break;
        }
    }

    auto appDataFolder = IOHelper::getFolder("Piano Synth2");

    auto jsonFile = IOHelper::getFile("allStyles.json");

    juce::String jsonString = juce::JSON::toString(allStylesJsonVar);
    jsonFile.replaceWithText(jsonString);
}

std::unordered_map<juce::Uuid, TrackEntry*> Display::buildTrackUuidMap()
{
    std::unordered_map<juce::Uuid, TrackEntry*> map;

    for (auto& [folderName, tracks] : *groupedTracks)
    {
        for (auto& tr : tracks)
        {
            map[tr.getUniqueID()] = &tr;
        }
    }

    return map;
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

            menu.addItem("Delete", [this]() {
                removeStyle();
                });

            menu.showMenuAsync(juce::PopupMenu::Options().withTargetComponent(this));
        }
    }
}

void StyleViewComponent::setNameLabel(const juce::String& name)
{
    label.setText(name, juce::dontSendNotification);
}

juce::String StyleViewComponent::getNameLabel() const
{
    return label.getText();
}

void StyleViewComponent::changeNameLabel()
{
    auto* window = new juce::AlertWindow{ "Rename track","Enter a name for your track",juce::AlertWindow::NoIcon };

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

    juce::MessageManager::callAsync([window]()
        {
            if (auto* editor = window->getTextEditor("nameEditor"))
                editor->grabKeyboardFocus();
        });
}

void StyleViewComponent::removeStyle()
{
    juce::AlertWindow::showOkCancelBox(
        juce::AlertWindow::WarningIcon,
        "Confirm Remove",
        "Are you sure you want to remove ' "+label.getText()+ "' ?",
        "Remove",
        "Cancel",
        this,
        juce::ModalCallbackFunction::create(
            [this](int result)
            {
                if (result == 0) // Cancel
                    return;

                if (onStyleRemoveComponent)
                    onStyleRemoveComponent(label.getText());

                juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::InfoIcon, "Remove style", "Style removed succesfully!");

            }
        )
    );
}

StylesListComponent::StylesListComponent(std::vector<juce::String> stylesNamesOut, std::function<void(const juce::String&)> onStyleClicked, int widthSize) : stylesNames{stylesNamesOut}, onStyleClicked{onStyleClicked}
{
    addAndMakeVisible(addButton);

    addButton.setMouseCursor(juce::MouseCursor::PointingHandCursor);

    addButton.onClick = [this]()
    {
        addNewStyle();
    };

    this->widthSize = widthSize;

    styleItemsContainer = std::make_unique<juce::Component>();
    viewport.setViewedComponent(styleItemsContainer.get(), false);
    addAndMakeVisible(viewport);

    populate();
}

void StylesListComponent::resized()
{

    this->widthSize = getWidth();

    const int controlBarHeight = 24;
    const int buttonWidth = 80;
    const int buttonHeight = 20;
    const int spacing = 10;

    addButton.setBounds(spacing/2, (controlBarHeight - buttonHeight) / 2, buttonWidth, buttonHeight);
    viewport.setBounds(0, controlBarHeight, getWidth(), getHeight() - controlBarHeight);

    layoutStyles();
}

void StylesListComponent::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::black);


    const int controlBarHeight = 24;

    auto lightColor = juce::Colour::fromRGB(80, 80, 80);
    auto darkColor = juce::Colour::fromRGB(40, 40, 40);

    juce::ColourGradient gradient(lightColor, 0, 0, darkColor, 0, (float)controlBarHeight, false);
    g.setGradientFill(gradient);
    g.fillRect(0, 0, getWidth(), controlBarHeight);

    auto borderColor = juce::Colours::darkgrey.withAlpha(0.3f);
    g.setColour(borderColor);
    g.drawLine(0.0f, (float)(controlBarHeight - 1), (float)getWidth(), (float)(controlBarHeight - 1), 1.0f);
}

void StylesListComponent::setWidthSize(const int newWidth)
{
    this->widthSize = newWidth;
}

void StylesListComponent::layoutStyles()
{
    const int columns = 2;
    const int spacing = 10;
    const int sidePadding = spacing;

    const int totalSpacing = (columns - 1) * spacing + 2 * sidePadding;
    const int itemWidth = (getWidth() - totalSpacing) / columns;
    const int itemHeight = 50;

    int x = sidePadding;
    int y = spacing;

    int count = 0;
    for (auto* style : allStyles)
    {
        style->setBounds(x, y, itemWidth, itemHeight);
        count++;

        if (count % columns == 0)
        {
            x = sidePadding;
            y += itemHeight + spacing;
        }
        else
        {
            x += itemWidth + spacing;
        }
    }

    int rows = static_cast<int>(std::ceil(allStyles.size() / (float)columns));
    int totalHeight = rows * itemHeight + (rows + 1) * spacing;

    styleItemsContainer->setSize(getWidth(), totalHeight);
}

void StylesListComponent::repopulate()
{
    stylesNames.clear();
    for (auto* sv : allStyles)
        stylesNames.push_back(sv->getNameLabel());

    populate();
    resized();
}

void StylesListComponent::addNewStyle()
{
    auto* window = new juce::AlertWindow{ "Rename track","Enter a name for your track",juce::AlertWindow::NoIcon };

    window->addTextEditor("nameEditor", "", "Style Name:");;

    window->addButton("OK", 1, juce::KeyPress(juce::KeyPress::returnKey));
    window->addButton("Cancel", 0, juce::KeyPress(juce::KeyPress::escapeKey));

    window->enterModalState(true, juce::ModalCallbackFunction::create([this, window](int result)
            {
                std::unique_ptr<juce::AlertWindow> cleanup{ window };
                if (result != 1)
                    return;

                juce::String newName = window->getTextEditor("nameEditor")->getText().trim();
                
                if (newName.isEmpty())
                {
                    juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::InfoIcon, "Adding a style", "Name can't be empty!");
                    return;
                }

                for (auto& name : stylesNames)
                {
                    if (name.trim() == newName)
                    {
                        juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::InfoIcon, "Adding a style", "Name already exists!");
                        return;
                    }
                }

                auto* newStyle = new StyleViewComponent{ newName };

                juce::String currentName = newName;
                
                allCallBacks(newStyle,currentName);

                if (onStyleAdd)
                    onStyleAdd(newName);

                stylesNames.push_back(newName);
                styleItemsContainer->addAndMakeVisible(newStyle);
                allStyles.add(newStyle);
                resized(); // instead of repopulating, we just layout the styles again ( repopulating would mean creating all the lambdas again and so on )

                juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::InfoIcon, "Add style", "The style '" + newName + "' has been added!");
            }
        )
    );

    juce::MessageManager::callAsync([window]()
        {
            if (auto* editor = window->getTextEditor("nameEditor"))
                editor->grabKeyboardFocus();
        });
}

void StylesListComponent::allCallBacks(StyleViewComponent* newStyle, const juce::String& currentName)
{
    newStyle->onStyleClicked = this->onStyleClicked;

    newStyle->onStyleRenamed = [this](const juce::String& oldName, const juce::String& newName)
    {
        onStyleRename(oldName, newName);
    };

    newStyle->onStyleRemoveComponent = [this](const juce::String& name)
    {
        removeStyleLocally(name);
    };

    newStyle->isInListNames = [this, currentName](const juce::String& newName)
    {
        bool isIn = false;
        for (auto& nameInList : stylesNames)
        {
            if (nameInList == newName && newName != currentName)
            {
                isIn = true;
                break;
            }
        }
        return isIn;
    };
}

void StylesListComponent::removeStyleLocally(const juce::String& name)
{
    for (int i = 0; i < allStyles.size(); ++i)
    {
        if (allStyles[i]->getNameLabel() == name)
        {
            auto* comp = allStyles[i];
            styleItemsContainer->removeChildComponent(comp);
            allStyles.remove(i); 
            break;
        }
    }

    stylesNames.clear();
    for (auto* sv : allStyles)
        stylesNames.push_back(sv->getNameLabel());

    if (onStyleRemove) 
        onStyleRemove(name);

    styleItemsContainer->removeAllChildren();
    allStyles.clear();
    populate();
    resized();
}

void StylesListComponent::addStyleLocally(const juce::String& newName)
{
    stylesNames.push_back(newName);

    styleItemsContainer->removeAllChildren();
    allStyles.clear();
    populate();
    resized();
}

void StylesListComponent::rebuildStyleNames()
{
    DBG("rebuildStyleNames called, stylesNames size before clear: " + juce::String(stylesNames.size()));
    stylesNames.clear();
    for (auto& style : allStyles)
    {
        juce::String name = style->getNameLabel();
        stylesNames.push_back(name);
    }
    DBG("rebuildStyleNames done, stylesNames size after rebuild: " + juce::String(stylesNames.size()));
}


void StylesListComponent::populate()
{
    styleItemsContainer->removeAllChildren();
    allStyles.clear();
    for (int i = 0; i < stylesNames.size(); i++)
    {
        juce::String name;
        if (i < stylesNames.size())
            name = stylesNames[i];
        else name = "Style " + juce::String(i);
        auto* newStyle = new StyleViewComponent{ name };

        juce::String currentName = name;
         
        allCallBacks(newStyle, currentName);

        styleItemsContainer->addAndMakeVisible(newStyle);
        allStyles.add(newStyle);
    }
}


void CurrentStyleComponent::startPlaying()
{
    int selectedID = playSettingsTracks.getSelectedId();
    if (selectedID == 3)
        selectedID = 1; //if the keybinds tab is selected, move it to the default one, (first one=play all tracks)
    
    playSettingsTracks.setSelectedId(selectedID);

    stopPlaying();

    std::vector<TrackEntry> selectedTracks;

    if (auto midiOut=outputDevice.lock())
    {
        if (selectedID == 1)
        {
            for (const auto& tr : allTracks)
            {
                auto it = mapUuidToTrackEntry.find(tr->getUsedID());
                if (it != mapUuidToTrackEntry.end() && it->second != nullptr)
                {
                    it->second->instrumentAssociated = tr->getInstrumentNumber();
                    it->second->volumeAssociated = tr->getVolume();
                    selectedTracks.push_back(*it->second);
                }
            }
        }
        else if (selectedID == 2)
        {
            if (lastSelectedTrack)
            {
                auto it = mapUuidToTrackEntry.find(lastSelectedTrack->getUsedID());
                if (it != mapUuidToTrackEntry.end())
                    selectedTracks.push_back(*it->second);
            }
        }
        if (selectedTracks.empty())
        {
            juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::WarningIcon, "Playing tracks", "There are no tracks to play");
            return;
        }
        trackPlayer->setTracks(selectedTracks);

        trackPlayer->syncPlaybackSettings();
        trackPlayer->start();
    }
    else {
        DBG("Output is null!");
        return;
    }

}

CurrentStyleComponent::CurrentStyleComponent(const juce::String& name, std::unordered_map<juce::Uuid, TrackEntry*>& map, std::weak_ptr<juce::MidiOutput> outputDevice) : name(name), mapUuidToTrackEntry(map), outputDevice(outputDevice)
{
    addMouseListener(this, true);
    nameOfStyle.setText(name, juce::dontSendNotification);
    nameOfStyle.setTooltip(name);
    addAndMakeVisible(nameOfStyle);

    playSettingsTracks.setMouseCursor(juce::MouseCursor::PointingHandCursor);

    playSettingsTracks.addItem("All tracks", 1);
    playSettingsTracks.addItem("Solo track(last selected)", 2);
    playSettingsTracks.addItem("Settings",3);
    playSettingsTracks.setSelectedId(1);
    playSettingsTracks.addListener(this);

    addAndMakeVisible(playSettingsTracks);
    addAndMakeVisible(startPlayingTracks);
    addAndMakeVisible(stopPlayingTracks);
    addAndMakeVisible(customBeatBar);


    trackPlayer = std::make_unique<MultipleTrackPlayer>(outputDevice);

    trackPlayer->onElapsedUpdate = [this](double ElapsedBeats)
    {
        customBeatBar.setCurrentBeatsElapsed(ElapsedBeats);
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

        newTrack->onShowInformation= [this](const juce::Uuid& uuid, int channel)
        {
            showingTheInformationNotesFromTrack(uuid, channel);
        };

        newTrack->syncVolumePercussionTracks = [this](double newVolume) {
            syncPercussionTracksVolumeChange(newVolume);
        };

        newTrack->onRequestTrackSelection = [this,newTrack](std::function<void(const juce::String&, const juce::Uuid& uuid, const juce::String& type)> trackChosenCallback)
        {
            if (onRequestTrackSelectionFromTrack)
                onRequestTrackSelectionFromTrack(trackChosenCallback);

            

            //DBG("Current tempo before change:" + juce::String(currentTempo));
            //applyBPMchangeForOne(currentTempo, newTrack->getUsedID());
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

    /*
    tempoSlider.onValueChange = [this]()
    {
        oldTempo = currentTempo;
        currentTempo = tempoSlider.getValue();
    };
    */

    tempoSlider.onDragEnd = [this]()
    {
        oldTempo = currentTempo;
        currentTempo = tempoSlider.getValue();

        if (trackPlayer)
            trackPlayer->setCurrentBPM(currentTempo);
        applyBPMchangeBeforePlayback(currentTempo);

        DBG("Activated!");
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

void CurrentStyleComponent::setDeviceOutputCurrentStyle(std::weak_ptr<juce::MidiOutput> newOutput)
{
    this->outputDevice = newOutput;
    if (trackPlayer)
        trackPlayer->setDeviceOutputTrackPlayer(newOutput);
}

void CurrentStyleComponent::applyChangesForOneTrack(TrackEntry& track)
{

    auto it = track.styleChangesMap.find(styleID);
    if (it != track.styleChangesMap.end())
    {
        std::vector<TrackIOHelper::NotePair> toApply;
        TrackIOHelper::extractNotePairEvents(track.sequence, toApply);
        TrackIOHelper::applyChangesToASequence(toApply, it->second);

        track.sequence.sort();
        track.sequence.updateMatchedPairs();
    }
}

void CurrentStyleComponent::applyChangesForAllTracksCurrentStyle()
{
    for (auto& [uuid, track] : mapUuidToTrackEntry)
    {
        if (track)
            applyChangesForOneTrack(*track);
    }

}

void CurrentStyleComponent::removingTrack(const juce::Uuid& uuid)
{
    for (auto& track : allTracks)
    {
        if (track->getUsedID() == uuid)
        {
            track->setNameLabel("None");
            track->setUUID(juce::Uuid{ "noID" });
            track->setTypeOfTrack("None");
            track->setChannel(2);
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
                track->setTypeOfTrack("None");
                track->setChannel(2);
                break;
            }
        }
    }
}

void CurrentStyleComponent::renamingTrack(const juce::Uuid& uuid, const juce::String& newName)
{
    for (auto& track:allTracks)
    {
        if (track->getUsedID() == uuid)
            track->setNameLabel(newName);
    }
}

void CurrentStyleComponent::showingTheInformationNotesFromTrack(const juce::Uuid& uuid, int channel)
{
    juce::MidiMessageSequence* sequence = nullptr;
    std::unordered_map<int, MidiChangeInfo>* changeMap = nullptr;
    juce::String displayName = "<Unnamed>";

    auto it = mapUuidToTrackEntry.find(uuid);
    if (it != mapUuidToTrackEntry.end())
    {
        auto& track = it->second;
        if (track != nullptr)
        {
            sequence = &track->sequence;
            changeMap = &track->styleChangesMap[styleID];

            if (track->displayName.isNotEmpty())
                displayName = track->displayName;
        }
    }

    // Fallbacks if no data
    static juce::MidiMessageSequence emptySequence;
    static std::unordered_map<int, MidiChangeInfo> emptyMap;

    auto container = std::make_unique<TableContainer>(
        sequence ? *sequence : emptySequence,
        displayName,
        channel,
        changeMap ? *changeMap : emptyMap
        );

    container->setSize(350, 340);

    container->updateToFile = [this]()
    {
        if (updateTrackFile)
            updateTrackFile();
    };

    if (container)
    {
        container->addModelAsListener(trackPlayer.get());
        container->removeModelFromListener = [this](TrackPlayerListener* listener)
        {
            if (trackPlayer)
                trackPlayer->removeListener(listener);
        };
    }

    juce::CallOutBox::launchAsynchronously(
        std::move(container),
        getScreenBounds(),
        nullptr
    );

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

void CurrentStyleComponent::applyBPMchangeBeforePlayback(double userBPM, bool whenLoading, bool applyStyleChanges)
{
    if (userBPM <= 0.0)
        userBPM = 120.0;

    currentTempo = userBPM;

    if (trackPlayer)
        trackPlayer->setCurrentBPM(currentTempo);

    int channelCounter = 0;

    for (const auto& trackPtr : allTracks)
    {
        auto& tr = mapUuidToTrackEntry[trackPtr->getUsedID()];
        if (tr == nullptr)
            continue;

        int targetChannel = (tr->type == TrackType::Percussion) ? 10 : (channelCounter + 2);
        if (tr->type != TrackType::Percussion)
            ++channelCounter;

        double originalBPM = (tr->originalBPM > 0.0) ? tr->originalBPM : 120.0;
        double ratio = originalBPM / userBPM;

        juce::MidiMessageSequence scaledSequence;

        for (int i = 0; i < tr->originalSequenceTicks.getNumEvents(); ++i)
        {
            const auto& event = tr->originalSequenceTicks.getEventPointer(i)->message;
            double scaledTime = event.getTimeStamp() * ratio;

            juce::MidiMessage newMsg = event;
            newMsg.setTimeStamp(scaledTime);
            newMsg.setChannel(targetChannel);

            scaledSequence.addEvent(newMsg);
        }

        scaledSequence.sort();
        scaledSequence.updateMatchedPairs();

        double firstNoteOnTime = 0.01;
        for (int i = 0; i < scaledSequence.getNumEvents(); ++i)
        {
            const auto& msg = scaledSequence.getEventPointer(i)->message;
            if (msg.isNoteOn())
            {
                firstNoteOnTime = std::max(0.01, msg.getTimeStamp() - 0.005);
                break;
            }
        }

        if (tr->volumeAssociated != -1)
        {
            scaledSequence.addEvent(
                juce::MidiMessage::controllerEvent(targetChannel, 7, (int)tr->volumeAssociated)
                .withTimeStamp(firstNoteOnTime - 0.004));
        }

        if (targetChannel != 10 && tr->instrumentAssociated != -1)
        {
            scaledSequence.addEvent(
                juce::MidiMessage::programChange(targetChannel, tr->instrumentAssociated)
                .withTimeStamp(firstNoteOnTime - 0.002));
        }

        tr->sequence = scaledSequence;
    }

    if (anyTrackChanged)
        anyTrackChanged();
}

void CurrentStyleComponent::applyBPMchangeForOne(double userBPM, const juce::Uuid& uuid)
{
    if (userBPM <= 0.0)
        userBPM = 120.0;

    currentTempo = userBPM;

    if (trackPlayer)
        trackPlayer->setCurrentBPM(currentTempo);

    int channelCounter = 0;
    int targetChannel = 10;  

    for (const auto& trackPtr : allTracks)
    {
        auto& tr = mapUuidToTrackEntry[trackPtr->getUsedID()];
        if (tr == nullptr)
            continue;

        if (trackPtr->getUsedID() == uuid)
        {
            targetChannel = (tr->type == TrackType::Percussion) ? 10 : (channelCounter + 2);
            break;
        }

        if (tr->type != TrackType::Percussion)
            ++channelCounter;
    }

    auto& tr = mapUuidToTrackEntry[uuid];
    if (tr == nullptr)
    {
        DBG("Track not found in mapUuidToTrackEntry for uuid: " + uuid.toString());
        return;
    }

    juce::MidiMessageSequence scaledSequence;

    double trackOriginalBPM = (tr->originalBPM > 0.0) ? tr->originalBPM : 120.0;

    DBG("BPMS: " + juce::String(userBPM) + " " + juce::String(trackOriginalBPM));

    for (int i = 0; i < tr->originalSequenceTicks.getNumEvents(); ++i)
    {
        const auto& event = tr->originalSequenceTicks.getEventPointer(i)->message;

        double scaledTime = event.getTimeStamp() * (trackOriginalBPM / userBPM);

        juce::MidiMessage newMsg = event;
        newMsg.setTimeStamp(scaledTime);
        newMsg.setChannel(targetChannel);

        scaledSequence.addEvent(newMsg);
    }

    scaledSequence.sort();

    double firstNoteOnTime = 0.01;
    for (int i = 0; i < scaledSequence.getNumEvents(); ++i)
    {
        const auto& msg = scaledSequence.getEventPointer(i)->message;
        if (msg.isNoteOn())
        {
            firstNoteOnTime = std::max(0.01, msg.getTimeStamp() - 0.005);
            break;
        }
    }

    if (tr->volumeAssociated != -1)
    {
        scaledSequence.addEvent(
            juce::MidiMessage::controllerEvent(targetChannel, 7, (int)tr->volumeAssociated).withTimeStamp(firstNoteOnTime - 0.004));
    }

    if (targetChannel != 10 && tr->instrumentAssociated != -1)
    {
        scaledSequence.addEvent(
            juce::MidiMessage::programChange(targetChannel, tr->instrumentAssociated).withTimeStamp(firstNoteOnTime - 0.002));
    }

    scaledSequence.updateMatchedPairs();

    tr->sequence = scaledSequence;
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

void CurrentStyleComponent::comboBoxChanged(juce::ComboBox* comboBoxThatHasChanged)
{
    if (comboBoxThatHasChanged == &playSettingsTracks)
    {
        int id = playSettingsTracks.getSelectedId();
        if (id == 3)
        {
            keybindTabStarting();
            playSettingsTracks.setSelectedId(1);
        }
    }
}

void CurrentStyleComponent::comboBoxChangeIndex(int Index)
{
    playSettingsTracks.setSelectedItemIndex(Index);
}

void CurrentStyleComponent::setTempo(double newTempo)
{
    oldTempo = currentTempo;
    this->currentTempo = newTempo;
    this->tempoSlider.setValue(newTempo);
}

void CurrentStyleComponent::setStyleID(const juce::String& newID)
{
    this->styleID = newID;
}


void CurrentStyleComponent::resized()
{
    float width = getWidth() / 8.0f;
    float height = 80.0f;
    float initialX = 0;
    float y = getHeight() - height;
    int yInt = static_cast<int>(y);

    int labelWidth = 50;
    int playSettingsWidth = 40;
    int heightFirst = 20;
    int startStopWidth = 35;
    int padding = 2;

    // Clamp heights so they never go below or above the track start y
    int playSettingsHeight = juce::jmin(heightFirst, yInt)-padding;
    int nameOfStyleHeight = juce::jmin(heightFirst, yInt)-padding;
    int startStopHeight = juce::jmin(heightFirst - padding * 2, yInt)-padding;
    int beatBarHeight = juce::jmin(heightFirst - padding * 2, yInt)-padding;
    int tempoSliderHeight = juce::jmin(heightFirst, yInt)-padding;

    playSettingsTracks.setBounds(getWidth() - playSettingsWidth, 0, playSettingsWidth, playSettingsHeight);
    playSettingsTracks.setMouseCursor(juce::MouseCursor::PointingHandCursor);

    nameOfStyle.setBounds((getWidth() - labelWidth) / 2, 0, getWidth() / 6 - 15, nameOfStyleHeight);

    startPlayingTracks.setBounds(0, padding, startStopWidth - 2, startStopHeight);
    startPlayingTracks.setButtonText("Start");
    startPlayingTracks.setMouseCursor(juce::MouseCursor::PointingHandCursor);

    stopPlayingTracks.setBounds(startPlayingTracks.getWidth() + 5, padding, startStopWidth, startStopHeight);
    stopPlayingTracks.setButtonText("Stop");
    stopPlayingTracks.setMouseCursor(juce::MouseCursor::PointingHandCursor);

    int xStarting = stopPlayingTracks.getX() + stopPlayingTracks.getWidth() + 5;
    customBeatBar.setBounds(xStarting, padding, nameOfStyle.getX() - xStarting, beatBarHeight);

    int paddingBetween = 5; 
    int sliderX = nameOfStyle.getX() + nameOfStyle.getWidth() + paddingBetween;
    int sliderWidth = playSettingsTracks.getX() - sliderX - paddingBetween;
    tempoSlider.setBounds(sliderX, 0, sliderWidth, tempoSliderHeight);

    for (int i = 0; i < allTracks.size(); i++)
    {
        allTracks[i]->setBounds(juce::Rectangle<int>(
            static_cast<int>(i * width),
            yInt,
            static_cast<int>(width),
            static_cast<int>(height)
            ));
    }
}


void CurrentStyleComponent::updateName(const juce::String& newName)
{
    name = newName;
    nameOfStyle.setText(name, juce::dontSendNotification);
    nameOfStyle.setTooltip(name);
}

CurrentStyleComponent::~CurrentStyleComponent()
{
    if (trackPlayer)
    {
        for (auto& track : allTracks)
            track->removeListener(trackPlayer.get());
    }
    if (&playSettingsTracks)
        playSettingsTracks.removeListener(this);
}

juce::String CurrentStyleComponent::getName()
{
    return name;
}

void CurrentStyleComponent::triggerStopClick()
{
    this->stopPlayingTracks.triggerClick();
}

void CurrentStyleComponent::triggerStartClick()
{
    this->startPlayingTracks.triggerClick();
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
    styleObj->setProperty("StyleID", styleID);

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
    setStyleID(obj->getProperty("StyleID").toString());

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
                        DBG("Track selection uuid:" + uuid.toString());
                        setUUID(uuid);
                        setTypeOfTrack(type);
                        if (type.trim() == "percussion")
                            this->channel = 10;
                    }
                );
            }
        }
        else if(event.mods.isRightButtonDown()){
            juce::PopupMenu menu;
            menu.addItem("Rename", [this]() 
                {
                    renameOneTrack();
                });
            menu.addItem("Delete", [this]() 
                { 
                    deleteOneTrack();
                });
            menu.addItem("Copy", [this]() 
                { 
                    copyOneTrack();
                });

            menu.addItem("Paste", [this]() 
                {
                    pasteOneTrack();
                });

            menu.addItem("Notes information", [this]()
                {
                   showNotesInformation();
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
    auto* window = new juce::AlertWindow{ "Rename track","Enter a name for your track",juce::AlertWindow::NoIcon};

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

    juce::MessageManager::callAsync([window]()
        {
            if (auto* editor = window->getTextEditor("nameEditor"))
                editor->grabKeyboardFocus();
        });
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

void Track::showNotesInformation()
{
    if (onShowInformation)
        onShowInformation(uniqueIdentifierTrack, channel);
}

TrackListComponent::TrackListComponent(std::shared_ptr<std::deque<TrackEntry>> tracks,
    std::shared_ptr<std::unordered_map<juce::String, std::deque<TrackEntry>>> groupedTracksMap,
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
    addAndMakeVisible(renameButtonFolder);

    addButtonFolder.setMouseCursor(juce::MouseCursor::PointingHandCursor);
    removeButtonFolder.setMouseCursor(juce::MouseCursor::PointingHandCursor);
    renameButtonFolder.setMouseCursor(juce::MouseCursor::PointingHandCursor);

    addButtonFolder.onClick = [this] {
        addToFolderList();
    };

    removeButtonFolder.onClick = [this] {
        removeFromFolderList();
    };

    renameButtonFolder.onClick = [this] {
        renameFromFolderList();
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

    if (renameButtonFolder.isVisible())
        renameButtonFolder.setBounds(controlBarArea.removeFromLeft(smallButtonWidth).reduced(spacing));

    if (backButton)
        backButton->setBounds(controlBarArea.removeFromLeft(60).reduced(spacing));

    if (sortComboBox)
        sortComboBox->setBounds(controlBarArea.removeFromLeft(120).reduced(spacing));

    if (addButton)
        addButton->setBounds(controlBarArea.removeFromLeft(80).reduced(spacing));

    if (removeButton)
        removeButton->setBounds(controlBarArea.removeFromLeft(80).reduced(spacing));

    if (renameButton)
        renameButton->setBounds(controlBarArea.removeFromLeft(80).reduced(spacing));

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

                double originalBpm = TrackIOHelper::getOriginalBpmFromFile(midiFile);
                TrackIOHelper::convertTicksToSeconds(midiFile, originalBpm);

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

                    newEntry.sequence.updateMatchedPairs();
                    newEntry.originalBPM = originalBpm;

                    newEntry.folderName = currentFolderName;

                    if (TrackIOHelper::foundPercussion(trackSequence))
                        newEntry.type = TrackType::Percussion;
                    else newEntry.type = TrackType::Melodic;

                    newEntry.uuid = TrackEntry::generateUUID();

                    availableTracks->push_back(newEntry);
                    (*groupedTracks)[currentFolderName].push_back(newEntry);

                    TrackEntry* storedPtr = &(*groupedTracks)[currentFolderName].back();

                    if (addToMapOnAdding)
                        addToMapOnAdding(storedPtr);

                    ++addedTracks;
                }

                listBox.updateContent();
                listBox.repaint();
                auto appDataFolder = IOHelper::getFolder("Piano Synth2");

                auto jsonFile = IOHelper::getFile("myTracks.json");

                TrackIOHelper::saveToFile(jsonFile, *groupedTracks);

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

                auto appDataFolder = IOHelper::getFolder("Piano Synth2");

                auto jsonFile = IOHelper::getFile("myTracks.json");
                TrackIOHelper::saveToFile(jsonFile, *groupedTracks);
            }
        )
    );
}

void TrackListComponent::renameFromTrackList()
{
    juce::SparseSet<int> selectedRows = listBox.getSelectedRows();

    if (selectedRows.isEmpty())
    {
        juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::WarningIcon, "Rename Track", "Please select one track to rename");
        return;
    }

    if (selectedRows.size() > 1)
    {
        juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::WarningIcon, "Rename track", "Please select only one track to rename");
        return;
    }

    auto* window = new juce::AlertWindow("Rename track",
        "Enter a new name for the track:",
        juce::AlertWindow::NoIcon);

    const int selectedIndex = selectedRows[0];

    auto& track = (*availableTracks)[selectedIndex];
    juce::String oldName = track.displayName;
    juce::Uuid trackUuid = track.uuid;

    window->addTextEditor("trackName", oldName);
    window->addButton("OK", 1, juce::KeyPress(juce::KeyPress::returnKey));
    window->addButton("Cancel", 0, juce::KeyPress(juce::KeyPress::escapeKey));

    window->enterModalState(true, juce::ModalCallbackFunction::create([this, oldName, window, trackUuid](int result)
        {
            std::unique_ptr<juce::AlertWindow> cleanup(window);
            if (result != 1)
                return;


            juce::String newName = window->getTextEditor("trackName")->getText().trim();

            if (oldName == newName)
                return;

            TrackEntry* track=nullptr;
            for (auto& tr : *availableTracks)
            {
                if (tr.displayName == newName)  // no need for tr.displayName!=oldName cause of the previous check with return
                {
                    juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::WarningIcon,
                        "Rename track", "A track with this name already exists.");
                    return;
                }
                else if (tr.displayName == oldName)
                    track = &tr;
            }

            if(track!=nullptr)
                track->displayName = newName;

            auto& tracks = (*groupedTracks)[currentFolderName];

            for (auto& tr : tracks)
            {
                if (tr.displayName == oldName)
                {
                    tr.displayName = newName;
                    break;
                }
            }

            
            comboBoxChanged(sortComboBox.get());


            listBox.deselectAllRows();
            listBox.updateContent();
            listBox.repaint();

            auto appDataFolder = IOHelper::getFolder("Piano Synth2");

            auto jsonFile = IOHelper::getFile("myTracks.json");
            TrackIOHelper::saveToFile(jsonFile, *groupedTracks);

            if (onRenameTrackFromList)
                onRenameTrackFromList(trackUuid, newName);

        }
    ));

    juce::MessageManager::callAsync([window]()
        {
            if (auto* editor = window->getTextEditor("folderName"))
                editor->grabKeyboardFocus();
        });

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

            auto appDataFolder = IOHelper::getFolder("Piano Synth2");

            auto jsonFile = IOHelper::getFile("myTracks.json");
            TrackIOHelper::saveToFile(jsonFile, *groupedTracks);

        }));

    juce::MessageManager::callAsync([window]()
        {
            if (auto* editor = window->getTextEditor("folderName"))
                editor->grabKeyboardFocus();
        });
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

                    auto appDataFolder = IOHelper::getFolder("Piano Synth2");

                    auto jsonFile = IOHelper::getFile("myTracks.json");
                    TrackIOHelper::saveToFile(jsonFile, *groupedTracks);
                }
            }
        )
    );
}

void TrackListComponent::renameFromFolderList()
{
    juce::SparseSet<int> selectedRows = listBox.getSelectedRows();

    if (selectedRows.isEmpty())
    {
        juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::WarningIcon, "Rename Folder", "Please select one folder to rename");
        return;
    }

    if (selectedRows.size() > 1)
    {
        juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::WarningIcon, "Rename folder", "Please select only one folder to rename");
        return;
    }

    auto* window = new juce::AlertWindow("Rename Folder",
        "Enter a name for the folder:",
        juce::AlertWindow::NoIcon);

    const int selectedIndex = selectedRows[0];

    juce::String oldName = (* groupedTrackKeys)[selectedIndex];

    window->addTextEditor("folderName", oldName);
    window->addButton("OK", 1, juce::KeyPress(juce::KeyPress::returnKey));
    window->addButton("Cancel", 0, juce::KeyPress(juce::KeyPress::escapeKey));

    window->enterModalState(true, juce::ModalCallbackFunction::create([this, oldName, window](int result)
        {
            std::unique_ptr<juce::AlertWindow> cleanup(window);
            if (result != 1)
                return;


            juce::String newName = window->getTextEditor("folderName")->getText().trim();

            if (oldName == newName)
                return;

            if (groupedTracks->find(newName) != groupedTracks->end())
            {
                juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::WarningIcon,
                    "Rename folder", "A folder with this name already exists.");
                return;
            }

            for (auto& folder : *groupedTrackKeys)
            {
                if (folder == oldName)
                {
                    folder = newName;
                    break;
                }
            }

            auto it = groupedTracks->find(oldName);
            if (it != groupedTracks->end())
            {
                auto entries = std::move(it->second);
                groupedTracks->erase(it);
                (*groupedTracks)[newName] = std::move(entries);
            }

            listBox.deselectAllRows();
            listBox.updateContent();
            listBox.repaint();

            auto appDataFolder = IOHelper::getFolder("Piano Synth2");

            auto jsonFile = IOHelper::getFile("myTracks.json");
            TrackIOHelper::saveToFile(jsonFile, *groupedTracks);
            
        }
    ));

    juce::MessageManager::callAsync([window]()
        {
            if (auto* editor = window->getTextEditor("folderName"))
                editor->grabKeyboardFocus();
        });
}

void TrackListComponent::backToFolderView()
{
    viewMode = ViewMode::FolderView;
    deallocateTracksFromList();
    addButtonFolder.setVisible(true);
    removeButtonFolder.setVisible(true);
    renameButtonFolder.setVisible(true);
    listBox.updateContent();
    listBox.repaint();
}

//the flat list availableTracks doesn't need to be built rn, because we have grouped tracks and that's structured, but for future if
//i wanna change anything, like to have idk, something with all the tracks, i won't need to iterate again through every structured folder name to get all the available tracks

std::deque<TrackEntry>& TrackListComponent::getAllAvailableTracks() const
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

void TrackListComponent::initializeTracksFromList()
{
    viewMode = ViewMode::TrackView;
    addButtonFolder.setVisible(false);
    removeButtonFolder.setVisible(false);
    renameButtonFolder.setVisible(false);

    backButton = std::make_unique<juce::TextButton>("Back");
    addButton = std::make_unique<juce::TextButton>("Add");
    removeButton = std::make_unique<juce::TextButton>("Remove");
    sortComboBox = std::make_unique<juce::ComboBox>();
    renameButton = std::make_unique<juce::TextButton>("Rename");

    sortComboBox->addListener(this);

    backButton->setMouseCursor(juce::MouseCursor::PointingHandCursor);
    addButton->setMouseCursor(juce::MouseCursor::PointingHandCursor);
    removeButton->setMouseCursor(juce::MouseCursor::PointingHandCursor);
    renameButton->setMouseCursor(juce::MouseCursor::PointingHandCursor);


    backButton->onClick = [this]() {
        backToFolderView();
    };

    addButton->onClick = [this]() {
        addToTrackList();
    };

    removeButton->onClick = [this]() {
        removeFromTrackList();
    };

    renameButton->onClick = [this]()
    {
        renameFromTrackList();
    };

    addAndMakeVisible(backButton.get());
    addAndMakeVisible(addButton.get());
    addAndMakeVisible(removeButton.get());
    addAndMakeVisible(sortComboBox.get());
    addAndMakeVisible(renameButton.get());

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

MyTabbedComponent::MyTabbedComponent(juce::TabbedButtonBar::Orientation orientation) : juce::TabbedComponent(orientation)
{
    getTabbedButtonBar().addMouseListener(this, true);
    
}

MyTabbedComponent::~MyTabbedComponent()
{
    getTabbedButtonBar().removeMouseListener(this);
}

void MyTabbedComponent::currentTabChanged(int newCurrentTabIndex, const juce::String& newTabName)
{
    if (onTabChanged)
        onTabChanged(newCurrentTabIndex, newTabName);
}

void MyTabbedComponent::mouseDown(const juce::MouseEvent& event)
{
    if (!event.mods.isRightButtonDown()) return;

    int numTabs = getNumTabs();
    if (numTabs <= 1) return;

    int tabWidth = getWidth() / numTabs;
    int clickedIndex = event.x / tabWidth;

    if (clickedIndex > 0 && clickedIndex < numTabs)
    {
        juce::PopupMenu menu;
        menu.addItem("Close Tab", [this, clickedIndex]() { removeTab(clickedIndex); });
        menu.showMenuAsync(juce::PopupMenu::Options());
    }
}
