/*
  ==============================================================================

    displayGUI.cpp
    Created: 8 Jun 2025 2:44:43am
    Author:  Kisuke

  ==============================================================================
*/

#include "displayGUI.h"
#include "InstrumentChooser.h"


void convertTicksToSeconds(juce::MidiFile& midiFile, double bpm=120.0)
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

Display::Display(int widthForList, juce::MidiOutput* outputDev): outputDevice{outputDev}
{
    createUserTracksFolder();
    tabComp = std::make_unique<MyTabbedComponent>(juce::TabbedButtonBar::TabsAtLeft);
    addAndMakeVisible(tabComp.get());

    auto* list = new StylesListComponent{ 10, [this](const juce::String& name) {
        showCurrentStyleTab(name); },widthForList
    };

    auto* scrollableView = new juce::Viewport();
    scrollableView->setScrollBarsShown(true, false); // only vertical
    scrollableView->setViewedComponent(list, true);
    tabComp->addTab("Styles", juce::Colour::fromRGB(10, 15, 10), scrollableView, true);

    list->resized();
    scrollableView->getViewedComponent()->resized();

    initializeAllStyles();
    loadAllStyles();

    tabComp->onTabChanged = [this](int newIndex, juce::String tabName)
    {
        if (createdTracksTab && tabName != "My tracks")
        {
            int myTracksIndex = tabComp->getNumTabs() - 1;
            tabComp->removeTab(myTracksIndex);
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
        [](int) {}
    );
    trackListComp->loadFromFile(jsonFile); //design problem, i need to pass here basically when i load from the file, the current base tempo but since every style has his own tempo, then????

    mapNameToTrack = trackListComp->buildTrackNameMap();
}

Display::~Display()
{

}

void Display::changeListenerCallback(juce::ChangeBroadcaster* source)
{
    auto* tabbedCast = dynamic_cast<juce::TabbedComponent*>(source);
    if (tabbedCast==tabComp.get())
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
        currentStyleComponent = std::make_unique<CurrentStyleComponent>(name,mapNameToTrack,outputDevice);
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
    currentStyleComponent->anyTrackChanged = [this,name]()
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
    trackListComp = std::make_unique<TrackListComponent>(availableTracksFromFolder,
        [this, onTrackSelected](int index)
        {
            auto& track = availableTracksFromFolder[index];
            juce::String typeString = TrackTypeConversion::toString(track.type);

            onTrackSelected(track.getDisplayName(),track.getUniqueID(),typeString);
            int in = tabComp->getNumTabs() - 2;
            if (in >= 0)
            {
                updateStyleInJson(currentStyleComponent->getName());
                tabComp->setCurrentTabIndex(in);
            }
        });

    trackListComp->onRemoveTracks = [this](const juce::Uuid& uuid)
    {
        removeTrackFromAllStyles(uuid);
    };
    tabComp->addTab("My tracks", juce::Colour::fromRGB(10, 15, 10), trackListComp.release(), true);
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
    if(currentStyleComponent)
        this->currentStyleComponent->stoppingPlayer();
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
        onStyleClicked(label.getText());
    }
}

StylesListComponent::StylesListComponent(int nrOfStyles, std::function<void(const juce::String&)> onStyleClicked, int widthSize): nrOfStyles{nrOfStyles}, onStyleClicked{onStyleClicked}
{
    this->widthSize = widthSize;
    populate();
}

void StylesListComponent::resized()
{
    this->widthSize = getWidth();
    const int itemWidth = getWidth()/2-10;   // Half of parent initial parent width - with spacing
    const int itemHeight =50;
    const int spacing = 10;
    const int columns = 2;

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
        auto* newStyle = new StyleViewComponent{ "Style " + juce::String(i + 1) };
        newStyle->onStyleClicked = this->onStyleClicked;
        allStyles.add(newStyle);
        addAndMakeVisible(newStyle);
    }

    // Very narrow width; Viewport will stretch it later
    const int totalHeight = nrOfStyles * (itemHeight + spacing) + spacing;
    setSize(widthSize, totalHeight/2);  // key fix: force vertical scrolling
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
    trackPlayer->applyBPMchangeBeforePlayback(baseTempo, currentTempo);

    trackPlayer->syncPlaybackSettings();
    trackPlayer->start();
}

CurrentStyleComponent::CurrentStyleComponent(const juce::String& name,std::unordered_map<juce::Uuid, TrackEntry>& map,juce::MidiOutput* outputDevice): name(name), mapNameToTrackEntry(map), outputDevice(outputDevice)
{
    addMouseListener(this, true);
    nameOfStyle.setText(name, juce::dontSendNotification);
    nameOfStyle.setTooltip(name);
    addAndMakeVisible(nameOfStyle);

    playSettingsTracks.setMouseCursor(juce::MouseCursor::PointingHandCursor);
    playSettingsTracks.addItem("All tracks",1);
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
        if(anyTrackChanged)
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
    int playSettingsWidth = getWidth()/6+30;
    int heightFirst = 20;
    int startStopWidth = 35;
    playSettingsTracks.setBounds(getWidth() - playSettingsWidth,0,playSettingsWidth,heightFirst);
    playSettingsTracks.setMouseCursor(juce::MouseCursor::PointingHandCursor);

    nameOfStyle.setBounds((getWidth()-labelWidth) / 2-40, 0, getWidth() / 6, heightFirst);
    startPlayingTracks.setBounds(0, 0, startStopWidth, heightFirst);
    startPlayingTracks.setButtonText("Start");
    startPlayingTracks.setMouseCursor(juce::MouseCursor::PointingHandCursor);

    stopPlayingTracks.setBounds(startPlayingTracks.getWidth() + 5, 0, startStopWidth, heightFirst);
    stopPlayingTracks.setButtonText("Stop");
    stopPlayingTracks.setMouseCursor(juce::MouseCursor::PointingHandCursor);
    
    elapsedTimeLabel.setBounds(stopPlayingTracks.getX() + stopPlayingTracks.getWidth(), 0, 50, heightFirst);

    //nameOfStyle.setBounds(stopPlayingTracks.getX() + stopPlayingTracks.getWidth() + 5, 0, getWidth() / 6, heightFirst);

    int sliderWidth = playSettingsTracks.getX()- (nameOfStyle.getX() + nameOfStyle.getWidth())+10;
    tempoSlider.setBounds(nameOfStyle.getX() + nameOfStyle.getWidth()-15, 0, sliderWidth, 18);

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

        juce::Uuid uuid{uuidSTR};
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
        if (onChange)
            onChange();

        if(static_cast<int>(volumeSlider.getValue())!=-1 && isPlaying() )
            notify(channel, static_cast<int>(volumeSlider.getValue()), -1);
    };

    addAndMakeVisible(volumeSlider);


    volumeLabel.setText(juce::String(volumeSlider.getValue()), juce::dontSendNotification);
    volumeLabel.setColour(juce::Label::textColourId,juce::Colours::crimson);
    addAndMakeVisible(volumeLabel);

    nameLabel.setText("None", juce::dontSendNotification);
    //nameLabel.setTooltip(nameLabel.getText());
    nameLabel.setColour(juce::Label::textColourId,juce::Colours::white);

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
    auto startY = (getHeight() - heightOfSlider)/2;
    volumeSlider.setBounds(0, startY-10, 15, heightOfSlider);
    volumeLabel.setBounds(20, 0, 30, 40);
    nameLabel.setBounds((getWidth() - 40) / 2, volumeLabel.getHeight()+volumeLabel.getY() + 20, getWidth(), 20);


    int x = volumeSlider.getX() + volumeSlider.getWidth() + 10;
    int maxWidth = getWidth() - x - 5;
    int desiredWidth = 40;
    int actualWidth = juce::jmin(desiredWidth, maxWidth);
    instrumentChooserButton.setBounds(x,nameLabel.getY()-25,maxWidth-2,20);
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

juce::String Track::getName()
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
    std::unique_ptr<InstrumentChooserComponent> chooser=std::make_unique<InstrumentChooserComponent>( instrumentlist );
    chooser->instrumentSelectedFunction = [this](int instrumentIndex, const juce::String& name)
    {
        setInstrumentNumber(instrumentIndex,true);
        nameLabel.setText(name,juce::dontSendNotification);
        if(onChange)
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
    if(usedInstrumentNumber!=-1 && shouldNotify)
        notify(channel, -1, usedInstrumentNumber);
}

int Track::getInstrumentNumber()
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
    if(value.getIntValue()!=-1 && shouldNotify)
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
}

void Track::setUUID(const juce::Uuid& newUUID)
{
    this->uniqueIdentifierTrack = newUUID;
}

void Track::setTypeOfTrack(const juce::String& newType)
{
    this->typeOfTrack = newType;
}

juce::Uuid Track::getUsedID()
{
    return uniqueIdentifierTrack;
}

void Track::setChannel(int newChannel)
{
    this->channel = newChannel;
}

double Track::getVolume()
{
    return this->volumeSlider.getValue();
}

TrackListComponent::TrackListComponent(std::vector<TrackEntry>& tracks, std::function<void(int)> onTrackChosen): availableTracks{tracks}, trackChosenCallBack{onTrackChosen}
{
    addAndMakeVisible(listBox);
    addAndMakeVisible(addButton);
    addAndMakeVisible(removeButton);
    addAndMakeVisible(sortComboBox);
    listBox.setModel(this);
    listBox.updateContent();
    sortComboBox.addListener(this);

    addButton.onClick = [this] {

        addToTrackList();
    };

    removeButton.onClick = [this]
    {
        removeFromTrackList();
    };

    sortComboBox.setMouseCursor(juce::MouseCursor::PointingHandCursor);
    addButton.setMouseCursor(juce::MouseCursor::PointingHandCursor);

    removeButton.setMouseCursor(juce::MouseCursor::PointingHandCursor);

    listBox.setMultipleSelectionEnabled(true);
    sortComboBox.addItem("Sort by Name(ascending)", 1);
    sortComboBox.addItem("Sort by Last Modified (Newest)", 2);
    sortComboBox.addItem("Sort by Last Modified (Oldest)", 3);

    sortComboBox.setSelectedId(1);
}

void TrackListComponent::resized()
{
    auto area = getLocalBounds();

    int controlBarHeight = 36;
    auto controlBarArea = area.removeFromTop(controlBarHeight);
    sortComboBox.setBounds(controlBarArea.removeFromLeft(150).reduced(5));

    addButton.setBounds(controlBarArea.removeFromLeft(80).reduced(5));

    removeButton.setBounds(controlBarArea.removeFromLeft(80).reduced(5));

    listBox.setBounds(area);
}

int TrackListComponent::getNumRows()
{
    return (int)availableTracks.size();
}

void TrackListComponent::comboBoxChanged(juce::ComboBox* comboBoxThatHasChanged)
{
    if (comboBoxThatHasChanged == &sortComboBox)
    {
        int id = sortComboBox.getSelectedId();
        if (id == 1)
        {
            std::sort(availableTracks.begin(), availableTracks.end(), [](const TrackEntry& first, const TrackEntry& second)
                {
                    return first.getDisplayName().toLowerCase() < second.getDisplayName().toLowerCase();
                });
        }
        else if (id == 2)
        {
            std::sort(availableTracks.begin(), availableTracks.end(), [](const TrackEntry& first, const TrackEntry& second)
                {
                    return first.file.getLastModificationTime() > second.file.getLastModificationTime();
                });
        }
        else if (id == 3)
        {
            std::sort(availableTracks.begin(), availableTracks.end(), [](const TrackEntry& first, const TrackEntry& second)
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
    g.drawText(availableTracks[rowNumber].getDisplayName(), 5, 0, width - 10, height, juce::Justification::centredLeft);

    g.setColour(juce::Colours::lightgrey);
    g.drawRect(0, 0, width, height, 1);
}

void TrackListComponent::listBoxItemClicked(int row, const juce::MouseEvent& event)
{
    if (event.getNumberOfClicks() == 2)
    {
        if (trackChosenCallBack)
            trackChosenCallBack(row);
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


                int totalTracks = midiFile.getNumTracks();
                int addedTracks = 0;
                int duplicateTracks = 0;

                for (int i = 0; i < totalTracks; ++i)
                {
                    bool duplicate = false;
                    for (const auto& trackEntry : availableTracks)
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


                    convertTicksToSeconds(midiFile);

                    newEntry.uuid = TrackEntry::generateUUID();

                    availableTracks.push_back(newEntry);
                    ++addedTracks;
                }

                listBox.updateContent();
                listBox.repaint();
                auto appDataFolder = juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
                    .getChildFile("Piano Synth2");
                auto jsonFile = appDataFolder.getChildFile("myTracks.json");
                saveToFile(jsonFile);
                int id = sortComboBox.getSelectedId();
                sortComboBox.setSelectedId(id);
                comboBoxChanged(&sortComboBox);

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
        juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::WarningIcon, "Remove Tracks", "Please select at least one track to remove");
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

                DBG("AM TRECUT");

                for (int i = selectedRows.size() - 1; i >= 0; --i)
                {
                    int rowIndex = selectedRows[i];
                    auto& track = availableTracks[rowIndex];
                    juce::Uuid uuid = track.getUniqueID();

                    availableTracks.erase(availableTracks.begin() + rowIndex);

                    if (onRemoveTracks) onRemoveTracks(uuid);
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


void TrackListComponent::saveToFile(const juce::File& fileToSave)
{
    juce::Array<juce::var> filesArray;
    std::map<juce::File, std::vector<TrackEntry>> grouped;


    for (auto& track : availableTracks)
    {
        grouped[track.file].push_back(track);
    }

    for (auto& [file, tracks] : grouped)
    {
        auto* fileObj = new juce::DynamicObject{};
        fileObj->setProperty("filePath", file.getFullPathName());

        juce::Array<juce::var> trackArray;

        for (auto& tr : tracks)
        {
            auto* trackObject = new juce::DynamicObject{};
            trackObject->setProperty("trackIndex", tr.trackIndex);
            trackObject->setProperty("displayName", tr.displayName);
            trackObject->setProperty("uuid", tr.uuid.toString());

            trackArray.add(juce::var(trackObject));
        }
        fileObj->setProperty("Tracks", trackArray);
        filesArray.add(juce::var(fileObj));

    }

    juce::var jsonVar(filesArray);
    juce::String jsonString = juce::JSON::toString(jsonVar);
    fileToSave.replaceWithText(jsonString);
}

void TrackListComponent::loadFromFile(const juce::File& fileToLoad)
{
    if (!fileToLoad.existsAsFile())
        return;

    juce::String jsonString = fileToLoad.loadFileAsString();
    juce::var jsonVar = juce::JSON::parse(jsonString);

    if (!jsonVar.isArray())
        return;

    juce::Array<juce::var>* jsonArray = jsonVar.getArray();
    availableTracks.clear();

    for (auto& item : *jsonArray)
    {
        auto* fileObj = item.getDynamicObject();
        if (fileObj == nullptr)
            continue;

        juce::String filePath = fileObj->getProperty("filePath").toString();
        juce::File file{ filePath };

        if (!file.existsAsFile())
            continue;

        juce::var tracksVar = fileObj->getProperty("Tracks");
        if (!tracksVar.isArray())
            continue;

        juce::Array<juce::var>* trackArray = tracksVar.getArray();

        juce::FileInputStream inputStream(file);
        if (!inputStream.openedOk())
            continue;

        juce::MidiFile midiFile;
        if (!midiFile.readFrom(inputStream))
            continue;

        // Convert ticks to seconds for all tracks just once
        convertTicksToSeconds(midiFile);

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

            bool foundPercussion = false;
            for (int i = 0; i < sequence->getNumEvents(); ++i)
            {
                auto msg = sequence->getEventPointer(i)->message;
                if (msg.isNoteOnOrOff() && msg.getChannel() == 10)
                {
                    foundPercussion = true;
                    break;
                }
            }

            if (foundPercussion)
                tr.type = TrackType::Percussion;
            else tr.type = TrackType::Melodic;

            if (uuidString.isNotEmpty())
                tr.uuid = juce::Uuid(uuidString);
            else tr.uuid = TrackEntry::generateUUID();

            availableTracks.push_back(tr);
        }
    }

    listBox.updateContent();
    listBox.repaint();
}

std::vector<TrackEntry>& TrackListComponent::getAllAvailableTracks() const
{
    return this->availableTracks;
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

    for (auto& tr : availableTracks)
    {
        map[tr.getUniqueID()] = tr;
    }
    return map;
}

void MyTabbedComponent::currentTabChanged(int newCurrentTabIndex, const juce::String& newTabName)
{
    if (onTabChanged)
        onTabChanged(newCurrentTabIndex, newTabName);
}
