#include "displayGUI.h"
#include "CustomTableContainer.h"
#include "Arranger/ArrangerPatternBuilder.h"

void CurrentStyleComponent::startPlaying()
{
    int selectedID = playSettingsTracks.getSelectedId();
    if (selectedID == 3)
        selectedID = 1; //if the keybinds tab is selected, move it to the default one, (first one=play all tracks)

    playSettingsTracks.setSelectedId(selectedID);

    stopPlaying(false);

    std::vector<TrackEntry> selectedTracks;

    const bool hasMidiOut     = outputDevice.lock() != nullptr;
    const bool hasAudioInject = trackPlayer && bool(trackPlayer->onMidiMessage);

    if (!hasMidiOut && !hasAudioInject)
    {
        juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::WarningIcon, "Playing tracks", "No output device selected.");
        return;
    }

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

    if (arrangerModeEnabled)
    {
        // Phase 1: fixed 4/4; real time-signature wiring is Phase 2.
        ArrangerStyle style = ArrangerPatternBuilder::buildSingleSectionStyle(selectedTracks, 4, 4);
        arrangerEngine->setBpm(currentTempo);
        arrangerEngine->setStyle(style);
        arrangerEngine->start();
        return;
    }

    trackPlayer->setTracks(selectedTracks);
    trackPlayer->syncPlaybackSettings();
    trackPlayer->start();

}

CurrentStyleComponent::CurrentStyleComponent(const juce::String& name, std::unordered_map<juce::Uuid, TrackEntry*>& map, std::weak_ptr<juce::MidiOutput> outputDevice,
    std::weak_ptr<std::unordered_map<juce::String, std::unordered_map<juce::String,StyleSection>>> styleSMap) : name(name), mapUuidToTrackEntry(map), outputDevice(outputDevice), styleSectionsMap{styleSMap}
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

    arrangerEngine = std::make_unique<ArrangerEngine>(outputDevice);
    arrangerEngine->onElapsedBeats = [this](double elapsedBeats)
    {
        customBeatBar.setCurrentBeatsElapsed(elapsedBeats);
    };

    startPlayingTracks.onClick = [this]
    {
        isPlaying = true;
        startPlaying();
    };

    stopPlayingTracks.onClick = [this]
    {
        isPlaying = false;
        stopPlaying();
    };

    customBeatBar.isPlayingCheck = [this]()
    {
        return isPlaying;
    };

    trackPlayer->onStopTriggerClickFromPlayer = [this]()
    {
        juce::MessageManager::callAsync([this]()
            {
                isPlaying = false;
                customBeatBar.repaint();
            });
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

        applyBPMchangeBeforePlayback(currentTempo,true);

        if (anyTrackChanged)
            anyTrackChanged();
    };

    addAndMakeVisible(tempoSlider);

    trackPlayer->addSubjectTrackPlayerModifyListener(this);
}

void CurrentStyleComponent::stopPlaying(bool shouldModify)
{
    if (arrangerModeEnabled)
    {
        if (arrangerEngine)
            arrangerEngine->stop();
        return;
    }

    if (trackPlayer)
        trackPlayer->stop(shouldModify);
}

void CurrentStyleComponent::setArrangerModeEnabled(bool shouldEnable)
{
    if (arrangerModeEnabled == shouldEnable)
        return;

    stopPlaying(false);          // stop whichever engine is currently active
    if (arrangerEngine)
        arrangerEngine->stop();

    arrangerModeEnabled = shouldEnable;
    isPlaying = false;
    customBeatBar.repaint();
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
    if (arrangerEngine)
        arrangerEngine->setDeviceOutput(newOutput);
}

void CurrentStyleComponent::setMidiInjectCallback(std::function<void(const juce::MidiMessage&)> cb)
{
    if (trackPlayer)
        trackPlayer->onMidiMessage = cb;
    if (arrangerEngine)
        arrangerEngine->onMidiMessage = cb;
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
    double originalBPMfromFile = 120.0;

    auto it = mapUuidToTrackEntry.find(uuid);
    if (it != mapUuidToTrackEntry.end())
    {
        auto& track = it->second;
        if (track != nullptr)
        {
            sequence = &track->sequence;
            changeMap = &track->styleChangesMap[styleID];
            originalBPMfromFile = track->originalBPM;

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
        changeMap ? *changeMap : emptyMap,
        isPlaying
        );

    trackPlayer->addSubjectTrackPlayerModifyListener(container.get());

    container->setSize(350, 340);

    container->updateToFile = [this]()
    {
        if (updateTrackFile)
            updateTrackFile();
    };

    container->addModelAsListenerToTrackPlayer(trackPlayer.get());
    container->removeModelFromListener = [this](TrackPlayerListener* listener)
    {
        if (trackPlayer)
            trackPlayer->removeSubjectTrackPlayerListener(listener);
    };

    container->removeContainerFromListeners = [this, container=container.get()]()
    {
        trackPlayer->removeSubjectTrackPlayerModifyListener(container);
    };

    container->getCurrentBPMstyle = [this]()
    {
        return this->currentTempo;
    };

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

std::vector<CurrentStyleComponent::TrackChannelInstrument> CurrentStyleComponent::getTrackChannelInstruments() const
{
    std::vector<TrackChannelInstrument> result;
    for (const auto* track : allTracks)
        result.push_back({ track->getChannel(), track->getInstrumentNumber() });
    return result;
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

void CurrentStyleComponent::applyBPMchangeBeforePlayback(double userBPM, bool applyStyleChanges)
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


        tr->sequence = scaledSequence;
        if (applyStyleChanges)
        {
            applyChangesForOneTrack(*tr);
        }

        auto& toApply = tr->sequence;

        double firstNoteOnTime = 0.01;
        for (int i = 0; i < toApply.getNumEvents(); ++i)
        {
            const auto& msg = toApply.getEventPointer(i)->message;
            if (msg.isNoteOn())
            {
                firstNoteOnTime = std::max(0.01, msg.getTimeStamp() - 0.005);
                break;
            }
        }

        if (tr->volumeAssociated != -1)
        {
            toApply.addEvent(
                juce::MidiMessage::controllerEvent(targetChannel, 7, (int)tr->volumeAssociated)
                .withTimeStamp(firstNoteOnTime - 0.004));
        }

        if (targetChannel != 10 && tr->instrumentAssociated != -1)
        {
            toApply.addEvent(
                juce::MidiMessage::programChange(targetChannel, tr->instrumentAssociated)
                .withTimeStamp(firstNoteOnTime - 0.002));
        }
    }
}

//if this function is used, there's an issue with the sync of the whole tracks
void CurrentStyleComponent::applyBPMchangeForOne(double userBPM, const juce::Uuid& uuid)
{

    if (userBPM <= 0.0)
        userBPM = 120.0;

    currentTempo = userBPM;

    if (trackPlayer)
        trackPlayer->setCurrentBPM(currentTempo);

    auto it = mapUuidToTrackEntry.find(uuid);
    if (it == mapUuidToTrackEntry.end() || it->second == nullptr)
        return;

    auto& tr = it->second;


    int channelCounter = 0;
    int targetChannel = (tr->type == TrackType::Percussion) ? 10 : 2;
    for (const auto& trackPtr : allTracks)
    {
        if (trackPtr->getUsedID() == uuid)
            break;

        auto& other = mapUuidToTrackEntry[trackPtr->getUsedID()];
        if (other && other->type != TrackType::Percussion)
            ++channelCounter;
    }
    if (tr->type != TrackType::Percussion)
        targetChannel = channelCounter + 2;

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

bool CurrentStyleComponent::getIsPlaying()
{
    return isPlaying;
}

void CurrentStyleComponent::handleIntroCurrentStyle(const juce::String& name, const std::unordered_map<juce::String, StyleSection>& section)
{
    if (trackPlayer)
        trackPlayer->setLastSectionUsed(section.at(name));
}

void CurrentStyleComponent::handleEndingCurrentStyle(const juce::String& name, const std::unordered_map<juce::String, StyleSection>& section)
{
    if (trackPlayer)
        trackPlayer->setLastSectionUsed(section.at(name));
}

void CurrentStyleComponent::handleVarCurrentStyle(const juce::String& name, const std::unordered_map<juce::String, StyleSection>& section)
{
    if (trackPlayer)
        trackPlayer->setLastSectionUsed(section.at(name));
}

void CurrentStyleComponent::handleFillCurrentStyle(const juce::String& name, const std::unordered_map<juce::String, StyleSection>& section)
{
    if (trackPlayer)
        trackPlayer->setLastSectionUsed(section.at(name));
}

void CurrentStyleComponent::handleBreakCurrentStyle(const juce::String& name, const std::unordered_map<juce::String, StyleSection>& section)
{
    if (trackPlayer)
        trackPlayer->setLastSectionUsed(section.at(name));
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

void CurrentStyleComponent::updateObjects()
{
    tempoSlider.setEnabled(!tempoSlider.isEnabled());
}

CurrentStyleComponent::~CurrentStyleComponent()
{
    if (trackPlayer)
    {
        for (auto& track : allTracks)
            track->removeListener(trackPlayer.get());

        trackPlayer->removeSubjectTrackPlayerModifyListener(this);
    }
    if (&playSettingsTracks)
        playSettingsTracks.removeListener(this);
}

juce::String CurrentStyleComponent::getName()
{
    return name;
}

juce::String CurrentStyleComponent::getStyleID()
{
    return this->styleID;
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
