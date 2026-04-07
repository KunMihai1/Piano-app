#include "MainComponent.h"

//==============================================================================
MainComponent::MainComponent()
{
    settingsInit();
    playButtonInit();
    headerPanelInit();
    togglePlayButton();
    initalizeSaveFileForUser();
    cachedImageMainWindow = juce::ImageFileFormat::loadFrom(BinaryData::MainWindow_png, BinaryData::MainWindow_pngSize);
    playBackground = juce::ImageFileFormat::loadFrom(BinaryData::playingBackground_png, BinaryData::playingBackground_pngSize);
    currentBackground = cachedImageMainWindow;

    buildChordLibrary();

    displayInit();

    setBounds(0, 0, juce::Desktop::getInstance().getDisplays().getPrimaryDisplay()->totalArea.getWidth(),
        juce::Desktop::getInstance().getDisplays().getPrimaryDisplay()->totalArea.getHeight() - 40);
    this->addKeyListener(&this->keyListener);
    addKeyListener(this);

    client = std::make_shared<SupabaseClient>();

    loginWindowInitialize();

    midiHandler.addListener(&recordPlayer);
    midiHandler.addListener(&keyboard);
    
    display->addListener(&midiHandler);
    this->display->callingListeners();

    MIDIDevice.getAvailableDevicesMidiIN(this->devicesIN);
    MIDIDevice.getAvailableDevicesMidiOUT(this->devicesOUT);
    this->toFront(true);

    if(settingsPanel.isVisible())
        populateUpdateComboBoxDevices();

    midiHandler.onStartNoteSetting = [this]()
    {
        if (display)
            display->startingPlayer();
    };

    midiHandler.onEndNoteSetting = [this]()
    {
        if (display)
        {
            display->stoppingPlayer();
        }
    };

    juce::Desktop::getInstance().addFocusChangeListener(this);

    juce::MessageManager::callAsync([this]()
        {
            if (isVisible())
                grabKeyboardFocus();
        });

    loadSettings();
    startTimer(1000);

}

MainComponent::~MainComponent()
{
    if (this->MIDIDevice.isOpenIN())
        this->MIDIDevice.deviceCloseIN();
    if (this->MIDIDevice.isOpenOUT())
    {
        this->display->stoppingPlayer();
        this->MIDIDevice.deviceCloseOUT();
    }
    if (treeView != nullptr && treeView->getRootItem() != nullptr)
    {
        //DBG("Deleting root item dest main: " << treeView->getRootItem()->getUniqueName());
        treeView->deleteRootItem();
        treeView->deleteRootItem();
    }
    else;
        //DBG("No root item to delete destructor main");
    removeKeyListener(this);
    if(&keyListener)
        removeKeyListener(&this->keyListener);
    if(&recordPlayer)
        midiHandler.removeListener(&recordPlayer);
    if(noteLayer)
        midiHandler.removeListener(noteLayer.get());
    if(&keyboard)
        midiHandler.removeListener(&keyboard);
    if (display)
        display->removeListener(&midiHandler);

    volumeKnob.setLookAndFeel(nullptr);  

}

//==============================================================================
void MainComponent::paint(juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
    if (currentBackground.isValid())
    {
            juce::Rectangle<float> bounds = getLocalBounds().toFloat();
            auto imageWidth = (float)cachedImageMainWindow.getWidth();
            auto imageHeight = (float)cachedImageMainWindow.getHeight();
            auto scale = juce::jmax(bounds.getWidth() / imageWidth, bounds.getHeight() / imageHeight);
            auto newWidth = imageWidth * scale;
            auto newHeight = imageHeight * scale;
            auto x = (bounds.getWidth() - newWidth) * 0.5f;
            auto y = (bounds.getHeight() - newHeight) * 0.5f;
            g.setColour(juce::Colours::grey);
            g.setImageResamplingQuality(juce::Graphics::highResamplingQuality);
            g.drawImage(currentBackground, juce::Rectangle<float>(x, y, newWidth, newHeight));
        g.setFont(juce::FontOptions(16.0f));
        g.setColour(juce::Colours::white);
    }
}

void MainComponent::focusGained(juce::Component::FocusChangeType)
{

    if (noteLayer)
    {
        noteLayer->toFront(false);
    }
}

void MainComponent::globalFocusChanged(juce::Component* focusedComponent)
{
    if (focusedComponent == nullptr)
    {
        // Focus left JUCE — but on Windows this also fires briefly during
        // within-app window switches.  Defer the hide so we don't flicker.
        juce::Timer::callAfterDelay(150,
            [safeThis = juce::Component::SafePointer<MainComponent>(this)]()
            {
                if (safeThis == nullptr) return;

                // After the delay, check if the app truly lost foreground.
                if (!juce::Process::isForegroundProcess())
                {
                    if (safeThis->overlayWindow)
                        safeThis->overlayWindow->setVisible(false);

                    if (safeThis->midiWindow)
                        safeThis->midiWindow->setVisible(false);

                    if (safeThis->soundEffectWindow)
                        safeThis->soundEffectWindow->setVisible(false);
                }
            });
    }
    else
    {
        // Focus returned to the app — only restore windows that were
        // actually hidden (e.g. after alt-tab away). During normal
        // within-app focus transitions nothing was hidden, so this
        // branch does nothing, avoiding any native-window side effects.
        if (overlayWindow && !overlayWindow->isVisible() && overlayShouldBeVisible)
            overlayWindow->setVisible(true);

        if (midiWindow && !midiWindow->isVisible() && midiWindowShouldBeVisible)
            midiWindow->setVisible(true);

        if (soundEffectWindow && !soundEffectWindow->isVisible() && soundEffecttWindowShouldBeVisible)
            soundEffectWindow->setVisible(true);
    }
}

void MainComponent::resized()
{
    // This is called when the MainComponent is resized.
    // If you add any child components, this is where you should
    // update their positions.
    int buttonWidth = getWidth() / 16;
    int buttonHeight = getHeight() / 24;
    int x = (int)(getWidth() * 0.5 - getWidth() * 0.175);
    int y = (int)(getHeight() * 0.5 + getHeight() * 0.162);

    playButton.setBounds(x, y, buttonWidth, buttonHeight);
    settingsButton.setBounds(0, 0, 250, 70);
    settingsPanel.setBounds(0, 70, 250, getHeight());
    midiButton.setBounds(0, 5, 200, 50);
    updateNumberOfKeysDevice.setBounds(0, midiButton.getBottom() + 10, 200, 50);
    devicesCBUpdate.setBounds(0, updateNumberOfKeysDevice.getBottom() + 5, 200, 25);

    helpIcon.setBounds(210, 20, 100, 20);

    headerPanel.setBounds(0, 0, getWidth(), 100);
    homeButton.setBounds(10, 10, 75, 30);
    colourSelectorButton.setBounds(90, 10, 100, 30);
    instrumentSelectorButton.setBounds(195, 10, 100, 30);
    particleToggle.setBounds(colourSelectorButton.getX(), colourSelectorButton.getY() +colourSelectorButton.getHeight(), 100, 25);
    noteNumbersAnnotation.setBounds(particleToggle.getX(), particleToggle.getBottom(), 100, 25);

    leftHandInstrumentToggle.setBounds(instrumentSelectorButton.getX(), instrumentSelectorButton.getY() + instrumentSelectorButton.getHeight(), 100, 25);
    rightHandInstrumentToggle.setBounds(instrumentSelectorButton.getX(), leftHandInstrumentToggle.getY() + leftHandInstrumentToggle.getHeight(), 100, 25);

    startPlayback.setBounds(getWidth() - 30 - 35, 10, 30, 30);
    stopRecording.setBounds(startPlayback.getX()-30-10, 10, 30, 30);
    startRecording.setBounds(stopRecording.getX()-30-10, 10, 30, 30);

    chordHelperButton.setBounds(getLocalBounds().getRight()-205, startRecording.getBottom()+5, 200, 50);

    saveRecordingButton.setBounds(startRecording.getX()-140-10, 10, 140, 30);
    playRecordingFileButton.setBounds(305, 10, 100, 30);



    //volumeKnob.setBounds(getWidth() / 2, 0, 70, 50);

    display->setBounds((headerPanel.getWidth()-400)/2, 0, 400, 100);

    if (noteLayer)
    {
        noteLayer->toFront(false);
        noteLayer->setAlwaysOnTop(true);
        noteLayer->setOpaque(true);
        keyboard.toFront(false);
        keyboard.setAlwaysOnTop(true);
        keyboard.setOpaque(true);
    }   

    if (introsEndings)
    {
        int width = display->getX() - playRecordingFileButton.getX() - playRecordingFileButton.getWidth() - 10;
        int height = playRecordingFileButton.getHeight() + 20;
        introsEndings->setBounds(playRecordingFileButton.getX() + playRecordingFileButton.getWidth() + 10, playRecordingFileButton.getY(), width, height);
    }

    if (variationsFills)
    {
        int varFillsX = playRecordingFileButton.getX();
        int varFillsY = introsEndings->getY() + introsEndings->getHeight()-7;
        int width = display->getX() - rightHandInstrumentToggle.getX() - rightHandInstrumentToggle.getWidth() - 10;
        int height = display->getHeight() - (introsEndings->getY() + introsEndings->getHeight());
        variationsFills->setBounds(varFillsX, varFillsY, width, height+10);
    }
}

void MainComponent::changeListenerCallback(juce::ChangeBroadcaster* source)
{
    if (source == colourSelector)
    {
        auto newColor = colourSelector->getCurrentColour();
        //DBG("Colour changed to: " << newColor.toString());
        this->noteLayer->setColourParticle(newColor);
    }
}

bool MainComponent::keyPressed(const juce::KeyPress& key, juce::Component*)
{

    if (key == juce::KeyPress::escapeKey)
    {

        if (!overlayWindow)
        {
            overlayWindow = std::make_unique<OverlayComponent>();
            auto topLevel = getTopLevelComponent();
            juce::Rectangle<int> screenBounds = topLevel->getScreenBounds();

            overlayWindow->setBounds(screenBounds);
            overlayWindow->setAlwaysOnTop(true);
            overlayWindow->setOpaque(false);

            overlayShouldBeVisible = true;
            overlayWindow->setVisible(true);


            overlayWindow->addToDesktop(
                juce::ComponentPeer::windowIsTemporary
                | juce::ComponentPeer::windowHasDropShadow,
                nullptr);

            overlayWindow->toFront(true);


            setCallBacksForOverlayWindow();

            overlayWindow->grabKeyboardFocus();
        }
        else
        {

            if (overlayWindow->onRequestClose)
                overlayWindow->onRequestClose();
            else
            {
                overlayShouldBeVisible = false;
                overlayWindow->removeFromDesktop();
                overlayWindow.reset();
            }
        }
    }

    if (!this->keyListener.getIsKeyboardInput())
        return false;

    int startNote = this->keyListener.getStartNoteKeyboardInput();
    int finishNote = this->keyListener.getFinishNoteKeyboardInput();
    //DBG("Start note:" << startNote);
    //DBG("End note:" << finishNote);

    if (key.getKeyCode() == 'Z')
    {
        if (startNote >= 24)
        {
            this->keyListener.setStartNoteKeyboardInput(startNote - 12);
            this->keyListener.setFinishNoteKeyboardInput(finishNote - 12);
            this->keyboard.set_min_and_max(keyListener.getStartNoteKeyboardInput(), keyListener.getFinishNoteKeyboardInput());
            
            this->midiHandler.allOffKeyboard();
            this->noteLayer->resetStateActiveNotes();
            this->keyListener.resetState();
            this->keyboard.setIsDrawn(false);
            this->keyboard.resetStateActiveNotes();
            this->display->set_min_max(startNote - 12, finishNote - 12);
            this->display->setNewSettingsHelperFunction(-12);
        }
    }
    else if (key.getKeyCode() == 'X')
    {
        //DBG("Finish note" << finishNote);
        if (finishNote <= 101)
        {
            this->keyListener.setStartNoteKeyboardInput(startNote + 12);
            this->keyListener.setFinishNoteKeyboardInput(finishNote + 12);
            this->keyboard.set_min_and_max(keyListener.getStartNoteKeyboardInput(), keyListener.getFinishNoteKeyboardInput());

            this->midiHandler.allOffKeyboard();
            this->noteLayer->resetStateActiveNotes();
            this->keyListener.resetState();
            this->keyboard.setIsDrawn(false);
            this->keyboard.resetStateActiveNotes();
            this->display->set_min_max(startNote + 12, finishNote + 12);
            this->display->setNewSettingsHelperFunction(12);
        }
    }
    return false;
}

void MainComponent::timerCallback()
{
    if(this->keyListener.getIsKeyboardInput()==false)
        checkMidiInputDeviceValid();

    if (settingsPanel.isVisible())
        populateUpdateComboBoxDevices();
}

void MainComponent::checkMidiInputDeviceValid()
{
    auto devices = juce::MidiInput::getAvailableDevices();
    bool deviceStillPresent = false;
    for (auto& device : devices)
    {
        if (device.identifier == MIDIDevice.get_identifier())
        {
            deviceStillPresent = true;
            break;
        }
    }

    if (!deviceStillPresent)
    {
        if (noteLayer)
            noteLayer->resetStateActiveNotes();

        keyboard.resetStateActiveNotes();
            

        this->MIDIDevice.deviceCloseIN();
        this->MIDIDevice.deviceCloseOUT();
    }
}

void MainComponent::comboBoxChanged(juce::ComboBox* comboBoxThatHasChanged)
{
    if (comboBoxThatHasChanged == &devicesCBUpdate)
    {

    }
}

void MainComponent::populateUpdateComboBoxDevices()
{
    bool ok = false;
    if (last == -1 || last != MIDIDevice.getNrInputActualDevices())
    {
        lastOfLast = last;
        last = MIDIDevice.getNrInputActualDevices();
        ok = true;
    }

    if (!ok)
        return;
    devicesCBUpdate.clear();
    int i = 1;
    for (auto& dev : this->MIDIDevice.getAvailableInputDevicesNameIdentifier())
    {
        juce::String name = dev.first;
        juce::String identifier = dev.second;
        juce::String PID = MIDIDevice.extractPID(identifier);
        juce::String VID = MIDIDevice.extractVID(identifier);
        if (dataBase.deviceExists(VID, PID)==false)
            continue;

        if (i == 1)
            updateDevicesMap.clear(); //map needs to be here because if we are here that means there has been at least a change in the devices, otherwise we clear the map each time


        updateDevicesMap[name] = identifier;

        this->devicesCBUpdate.addItem(name, i++);
    }
    if(lastOfLast!=-1)
    {
        devicesCBUpdate.hidePopup();
        devicesCBUpdate.showPopup();
    }
    devicesCBUpdate.setSelectedId(1);
}

void MainComponent::initalizeSaveFileForUser()
{
    juce::PropertiesFile::Options options;
    options.applicationName= "Piano Synth2";
    options.filenameSuffix = "settings";
    options.folderName="Piano Synth2";
    options.osxLibrarySubFolder = "Application Support";
    options.commonToAllUsers = false;

    appProperties.setStorageParameters(options);
    propertiesFile = appProperties.getUserSettings();
}

void MainComponent::loginWindowInitialize()
{
    playButton.setVisible(false);
    loginWindow = std::make_unique<LoginComponent>(client);
    addAndMakeVisible(loginWindow.get());

    auto parentBounds = getLocalBounds();
    int panelWidth = 400;
    int panelHeight = 400;

    int x = (parentBounds.getWidth() - panelWidth) / 2;
    int y = (parentBounds.getHeight() - panelHeight) / 2;

    loginWindow->setBounds(x, y, panelWidth, panelHeight);

    loginWindow->onSuccessfullLogin = [this]()
    {
        /*
        playtimeTracker = std::make_unique<PlaytimeTracker>([this](int secondsToIncreaseWith)
            {
                auto clientPtr = client; 
                auto VID = MIDIDevice.getVID();
                auto PID = MIDIDevice.getPID();

                std::thread([clientPtr,VID,PID,secondsToIncreaseWith]() {
                    clientPtr->incrementPlaytime(secondsToIncreaseWith,VID,PID);
                    }).detach();
            },300);
        */
        playButton.setVisible(true);
        loginWindow->setVisible(false);
       
        juce::MessageManager::callAsync([this]() {
            loginWindow.reset();
            });
        
    };

}

void MainComponent::loadEffectsFromFile()
{
    auto load = [&](const juce::String& name, auto setter)
    {
        int fallbackValue=64;
        int first = propertiesFile->getIntValue(name + "First", fallbackValue);
        int second = propertiesFile->getIntValue(name + "Second", fallbackValue);

        setter(first, 1);   // Channel 1
        setter(second, 16); // Channel 16
    };

    // Line 1
    load("midiBrightness", [&](int v, int ch) { MIDIDevice.setBrightness(v, ch); });
    load("midiExpression", [&](int v, int ch) { MIDIDevice.setExpression(v, ch); });
    load("midiChorus", [&](int v, int ch) { MIDIDevice.setChorus(v, ch); });
    load("midiResonance", [&](int v, int ch) { MIDIDevice.setResonance(v, ch); });

    // Line 2
    load("midiAttack", [&](int v, int ch) { MIDIDevice.setAttack(v, ch); });
    load("midiDecay", [&](int v, int ch) { MIDIDevice.setDecay(v, ch); });
    load("midiRelease", [&](int v, int ch) { MIDIDevice.setRelease(v, ch); });
    load("midiVibrato", [&](int v, int ch) { MIDIDevice.setVibrato(v, ch); });

    // Line 3
    load("midiDelay", [&](int v, int ch) { MIDIDevice.setDelay(v, ch); });
    load("midiPan", [&](int v, int ch) { MIDIDevice.setPan(v, ch); });
    load("midiReverb", [&](int v, int ch) { MIDIDevice.setReverb(v, ch); });
    load("midiVolume", [&](int v, int ch) { MIDIDevice.setVolume(v, ch); });

    // Line 4
    load("midiDistortion", [&](int v, int ch) { MIDIDevice.setDistortion(v, ch); });
    load("midiFilterTrack", [&](int v, int ch) { MIDIDevice.setFilterTrack(v, ch); });
    load("midiTremolo", [&](int v, int ch) { MIDIDevice.setTremolo(v, ch); });
    load("midiRandomMod", [&](int v, int ch) { MIDIDevice.setRandomMod(v, ch); });
}

void MainComponent::loadSettings()
{
    if (propertiesFile)
    {

        loadEffectsFromFile();

        int leftHandInstrumentNumber = propertiesFile->getIntValue("leftInstrumentNumber", 0);
        int rightHandInstrumentNumber = propertiesFile->getIntValue("rightInstrumentNumber", 0);


        this->midiHandler.setProgramNumber(leftHandInstrumentNumber, "left");
        this->midiHandler.setProgramNumber(rightHandInstrumentNumber, "right");
    }
}

void MainComponent::playChordOnClick(const Chord& c)
{
    auto notes = ChordHelper::getNotesForChord(c);

    for (auto note : notes)
    {

        juce::MidiMessage noteOn = juce::MidiMessage::noteOn(13, note, (juce::uint8)100); //Channels: 14,15 taken by record player  || 1,16 user playing || 2->7 and 10 tracks (some can be reused but it's 100% safe like this if we don't run out of channels
        midiHandler.handleIncomingMidiMessage(nullptr, noteOn);


        juce::Timer::callAfterDelay(500, [this, note]() {
            juce::MidiMessage noteOff = juce::MidiMessage::noteOff(13, note);
            midiHandler.handleIncomingMidiMessage(nullptr, noteOff);
            });
    }
}

void MainComponent::buildChordLibrary()
{
    myChordLibrary.clear();

    std::vector<juce::String> roots =
    {
        "C","C#","D","D#","E",
        "F","F#","G","G#","A","A#","B"
    };

    std::vector<juce::String> types =
    {
        "Major",
        "Minor",
        "7",
        "Augmented",
        "Diminished"
    };

    for (const auto& root : roots)
    {
        for (const auto& type : types)
        {
            Chord chord;

            if (type == "7")
                chord.name = root + "7";
            else
                chord.name = root + " " + type;

            myChordLibrary.push_back(chord);
        }
    }

}

void MainComponent::setCallBacksForEffectWindow()
{
    std::function<void(int, int)> saveCallback = [this](int ccNumber, int value)
    {
        int currentChannel = this->soundEffectWindow->getContent().getSelectedChannel();

        juce::String key;

        switch (ccNumber)
        {
        // Line 1 – Tone
        case 74: key = "midiBrightness"; MIDIDevice.setBrightness(value, currentChannel); break;
        case 11: key = "midiExpression"; MIDIDevice.setExpression(value, currentChannel); break;
        case 93: key = "midiChorus"; MIDIDevice.setChorus(value, currentChannel); break;
        case 71: key = "midiResonance"; MIDIDevice.setResonance(value, currentChannel); break;
        case 64: key = "midiSustain"; MIDIDevice.setSustainToggle(value, currentChannel); break;

        // Line 2 – Envelope
        case 1:key = "midiVibrato"; MIDIDevice.setVibrato(value, currentChannel); break;
        case 73: key = "midiAttack"; MIDIDevice.setAttack(value, currentChannel); break;
        case 75: key = "midiDecay"; MIDIDevice.setDecay(value, currentChannel); break;
        case 72: key = "midiRelease"; MIDIDevice.setRelease(value, currentChannel); break;

        // Line 3 – Space
        case 7: key = "midiVolume"; MIDIDevice.setVolume(value, currentChannel);  break;
        case 91:key = "midiReverb"; MIDIDevice.setReverb(value, currentChannel);  break;
        case 94: key = "midiDelay"; MIDIDevice.setDelay(value, currentChannel); break;
        case 10: key = "midiPan"; MIDIDevice.setPan(value, currentChannel); break;

        // Line 4 – Modulation
        case 80: key = "midiDistortion"; MIDIDevice.setDistortion(value, currentChannel); break;
        case 76: key = "midiFilterTrack"; MIDIDevice.setFilterTrack(value, currentChannel); break;
        case 92: key = "midiTremolo"; MIDIDevice.setTremolo(value, currentChannel); break;
        case 95: key = "midiRandomMod"; MIDIDevice.setRandomMod(value, currentChannel); break;

        default: return;
        }

        if (currentChannel == 1)       key += "First";
        else if (currentChannel == 16) key += "Second";

        if (!playButton.isVisible())
        {
            MIDIDevice.sendMidiCC(currentChannel, ccNumber, value);
        }

        propertiesFile->setValue(key, value);
        propertiesFile->saveIfNeeded();
    };


    auto& content = this->soundEffectWindow->getContent();

    // Line 1 – Tone
    content.getBrightnessKnob().onValueChanged = saveCallback;
    content.getExpressionKnob().onValueChanged = saveCallback;
    content.getChorusKnob().onValueChanged = saveCallback;
    content.getResonanceKnob().onValueChanged = saveCallback;
    content.getSustainToggle().onValueChanged = saveCallback;

    // Line 2 – Envelope
    content.getAttackKnob().onValueChanged   = saveCallback;
    content.getDecayKnob().onValueChanged     = saveCallback;
    content.getReleaseKnob().onValueChanged   = saveCallback;
    content.getVibratoKnob().onValueChanged = saveCallback;

    // Line 3 – Space
    content.getVolumeKnob().onValueChanged = saveCallback;
    content.getReverbKnob().onValueChanged = saveCallback;
    content.getDelayKnob().onValueChanged     = saveCallback;
    content.getPanKnob().onValueChanged       = saveCallback;

    // Line 4 – Modulation
    content.getDistortionKnob().onValueChanged  = saveCallback;
    content.getFilterTrackKnob().onValueChanged = saveCallback;
    content.getTremoloKnob().onValueChanged     = saveCallback;
    content.getRandomModKnob().onValueChanged   = saveCallback;

    content.onChannelChanged = [this](int channel)
    {
        auto& content = this->soundEffectWindow->getContent();

        // Line 1 – Tone
        content.getBrightnessKnob().setValue(MIDIDevice.getBrightness(channel));
        content.getExpressionKnob().setValue(MIDIDevice.getExpression(channel));
        content.getChorusKnob().setValue(MIDIDevice.getChorus(channel));
        content.getResonanceKnob().setValue(MIDIDevice.getResonance(channel));
        content.getSustainToggle().setOn(MIDIDevice.getSustainToggle(channel));

        // Line 2 – Envelope
        content.getAttackKnob().setValue(MIDIDevice.getAttack(channel));
        content.getDecayKnob().setValue(MIDIDevice.getDecay(channel));
        content.getReleaseKnob().setValue(MIDIDevice.getRelease(channel));
        content.getVibratoKnob().setValue(MIDIDevice.getVibrato(channel));

        // Line 3 – Space
        content.getVolumeKnob().setValue(MIDIDevice.getVolume(channel));
        content.getReverbKnob().setValue(MIDIDevice.getReverb(channel));
        content.getDelayKnob().setValue(MIDIDevice.getDelay(channel));
        content.getPanKnob().setValue(MIDIDevice.getPan(channel));

        // Line 4 – Modulation
        content.getDistortionKnob().setValue(MIDIDevice.getDistortion(channel));
        content.getFilterTrackKnob().setValue(MIDIDevice.getFilterTrack(channel));
        content.getTremoloKnob().setValue(MIDIDevice.getTremolo(channel));
        content.getRandomModKnob().setValue(MIDIDevice.getRandomMod(channel));
    };
}

void MainComponent::setInitialValuesFromSettingsFileEffectWindow()
{
    auto& content = soundEffectWindow->getContent();

    content.getBrightnessKnob().setValue(MIDIDevice.getBrightness(1));
    content.getChorusKnob().setValue(MIDIDevice.getChorus(1));
    content.getExpressionKnob().setValue(MIDIDevice.getExpression(1));
    content.getResonanceKnob().setValue(MIDIDevice.getResonance(1));
    content.getSustainToggle().setOn(MIDIDevice.getSustainToggle(1));

    // Line 2 - Envelope & Modulation
    content.getAttackKnob().setValue(MIDIDevice.getAttack(1));
    content.getDecayKnob().setValue(MIDIDevice.getDecay(1));
    content.getReleaseKnob().setValue(MIDIDevice.getRelease(1));
    content.getVibratoKnob().setValue(MIDIDevice.getVibrato(1));

    // Line 3 - Effects & Pan
    content.getVolumeKnob().setValue(MIDIDevice.getVolume(1));
    content.getReverbKnob().setValue(MIDIDevice.getReverb(1));
    content.getDelayKnob().setValue(MIDIDevice.getDelay(1));
    content.getPanKnob().setValue(MIDIDevice.getPan(1));

    // Line 4 - Additional Effects
    content.getDistortionKnob().setValue(MIDIDevice.getDistortion(1));
    content.getFilterTrackKnob().setValue(MIDIDevice.getFilterTrack(1));
    content.getTremoloKnob().setValue(MIDIDevice.getTremolo(1));
    content.getRandomModKnob().setValue(MIDIDevice.getRandomMod(1));
}

void MainComponent::sendEffectsBeforePlaying()
{
    struct EffectCC
    {
        int ccNumber;
        std::function<int(int)> getter; // int parameter = channel
    };

    std::vector<EffectCC> effects = {
    { 74, [&](int ch) { return MIDIDevice.getBrightness(ch); } },
    { 11, [&](int ch) { return MIDIDevice.getExpression(ch); } },
    { 93, [&](int ch) { return MIDIDevice.getChorus(ch); } },
    { 71, [&](int ch) { return MIDIDevice.getResonance(ch); } },
    { 64, [&](int ch) { return MIDIDevice.getSustainToggle(ch) ? 127 : 0; } },

    { 73, [&](int ch) { return MIDIDevice.getAttack(ch); } },
    { 75, [&](int ch) { return MIDIDevice.getDecay(ch); } },
    { 72, [&](int ch) { return MIDIDevice.getRelease(ch); } },
    { 1,  [&](int ch) { return MIDIDevice.getVibrato(ch); } },

    { 94, [&](int ch) { return MIDIDevice.getDelay(ch); } },
    { 10, [&](int ch) { return MIDIDevice.getPan(ch); } },
    { 91, [&](int ch) { return MIDIDevice.getReverb(ch); } },
    { 7,  [&](int ch) { return MIDIDevice.getVolume(ch); } },

    { 80, [&](int ch) { return MIDIDevice.getDistortion(ch); } },
    { 76, [&](int ch) { return MIDIDevice.getFilterTrack(ch); } },
    { 92, [&](int ch) { return MIDIDevice.getTremolo(ch); } },
    { 95, [&](int ch) { return MIDIDevice.getRandomMod(ch); } }
    };

    std::vector<int> channels = { 1, 16 };

    for (int ch : channels)
    {
        for (auto& effect : effects)
        {
            MIDIDevice.sendMidiCC(ch, effect.ccNumber, effect.getter(ch));
        }
    }

}

void MainComponent::setCallBacksForOverlayWindow()
{
    overlayWindow->onRequestClose = [this]()
    {
        if (overlayWindow)
        {
            overlayShouldBeVisible = false;
            overlayWindow->removeFromDesktop();
            overlayWindow.reset();
        }
    };

    overlayWindow->setWindowFlag = [this]()
    {
        overlayShouldBeVisible = false;
    };

    overlayWindow->bringSeparateWindowFront = [this]()
    {
        // Bring the child windows above the overlay without stealing
        // focus.  toFront(true) + grabKeyboardFocus from inside the
        // overlay's mouseDown causes a focus fight: after the handler
        // returns the OS restores activation to the overlay (the
        // clicked native window), leaving both DocumentWindows grey.
        if (midiWindow)
            midiWindow->toFront(false);

        if (soundEffectWindow)
            soundEffectWindow->toFront(false);

    };

    overlayWindow->onSettingsClick = [this]()
    {

        midiWindowShouldBeVisible = true;

        if (!midiWindow)
        {
            if (soundEffectWindow)
                soundEffectWindow->closeButtonPressed();

            midiWindow = std::make_unique<MIDIWindow>(
                this->MIDIDevice, devicesIN, devicesOUT, propertiesFile);


            midiWindow->setAlwaysOnTop(true);

            midiWindow->setVisible(true);

            midiWindow->onWindowClosed = [this]()
            {
                midiWindowShouldBeVisible = false;
                midiWindow.reset();
            };

            midiWindow->isMidiDeviceOpen = [this]()
            {
                return !playButton.isVisible();
            };

            midiWindow->volumeSliderSetValue(MIDIDevice.getVolume(1));
            midiWindow->reverbSliderSetValue(MIDIDevice.getReverb(1));


        }
        else
        {
            midiWindow->setVisible(true);
            midiWindow->toFront(true);
        }

        // Grab focus asynchronously so the overlay's button-click event
        // finishes first and the OS doesn't steal activation back.
        juce::MessageManager::callAsync([this]()
        {
            if (midiWindow)
            {
                midiWindow->toFront(false);
                midiWindow->grabKeyboardFocus();
            }
                
        });
    };

    overlayWindow->onEffectsClick = [this]()
    {
        soundEffecttWindowShouldBeVisible = true;

        if (!soundEffectWindow)
        {
            if (midiWindow)
                midiWindow->closeButtonPressed();

            soundEffectWindow = std::make_unique<SoundEffectWindow>();

            soundEffectWindow->setAlwaysOnTop(true);
            soundEffectWindow->setVisible(true);

            soundEffectWindow->onWindowClosed = [this]()
            {
                soundEffecttWindowShouldBeVisible = false;
                soundEffectWindow.reset();
            };

            setCallBacksForEffectWindow();

            setInitialValuesFromSettingsFileEffectWindow();


        }
        else {
            soundEffectWindow->setVisible(true);
            soundEffectWindow->toFront(true);
        }

        // Grab focus asynchronously so the overlay's button-click event
        // finishes first and the OS doesn't steal activation back.
        juce::MessageManager::callAsync([this]()
        {
            if (soundEffectWindow)
            {
                soundEffectWindow->toFront(false);
                soundEffectWindow->grabKeyboardFocus();
            }
        });
    };
}

void MainComponent::toggleSettingsPanel()
{
    if (settingsPanel.isVisible()) {
        settingsPanel.setVisible(false);
    }
    else {
        settingsPanel.setVisible(true);
    }
}

void MainComponent::toggleSettingsButton()
{
    if (settingsButton.isVisible())
        settingsButton.setVisible(false);
    else settingsButton.setVisible(true);
}

void MainComponent::toggleMIDIButton()
{
    if (this->midiButton.isVisible())
        midiButton.setVisible(false);
    else midiButton.setVisible(true);
}

void MainComponent::togglePlayButton()
{
    if (this->playButton.isVisible())
        this->playButton.setVisible(false);
    else playButton.setVisible(true);
}

void MainComponent::toggleMIDIsettingsIcon()
{
    if (this->helpIcon.isVisible())
        helpIcon.setVisible(false);
    else helpIcon.setVisible(true);
}

void MainComponent::toggleHPanel()
{
    if (headerPanel.isVisible())
        headerPanel.setVisible(false);
    else headerPanel.setVisible(true);

    toggleHomeButton();
    toggleColourSelectorButton();
    toggleInstrumentSelectorButton();
    toggleRecordButtons();
    togglePlayRecordingButton();
    toggleKnobs();
    toggleDisplay();
    toggleParticleToggle();
    toggleHandInstrumentToggle();
    toggleAnnotation();
    toggleSections();
    toggleChordHelperButton();
}

void MainComponent::toggleHomeButton()
{
    if (homeButton.isVisible())
        homeButton.setVisible(false);
    else homeButton.setVisible(true);
}

void MainComponent::toggleColourSelectorButton()
{
    if (colourSelectorButton.isVisible())
        colourSelectorButton.setVisible(false);
    else colourSelectorButton.setVisible(true);
}

void MainComponent::toggleInstrumentSelectorButton()
{
    if (instrumentSelectorButton.isVisible())
        instrumentSelectorButton.setVisible(false);
    else instrumentSelectorButton.setVisible(true);
}

void MainComponent::toggleRecordButtons()
{
    if (startRecording.isVisible())
        startRecording.setVisible(false);
    else startRecording.setVisible(true);

    if (stopRecording.isVisible())
        stopRecording.setVisible(false);
    else stopRecording.setVisible(true);

    if (startPlayback.isVisible())
        startPlayback.setVisible(false);
    else startPlayback.setVisible(true);
}

void MainComponent::toggleSaveRecordingButton()
{
    if (saveRecordingButton.isVisible())
        saveRecordingButton.setVisible(false);
    else saveRecordingButton.setVisible(true);
}

void MainComponent::togglePlayRecordingButton()
{
    if (playRecordingFileButton.isVisible())
        playRecordingFileButton.setVisible(false);
    else playRecordingFileButton.setVisible(true);
}

void MainComponent::toggleKnobs()
{
    if (volumeKnob.isVisible())
        volumeKnob.setVisible(false);
    else volumeKnob.setVisible(true);
}

void MainComponent::toggleDisplay()
{
    if (display->isVisible())
        display->setVisible(false);
    display->setVisible(true);
}

void MainComponent::toggleParticleToggle()
{
    if (particleToggle.isVisible())
        particleToggle.setVisible(false);
    else particleToggle.setVisible(true);
}

void MainComponent::toggleHandInstrumentToggle()
{
    if (leftHandInstrumentToggle.isVisible())
        leftHandInstrumentToggle.setVisible(false);
    else leftHandInstrumentToggle.setVisible(true);

    if (rightHandInstrumentToggle.isVisible())
        rightHandInstrumentToggle.setVisible(false);
    else rightHandInstrumentToggle.setVisible(true);
}

void MainComponent::toggleUpdateKeysButton()
{
    if (updateNumberOfKeysDevice.isVisible())
        updateNumberOfKeysDevice.setVisible(false);
    else updateNumberOfKeysDevice.setVisible(true);
}

void MainComponent::toggleDevicesCBUpdate()
{
    if (devicesCBUpdate.isVisible())
        devicesCBUpdate.setVisible(false);
    else devicesCBUpdate.setVisible(true);
}

void MainComponent::toggleSections()
{
    if (introsEndings->isVisible())
        introsEndings->setVisible(false);
    else introsEndings->setVisible(true);

    if (variationsFills->isVisible())
        variationsFills->setVisible(false);
    else variationsFills->setVisible(true);
}

void MainComponent::toggleAnnotation()
{
    if (noteNumbersAnnotation.isVisible())
        noteNumbersAnnotation.setVisible(false);
    else noteNumbersAnnotation.setVisible(true);
}

void MainComponent::toggleChordHelperButton()
{
    if (chordHelperButton.isVisible())
        chordHelperButton.setVisible(false);
    else chordHelperButton.setVisible(true);
}

void MainComponent::toggleForPlaying()
{
    if (this->midiButton.isVisible())
        this->midiButton.setVisible(false);
    if (this->helpIcon.isVisible())
        this->helpIcon.setVisible(false);
    if (this->playButton.isVisible())
        this->playButton.setVisible(false);
    if (this->settingsPanel.isVisible())
        this->settingsPanel.setVisible(false);
    if (this->settingsButton.isVisible())
        this->settingsButton.setVisible(false);
    if (this->updateNumberOfKeysDevice.isVisible())
        this->updateNumberOfKeysDevice.setVisible(false);
    if (this->devicesCBUpdate.isVisible())
        this->devicesCBUpdate.setVisible(false);
}

void MainComponent::settingsInit()
{
    addAndMakeVisible(settingsButton);
    
    settingsButton.setVisible(false); //making settings button invisible for now

    settingsButton.setMouseCursor(juce::MouseCursor::PointingHandCursor);
    settingsButton.onClick = [this] {  
        settingsButtonOnClick();
    };
    panelInit();
    midiSettingsInit();
    midiIconInit();
    updateKeysButtonInit();
    devicesCBUpdateInit();
}


void MainComponent::playButtonInit()
{
    playButton.setButtonText("Play");
    playButton.setLookAndFeel(&this->customLookAndFeel);
    playButton.setColour(juce::TextButton::buttonColourId, juce::Colours::transparentBlack);
    playButton.setMouseCursor(juce::MouseCursor::PointingHandCursor);
    playButton.onClick = [this] {
        playButtonOnClick();
    };
    addAndMakeVisible(this->playButton);
    playButton.setVisible(false);
}

void MainComponent::colourSelectorButtonInit()
{
    colourSelectorButton.setButtonText("Particle colours");
    colourSelectorButton.setMouseCursor(juce::MouseCursor::PointingHandCursor);

    colourSelectorButton.onClick = [this] {
        showColourSelector();
    };
    headerPanel.addAndMakeVisible(colourSelectorButton);
    colourSelectorButton.setVisible(false);
}

void MainComponent::instrumentSelectorButtonInit()
{
    instrumentSelectorButton.setButtonText("Instruments");
    instrumentSelectorButton.setMouseCursor(juce::MouseCursor::PointingHandCursor);

    instrumentSelectorButton.onClick = [this] {
        showInstrumentSelector();
    };
    headerPanel.addAndMakeVisible(instrumentSelectorButton);
    instrumentSelectorButton.setVisible(false);
}

void MainComponent::recordButtonsInit()
{
    auto xmlRecord = juce::XmlDocument::parse(BinaryData::recordPlayback_svg);
    recordDrawable = juce::Drawable::createFromSVG(*xmlRecord);

    auto xmlStop = juce::XmlDocument::parse(BinaryData::stopButton_svg);
    stopDrawable = juce::Drawable::createFromSVG(*xmlStop);


    auto xmlPlay = juce::XmlDocument::parse(BinaryData::playButton_svg);
    playDrawable = juce::Drawable::createFromSVG(*xmlPlay);

    startRecording.setImages(recordDrawable.get());
    stopRecording.setImages(stopDrawable.get());
    startPlayback.setImages(playDrawable.get());

    startRecording.setMouseCursor(juce::MouseCursor::PointingHandCursor);
    stopRecording.setMouseCursor(juce::MouseCursor::PointingHandCursor);
    startPlayback.setMouseCursor(juce::MouseCursor::PointingHandCursor);


    startRecording.onClick = [this] {
        recordPlayer.startRecording();
        recordPlayer.handleIncomingMessage(juce::MidiMessage::programChange(1, midiHandler.getProgramNumberLeftHand()));
        recordPlayer.handleIncomingMessage(juce::MidiMessage::programChange(16, midiHandler.getProgramNumberRightHand()));

        saveRecordingButton.setVisible(false);

        if (temporaryPopup)
        {
            temporaryPopup->updateText("Recording started!");
            temporaryPopup->restartTimer();
        }
        else {
            temporaryPopup = std::make_unique<TemporaryMessage>("Recording started!");
            headerPanel.addChildComponent(temporaryPopup.get());
            temporaryPopup->setBounds(getWidth() / 2 - 50, 10, 100, 30);
            temporaryPopup->setFinishedCallBack([this] {
                //saveRecording.setVisible(true);
                temporaryPopup.reset();
                });
            temporaryPopup->setVisible(true);
        }
    };

    stopRecording.onClick = [this] {
        int result = recordPlayer.stopRecording();

        if (result == 1)
            saveRecordingButton.setVisible(true);

        if (temporaryPopup)
        {
            temporaryPopup->updateText("Recording stopped!");
            temporaryPopup->restartTimer();
        }
        else {
            temporaryPopup = std::make_unique<TemporaryMessage>("Recording stopped!");
            headerPanel.addChildComponent(temporaryPopup.get());
            temporaryPopup->setBounds(getWidth() / 2 - 50, 10, 100, 30);
            temporaryPopup->setFinishedCallBack([this] {
                temporaryPopup.reset();

                });
            temporaryPopup->setVisible(true);
        }
    };

    recordPlayer.applyPresetFunction= [&]()
    {
        midiHandler.setProgramNumber(midiHandler.getProgramNumberLeftHand(), "left");
        midiHandler.setProgramNumber(midiHandler.getProgramNumberRightHand(), "right");
    };

    recordPlayer.notifyFunction = [&]()
    {
        if (temporaryPopup)
        {
            temporaryPopup->updateText("Playback stopped!");
            temporaryPopup->restartTimer();
        }
        else
        {
            temporaryPopup = std::make_unique<TemporaryMessage>("Playback Stopped!");
            headerPanel.addChildComponent(temporaryPopup.get());
            temporaryPopup->setBounds(getWidth() / 2 - 50, 10, 100, 30);
            temporaryPopup->setFinishedCallBack([this] {
                temporaryPopup.reset();
                });
            temporaryPopup->setVisible(true);
        }
    };

    startPlayback.onClick = [this] {
        if (recordPlayer.getIsRecording())
            recordPlayer.stopRecording();

        int result=recordPlayer.startPlayBack();
        saveRecordingButton.setVisible(true);

        if(result==0)
            juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::WarningIcon, "Play recording", "No recorded events to play.");

        if (temporaryPopup && result==1)
        {
            temporaryPopup->updateText("Playback started!");
            temporaryPopup->restartTimer();
        }
        else if(temporaryPopup==nullptr) {
            temporaryPopup = std::make_unique<TemporaryMessage>("Playback started!");
            headerPanel.addChildComponent(temporaryPopup.get());
            temporaryPopup->setBounds(getWidth() / 2-50, 10, 100, 30);
            temporaryPopup->setFinishedCallBack([this] {
                temporaryPopup.reset();
                });
            temporaryPopup->setVisible(true);
        }
    };

    startRecording.setColour(juce::TextButton::buttonColourId, juce::Colours::red);
    stopRecording.setColour(juce::TextButton::buttonColourId, juce::Colours::darkgrey);
    startPlayback.setColour(juce::TextButton::buttonColourId, juce::Colours::green);

    headerPanel.addAndMakeVisible(startRecording);
    headerPanel.addAndMakeVisible(stopRecording);
    headerPanel.addAndMakeVisible(startPlayback);

    startRecording.setVisible(false);
    stopRecording.setVisible(false);
    startPlayback.setVisible(false);

}

void MainComponent::saveRecordingButtonInit()
{
    saveRecordingButton.setButtonText("Save recording");

    saveRecordingButton.onClick = [this]{
        saveRecordingToFile();
        };

    saveRecordingButton.setMouseCursor(juce::MouseCursor::PointingHandCursor);
    headerPanel.addAndMakeVisible(saveRecordingButton);
    saveRecordingButton.setVisible(false);
}

void MainComponent::playRecordingButtonInit()
{
    playRecordingFileButton.setButtonText("Play file recording");

    playRecordingFileButton.onClick = [this] {
        playRecordingFromFile();
    };

    playRecordingFileButton.setMouseCursor(juce::MouseCursor::PointingHandCursor);
    headerPanel.addAndMakeVisible(playRecordingFileButton);
    playRecordingFileButton.setVisible(false);
}

void MainComponent::knobsInit()
{
    volumeKnob.setSliderStyle(juce::Slider::Rotary);
    volumeKnob.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);

    volumeKnob.setLookAndFeel(&customKnobLookAndFeel);
    headerPanel.addAndMakeVisible(volumeKnob);
    volumeKnob.setVisible(false);
}

void MainComponent::displayInit()
{
    display = std::make_unique<Display>(deviceOpenedOUT,400);
    headerPanel.addAndMakeVisible(display.get());
    display->setVisible(false);
    display->setVisible(false);
}

void MainComponent::toggleButtonInit()
{
    particleToggle.setButtonText("Toggle particles");
    particleToggle.setMouseCursor(juce::MouseCursor::PointingHandCursor);

    particleToggle.onClick = [this]()
    {
        noteLayer->setSpawnParticleState(particleToggle.getToggleState());
    };

    particleToggle.setColour(juce::ToggleButton::tickColourId, juce::Colours::green);
    particleToggle.setColour(juce::ToggleButton::tickDisabledColourId, juce::Colours::grey);

    particleToggle.setToggleState(false,juce::dontSendNotification);

    headerPanel.addAndMakeVisible(particleToggle);
    particleToggle.setVisible(false);
}

void MainComponent::keyBoardUIinit(int min, int max)
{
    keyboardInitialized = true;
    addAndMakeVisible(keyboard);
    int keyboardHeight = (int)getHeight() * 0.21;

    keyboard.setBounds(0, getHeight() - keyboardHeight, getWidth(), keyboardHeight);

    this->keyboard.set_min_and_max(min, max);
    noteLayer = std::make_unique<NoteLayer>(this->keyboard);
    midiHandler.addListener(noteLayer.get());
    noteLayer->setBounds(0, headerPanel.getHeight(), getWidth(), getHeight() - keyboardHeight - headerPanel.getHeight());
    addAndMakeVisible(noteLayer.get());

    this->keyboard.repaint();
}

void MainComponent::panelInit()
{
    addAndMakeVisible(settingsPanel);
    settingsPanel.toFront(true);
    settingsPanel.setVisible(false);
}

void MainComponent::midiIconInit()
{
    helpIcon.setFont(juce::Font(30.0f));  // Set the font size
    helpIcon.setColour(juce::Label::textColourId, juce::Colours::lightgrey.withAlpha(0.9f));
    helpIcon.setTooltip("This setting helps you adjust/select:\n *The volume\n *The reverb\n *The input device\n *The output device ");
    helpIcon.setJustificationType(juce::Justification::left);
    settingsPanel.addAndMakeVisible(helpIcon);
    helpIcon.setText("?", juce::dontSendNotification);
    helpIcon.setVisible(false);
}

void MainComponent::midiSettingsInit()
{
    midiButton.setButtonText("MIDI Settings");
    midiButton.onClick = [this] {
        midiButtonOnClick();
    };
    settingsPanel.addAndMakeVisible(midiButton);
    midiButton.setMouseCursor(juce::MouseCursor::PointingHandCursor);
    midiButton.setVisible(false);
}

void MainComponent::headerPanelInit()
{
    addAndMakeVisible(headerPanel);
    headerPanel.toFront(true);
    headerPanel.setVisible(false);

    homeButtonInit();
    colourSelectorButtonInit();
    instrumentSelectorButtonInit();
    recordButtonsInit();
    saveRecordingButtonInit();
    playRecordingButtonInit();
    toggleButtonInit();
    toggleHandButtonsInit();
    knobsInit();
    annotationInit();
    sectionsInit();
    chordHelperButtonInit();
}

void MainComponent::homeButtonInit()
{
    homeButton.setButtonText("Home");
    homeButton.setMouseCursor(juce::MouseCursor::PointingHandCursor);

    homeButton.onClick = [this] {
        homeButtonOnClick();
    };

    headerPanel.addAndMakeVisible(homeButton);
    homeButton.setVisible(false);

}

void MainComponent::toggleHandButtonsInit()
{
    leftHandInstrumentToggle.setButtonText("Left hand");
    leftHandInstrumentToggle.setMouseCursor(juce::MouseCursor::PointingHandCursor);

    leftHandInstrumentToggle.setColour(juce::ToggleButton::tickColourId, juce::Colours::green);
    leftHandInstrumentToggle.setColour(juce::ToggleButton::tickDisabledColourId, juce::Colours::grey);

    leftHandInstrumentToggle.setToggleState(false,juce::dontSendNotification);
    headerPanel.addAndMakeVisible(leftHandInstrumentToggle);
    leftHandInstrumentToggle.setVisible(false);


    rightHandInstrumentToggle.setButtonText("Right hand");
    rightHandInstrumentToggle.setMouseCursor(juce::MouseCursor::PointingHandCursor);

    rightHandInstrumentToggle.setColour(juce::ToggleButton::tickColourId, juce::Colours::green);
    rightHandInstrumentToggle.setColour(juce::ToggleButton::tickDisabledColourId, juce::Colours::grey);

    rightHandInstrumentToggle.setToggleState(false, juce::dontSendNotification);
    headerPanel.addAndMakeVisible(rightHandInstrumentToggle);
    rightHandInstrumentToggle.setVisible(false);

}

void MainComponent::updateKeysButtonInit()
{
    updateNumberOfKeysDevice.setButtonText("Update number of keys");
    updateNumberOfKeysDevice.setMouseCursor(juce::MouseCursor::PointingHandCursor);

    updateNumberOfKeysDevice.onClick = [this]()
    {
        updateKeysButtonOnClick();
    };

    settingsPanel.addAndMakeVisible(updateNumberOfKeysDevice);
    updateNumberOfKeysDevice.setVisible(false);
}

void MainComponent::devicesCBUpdateInit()
{
    devicesCBUpdate.setText("Select a device", juce::dontSendNotification);
    devicesCBUpdate.setMouseCursor(juce::MouseCursor::PointingHandCursor);
    settingsPanel.addAndMakeVisible(devicesCBUpdate);
    devicesCBUpdate.setVisible(false);
}

void MainComponent::sectionsInit()
{
    std::unordered_map<juce::String, std::function<void()>> callbacksIntroEnding = {
        {"Intro 1", [this]() { handleIntro("Intro 1");  }},
        {"Intro 2", [this]() { handleIntro("Intro 2");  }},
        {"Intro 3", [this]() { handleIntro("Intro 3");  }},

        {"Ending 1", [this]() { handleEnding("Ending 1"); }},
        {"Ending 2", [this]() { handleEnding("Ending 2");  }},
        {"Ending 3", [this]() { handleEnding("Ending 3");  }}

    };


    introsEndings = std::make_unique<StyleSectionComponent>(std::vector<juce::String>{"Intros", "Endings"},
        std::vector<std::vector<juce::String>>{ {"Intro 1", "Intro 2", "Intro 3"}, { "Ending 1", "Ending 2", "Ending 3"}},callbacksIntroEnding);



    std::unordered_map<juce::String, std::function<void()>> callbacksVarFillBreak = {
        {"Var 1", [this]() { handleVar("Var 1");  }},
        {"Var 2", [this]() { handleVar("Var 2");  }},
        {"Var 3", [this]() { handleVar("Var 3");  }},
        {"Var 4", [this]() { handleVar("Var 4");  }},

        {"Fill 1", [this]() { handleFill("Fill 1");  }},
        {"Fill 2", [this]() { handleFill("Fill 2");  }},
        {"Fill 3", [this]() { handleFill("Fill 3");  }},
        {"Fill 4", [this]() { handleFill("Fill 4");  }},

        {"Break", [this]() { handleBreak("Break");  }}

    };

    variationsFills = std::make_unique<StyleSectionComponent>(std::vector<juce::String>{"Variations", "Fills"},
        std::vector<std::vector<juce::String>>{ {"Var 1", "Var 2", "Var 3", "Var 4"}, {"Fill 1", "Fill 2", "Fill 3", "Fill 4"}}, callbacksVarFillBreak);

    
    introsEndings->undoLastClickedForOtherGroupSections = [this]()
    {
        variationsFills->deactivateLastClicked();
    };

    variationsFills->undoLastClickedForOtherGroupSections = [this]()
    {
        introsEndings->deactivateLastClicked();
    };

    headerPanel.addAndMakeVisible(introsEndings.get());
    introsEndings->setVisible(false);

    headerPanel.addAndMakeVisible(variationsFills.get());
    variationsFills->setVisible(false);

}

void MainComponent::annotationInit()
{
    noteNumbersAnnotation.setButtonText("Note annotation");
    noteNumbersAnnotation.setMouseCursor(juce::MouseCursor::PointingHandCursor);

    noteNumbersAnnotation.setColour(juce::ToggleButton::tickColourId, juce::Colours::green);
    noteNumbersAnnotation.setColour(juce::ToggleButton::tickDisabledColourId, juce::Colours::grey);

    noteNumbersAnnotation.setToggleState(false, juce::dontSendNotification);
    headerPanel.addAndMakeVisible(noteNumbersAnnotation);

    noteNumbersAnnotation.onClick = [this]()
    {
        if (noteNumbersAnnotation.getToggleState() == true)
            keyboard.setAnnotationState(true);
        else keyboard.setAnnotationState(false);

        keyboard.repaint();
    };

    noteNumbersAnnotation.setVisible(false);
}

void MainComponent::chordHelperButtonInit()
{
    chordHelperButton.setButtonText("Chord helper");
    chordHelperButton.setMouseCursor(juce::MouseCursor::PointingHandCursor);

    chordHelperButton.onClick = [this]()
    {
        std::vector<Chord> lib=myChordLibrary;

        auto chordBrowser = std::make_unique<ChordBrowserComponent>(lib);

        chordBrowser->onChordChosen = [this](const Chord& c)
        {
            playChordOnClick(c);
        };

        juce::CallOutBox::launchAsynchronously(std::move(chordBrowser), chordHelperButton.getScreenBounds(), nullptr);
    };

    headerPanel.addAndMakeVisible(chordHelperButton);
    chordHelperButton.setVisible(false);
}

void MainComponent::settingsButtonOnClick()
{
    toggleSettingsPanel(); toggleMIDIButton(); toggleMIDIsettingsIcon(); toggleUpdateKeysButton(); toggleDevicesCBUpdate();
}

void MainComponent::midiButtonOnClick()
{
    if (!midiWindow)
    {
        midiWindow = std::make_unique<MIDIWindow>(this->MIDIDevice, devicesIN, devicesOUT, propertiesFile);
        loadSettings();
    }
    else {
        midiWindow->setVisible(true);
        midiWindow->toFront(true);
    }
}

void MainComponent::homeButtonOnClick()
{
    currentBackground = cachedImageMainWindow;
    repaint();

    toggleHPanel();

    togglePlayButton();
    //toggleSettingsButton(); don't need this button for now
    keyboard.setVisible(false);
    this->noteLayer->setVisible(false);
    this->noteLayer->resetState();
    if(this->recordPlayer.getIsPlaying())
        this->recordPlayer.stopPlayBack();

    if (display && display->getNumTabs()==3)
        display->homeButtonInteraction();
}

void MainComponent::playButtonOnClick()
{
    MIDIDevice.getAvailableDevicesMidiIN(devicesIN);
    MIDIDevice.getAvailableDevicesMidiOUT(devicesOUT);

    if (openingDevicesForPlay()) {
        if (!this->keyListener.getIsKeyboardInput())
        {
            midiHandler.onAddCallBack = [this](const juce::String& vid, const juce::String& pid, std::function<void(const juce::String&, int)> resultCallback)
            {
                addDeviceWindow = std::make_unique<AddDeviceWindow>(vid, pid);
                addDeviceWindow->onAddDevice = [this,vid,pid,resultCallback](const juce::String& name, int keys)
                {
                    resultCallback(name, keys);
                    dataBase.addDeviceJson(vid, pid, name, keys);
                };
                addDeviceWindow->setVisible(true);
            };
        }

        juce::String PID = MIDIDevice.extractPID(MIDIDevice.get_identifier());
        juce::String VID = MIDIDevice.extractVID(MIDIDevice.get_identifier());
        juce::String name = dataBase.getDeviceName(VID, PID);
        int nrKeys = dataBase.getNrKeysPidVid(VID, PID);

        if (midiHandler.handlePlayableRange(VID, PID, dataBase.getNrKeysPidVid(VID, PID), this->keyListener.getIsKeyboardInput()) < 0)
        {
            this->MIDIDevice.deviceCloseIN();
            this->MIDIDevice.deviceCloseOUT();
            return;
        }



        this->recordPlayer.setOutputDevice(MIDIDevice.getDeviceOUT());
        if(midiWindow)
            this->midiWindow->setVisible(false);


        
        

        MIDIDevice.setPID(PID);
        MIDIDevice.setVID(VID);
        MIDIDevice.setDeviceName(name);

        if (VID.length() < 4 || PID.length() < 4)
            this->display->set_VID_PID("", "");
        else this->display->set_VID_PID(VID, PID);

        this->display->readSettingsFromJSON();

        currentBackground = playBackground;
        repaint();
        toggleHPanel();

        sendEffectsBeforePlaying();

        /*
        MIDIDevice.sendMidiCC(1, 7, MIDIDevice.getVolume(1));
        MIDIDevice.sendMidiCC(1,91,MIDIDevice.getReverb(1));
        MIDIDevice.sendMidiCC(16,91,MIDIDevice.getReverb(16));
        MIDIDevice.sendMidiCC(16,7,MIDIDevice.getVolume(16));
        */

        recordPlayer.setReverb(MIDIDevice.getReverb(1),1);
        recordPlayer.setReverb(MIDIDevice.getReverb(16), 16);

        recordPlayer.setVolume(MIDIDevice.getVolume(1), 1);
        recordPlayer.setVolume(MIDIDevice.getVolume(16), 16);

        display->setDeviceOutput(MIDIDevice.getDeviceOUT());

        if (!keyboardInitialized)
        {
            keyBoardUIinit(MIDIDevice.get_minNote(), MIDIDevice.get_maxNote());
            midiHandler.setProgramNumber(midiHandler.getProgramNumberLeftHand(), "left");
            midiHandler.setProgramNumber(midiHandler.getProgramNumberRightHand(), "right");

            recordPlayer.setProgarmNumber(midiHandler.getProgramNumberLeftHand(),"left");
            recordPlayer.setProgarmNumber(midiHandler.getProgramNumberRightHand(), "right");
        }
        else {
            keyboard.setVisible(true);
            this->noteLayer->setVisible(true);
        }

        if (this->keyListener.getIsKeyboardInput())
        {
            this->keyboard.set_min_and_max(keyListener.getStartNoteKeyboardInput(), keyListener.getFinishNoteKeyboardInput());
            this->display->set_min_max(keyListener.getStartNoteKeyboardInput(), keyListener.getFinishNoteKeyboardInput());
        }
        else
        {
            this->keyboard.set_min_and_max(MIDIDevice.get_minNote(), MIDIDevice.get_maxNote());
            this->display->set_min_max(MIDIDevice.get_minNote(), MIDIDevice.get_maxNote());
        }
        
        this->midiHandler.set_start_end_notes(this->display->getStartNote(), this->display->getEndNote());
        this->midiHandler.set_left_right_bounds(this->display->getLeftBound(), this->display->getRightBound());

        //this->display()

        keyboard.setIsDrawn(false);
        keyboard.repaint();
        toggleForPlaying();

        if (!this->keyListener.getIsKeyboardInput())
        {
            
            juce::String vid = MIDIDevice.getVID();
            juce::String pid = MIDIDevice.getPID();
            juce::String name = MIDIDevice.getName();
            int nrKeys = MIDIDevice.getNrKeysAfterInitialized();
            std::thread([this, vid, pid, name, nrKeys]()
                {
                    client->addOrUpdateDevice(vid, pid, name, nrKeys);
                }).detach();
        }
    }
}

void MainComponent::updateKeysButtonOnClick()
{
    juce::String selectedText = devicesCBUpdate.getText();
    if (selectedText == "Select a device" || selectedText == "No devices found!")
        return;

    juce::String identifier = updateDevicesMap[selectedText];
    juce::String VID = MIDIDevice.extractVID(identifier);
    juce::String PID = MIDIDevice.extractPID(identifier);


    auto* window = new juce::AlertWindow(
        "Enter Number of Keys",
        "Please enter how many keys this device has:",
        juce::AlertWindow::QuestionIcon
    );

    window->addTextEditor("keys", "", "Number of keys");
    window->addButton("OK", 1, juce::KeyPress(juce::KeyPress::returnKey));
    window->addButton("Cancel", 0, juce::KeyPress(juce::KeyPress::escapeKey));
    window->enterModalState(
        true, // make modal
        juce::ModalCallbackFunction::create(
            [this, VID, PID, selectedText, window](int result)
            {
                if (result == 0)
                {
                    delete window;
                    return;
                }

                juce::String keysStr = window->getTextEditorContents("keys").trim();
                delete window;

                if (!keysStr.containsOnly("0123456789"))
                {
                    juce::AlertWindow::showMessageBoxAsync(
                        juce::AlertWindow::WarningIcon,
                        "Invalid Input",
                        "Please enter a valid number of keys.");
                    return;
                }

                int numKeys = keysStr.getIntValue();
                if (numKeys != 49 && numKeys != 61 && numKeys != 76 && numKeys != 88)
                {
                    juce::AlertWindow::showMessageBoxAsync(
                        juce::AlertWindow::WarningIcon,
                        "Invalid Key Count",
                        "Please enter one of: 49, 61, 76 or 88.");
                    return;
                }

                dataBase.updateDeviceJson(VID, PID, "", numKeys);

                juce::AlertWindow::showMessageBoxAsync(
                    juce::AlertWindow::InfoIcon,
                    "Success",
                    "Device updated successfully!");
            }
        )
    );

}

juce::Image MainComponent::getImageForInstruments(const std::string& type)
{
    const void* binaryData=nullptr;
    size_t binaryDataSize = 0;
    if (type == "AGP")
    {
        binaryData = BinaryData::AcousticGrandPiano_png;
        binaryDataSize = BinaryData::AcousticGrandPiano_pngSize;
    }
    else if (type == "EP")
    {
        binaryData = BinaryData::ElectricPiano_png;
        binaryDataSize = BinaryData::ElectricPiano_pngSize;
    }
    else if (type == "ACB")
    {
        binaryData = BinaryData::AcousticBass_png;
        binaryDataSize = BinaryData::AcousticBass_pngSize;
    }
    else if (type == "EBF")
    {
        binaryData = BinaryData::ElectricBass_png;
        binaryDataSize = BinaryData::ElectricBass_pngSize;
    }
    else if (type == "EBP")
    {
        binaryData = BinaryData::ElectricBass2_png;
        binaryDataSize = BinaryData::ElectricBass2_pngSize;
    }
    else if (type == "FTB")
    {
        binaryData = BinaryData::FretlessBass_png;
        binaryDataSize = BinaryData::FretlessBass_pngSize;
    }
    else if (type == "SLB1")
    {
        binaryData = BinaryData::SlapBass_png;
        binaryDataSize = BinaryData::SlapBass_pngSize;
    }
    else if (type == "SLB2")
    {
        binaryData = BinaryData::SlapBass2_png;
        binaryDataSize = BinaryData::SlapBass2_pngSize;
    }
    else if (type == "SYB1")
    {
        binaryData = BinaryData::SynthBass_png;
        binaryDataSize = BinaryData::SynthBass_pngSize;
    }
    else if (type == "SYB2")
    {
        binaryData = BinaryData::SynthBass2_png;
        binaryDataSize = BinaryData::SynthBass2_pngSize;
    }
    if (binaryData)
    {
        juce::MemoryInputStream imageStream{ binaryData,binaryDataSize,false };
        juce::Image img = juce::ImageFileFormat::loadFrom(imageStream);
        return img;
    }
    return juce::Image();
}

void MainComponent::buildTree()
{
    auto dummyRoot = std::make_unique< InstrumentTreeItem>("Instruments");

    auto rootPianos = std::make_unique<InstrumentTreeItem>( "Pianos" );
    rootPianos->addSubItem(createInstrumentItem(getImageForInstruments("AGP"), "Acoustic Grand Piano", 0));
    rootPianos->addSubItem(createInstrumentItem(getImageForInstruments("EP"),"Electric Piano", 4));
    rootPianos->setOpen(false);
    dummyRoot->addSubItem(rootPianos.release());

    auto rootBasses = std::make_unique<InstrumentTreeItem>( "Basses" );
    rootBasses->addSubItem(createInstrumentItem(getImageForInstruments("ACB"),"Acoustic Bass", 32));
    rootBasses->addSubItem(createInstrumentItem(getImageForInstruments("EBF"),"Electric Bass (finger)", 33));
    rootBasses->addSubItem(createInstrumentItem(getImageForInstruments("EBP"),"Electric Bass (pick)", 34));
    rootBasses->addSubItem(createInstrumentItem(getImageForInstruments("FTB"),"Fretless Bass", 35));
    rootBasses->addSubItem(createInstrumentItem(getImageForInstruments("SLB1"),"Slap Bass 1", 36));
    rootBasses->addSubItem(createInstrumentItem(getImageForInstruments("SLB2"),"Slap Bass 2", 37));
    rootBasses->addSubItem(createInstrumentItem(getImageForInstruments("SYB1"),"Synth Bass 1", 38));
    rootBasses->addSubItem(createInstrumentItem(getImageForInstruments("SYB2"),"Synth Bass 2", 39));
    rootBasses->setOpen(false);
    dummyRoot->addSubItem(rootBasses.release());

    auto rootGuitars = std::make_unique<InstrumentTreeItem>("Guitars");
    rootGuitars->addSubItem(createInstrumentItem(getImageForInstruments("ACB"),"Nylon Acoustic Guitar",24));
    rootGuitars->addSubItem(createInstrumentItem(getImageForInstruments("ACB"), "Steel Acoustic Guitar", 25));
    rootGuitars->addSubItem(createInstrumentItem(getImageForInstruments("ACB"), "Jazz Electric Guitar", 26));
    rootGuitars->addSubItem(createInstrumentItem(getImageForInstruments("ACB"), "Clean Electric Guitar", 27));
    rootGuitars->setOpen(false);
    dummyRoot->addSubItem(rootGuitars.release());

    auto woodWinds = std::make_unique<InstrumentTreeItem>("WoodWinds");
    woodWinds->addSubItem(createInstrumentItem(getImageForInstruments("ACB"), "Flute", 73));
    woodWinds->addSubItem(createInstrumentItem(getImageForInstruments("ACB"), "Clarinet", 71));
    woodWinds->addSubItem(createInstrumentItem(getImageForInstruments("ACB"), "Oboe", 68));
    woodWinds->addSubItem(createInstrumentItem(getImageForInstruments("ACB"), "Bassoon", 70));
    woodWinds->setOpen(false);
    dummyRoot->addSubItem(woodWinds.release());

    auto rootBrass = std::make_unique<InstrumentTreeItem>("Brass");
    rootBrass->addSubItem(createInstrumentItem(getImageForInstruments("ACB"), "Trumpet", 56));
    rootBrass->addSubItem(createInstrumentItem(getImageForInstruments("ACB"), "Trombone", 57));
    rootBrass->addSubItem(createInstrumentItem(getImageForInstruments("ACB"), "French Horn", 59));
    rootBrass->setOpen(false);
    dummyRoot->addSubItem(rootBrass.release());


    auto rootStrings = std::make_unique<InstrumentTreeItem>("Strings");
    rootStrings->addSubItem(createInstrumentItem(getImageForInstruments("ACB"), "Violin", 40));
    rootStrings->addSubItem(createInstrumentItem(getImageForInstruments("ACB"), "Viola", 41));
    rootStrings->addSubItem(createInstrumentItem(getImageForInstruments("ACB"), "Cello", 42));
    rootStrings->addSubItem(createInstrumentItem(getImageForInstruments("ACB"), "Contrabass", 43));
    rootStrings->setOpen(false);
    dummyRoot->addSubItem(rootStrings.release());

    auto rootReeds = std::make_unique<InstrumentTreeItem>("Reeds");
    rootReeds->addSubItem(createInstrumentItem(getImageForInstruments("ACB"), "Accordion", 21));   // GM program 21
    rootReeds->addSubItem(createInstrumentItem(getImageForInstruments("ACB"), "Harmonica", 22));   // GM program 22
    rootReeds->addSubItem(createInstrumentItem(getImageForInstruments("ACB"), "Bandoneon", 23));   // Custom / assign if needed
    rootReeds->setOpen(false);
    dummyRoot->addSubItem(rootReeds.release());

    auto rootOrgans = std::make_unique<InstrumentTreeItem>("Organs");
    rootOrgans->addSubItem(createInstrumentItem(getImageForInstruments("ACB"), "Hammond Organ", 16));     // GM Program 17
    rootOrgans->addSubItem(createInstrumentItem(getImageForInstruments("ACB"), "Percussive Organ", 17)); // GM Program 18
    rootOrgans->addSubItem(createInstrumentItem(getImageForInstruments("ACB"), "Rock Organ", 18));       // GM Program 19
    rootOrgans->addSubItem(createInstrumentItem(getImageForInstruments("ACB"), "Church Organ", 19));     // GM Program 20
    rootOrgans->addSubItem(createInstrumentItem(getImageForInstruments("ACB"), "Reed Organ", 20));       // GM Program 21
    rootOrgans->setOpen(false);
    dummyRoot->addSubItem(rootOrgans.release());

    dummyRoot->setOpen(true);
    treeView->setRootItem(dummyRoot.release());
    
}

InstrumentTreeItem* MainComponent::createInstrumentItem(const juce::Image& img, const juce::String& name, int program)
{
    auto item = std::make_unique<InstrumentTreeItem>(img,name,program);
    item->onProgramSelected = [&](int programNumber, juce::String name="")
    {
        if (leftHandInstrumentToggle.getToggleState() == true)
        {
            midiHandler.setProgramNumber(programNumber, "left"); //setting the actual sound to the new program for both midiHandler and also the recordPlayer by listener
            if (propertiesFile)
            {
                propertiesFile->setValue("leftInstrumentNumber", programNumber);
                propertiesFile->saveIfNeeded();
            }
        }

        if (rightHandInstrumentToggle.getToggleState() == true)
        {
            midiHandler.setProgramNumber(programNumber, "right");
            if (propertiesFile)
            {
                propertiesFile->setValue("rightInstrumentNumber", programNumber);
                propertiesFile->saveIfNeeded();
            }
        }

        recordPlayer.setProgarmNumber(programNumber); //setting only the value
    };

    return item.release();
}

bool MainComponent::openingDevicesForPlay()
{
    for (const auto& device : devicesIN)
    {
        if (device == "No devices found!")
        {
            juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::InfoIcon, "ERROR", "There are no available input devices!", "OK");
            return false;
        }
    }
    for (const auto& device : devicesOUT)
    {
        if (device == "No devices found!")
        {
            juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::InfoIcon, "ERROR", "There are no available output devices!", "OK");
            return false;
        }
    }
    int indexIN = this->MIDIDevice.getDeviceIndexIN();
    int indexOUT = this->MIDIDevice.getDeviceIndexOUT();
    bool result;

    if (this->MIDIDevice.getDeviceNameBasedOnIndex(indexIN, 0) == "PC Keyboard")
    {
        //here is the case where user selects keyboard from pc.
        this->MIDIDevice.set_minNote(keyListener.getStartNoteKeyboardInput());
        this->MIDIDevice.set_maxNote(keyListener.getFinishNoteKeyboardInput());
        this->keyboard.set_min_and_max(keyListener.getStartNoteKeyboardInput(), keyListener.getFinishNoteKeyboardInput());
        this->keyListener.setIsKeyboardInput(true);
        this->grabKeyboardFocus();
    }
    else {
        this->keyListener.setIsKeyboardInput(false);
        result = this->MIDIDevice.deviceOpenIN(indexIN, &midiHandler);
        if (!result)
        {

            juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::InfoIcon, "ERROR", "Failed to open input device.", "OK");
            return false;
        }
        this->deviceOpenedIN = this->MIDIDevice.getDeviceIN();
    }
    result = this->MIDIDevice.deviceOpenOUT(indexOUT);
    if (!result)
    {
        this->MIDIDevice.deviceCloseIN();  
        juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::InfoIcon, "ERROR", "Failed to open output device.", "OK");
        return false;
    }
    this->deviceOpenedOUT = this->MIDIDevice.getDeviceOUT();
    return true;
}

void MainComponent::showColourSelector()
{
    colourSelector = new juce::ColourSelector(
        juce::ColourSelector::showSliders |
        juce::ColourSelector::showColourAtTop |
        juce::ColourSelector::showColourspace
        );
    colourSelector->setName("Colour Picker");
    colourSelector->setColour(juce::ColourSelector::backgroundColourId, juce::Colours::transparentBlack);
    colourSelector->addChangeListener(this);
    colourSelector->setSize(500, 300);
    auto area1 = juce::Rectangle<int>(0, 0, 500, headerPanel.getHeight()+headerPanel.getY());


    noteLayer->setVisible(false);
    noteLayer->resetState();
    //noteLayer->repaint();
    this->keyboard.setIsDrawn(false);
    //this->keyboard.repaint();


    repaint();


    juce::CallOutBox::launchAsynchronously(std::unique_ptr<juce::ColourSelector>(colourSelector),area1,this);
}

void MainComponent::showInstrumentSelector()
{
    if (treeView == nullptr)
    {
        treeView = std::make_unique<juce::TreeView>();
        treeView->setSize(500, 300);
    }
    if (treeView->getRootItem() != nullptr)
    {
        treeView->deleteRootItem();
        //DBG("deleting root from show");
    }
    //else DBG("not deleting root from show");
    buildTree();
    treeView->setColour(juce::TreeView::backgroundColourId, juce::Colours::white);
    auto area1 = juce::Rectangle<int>(0, 0, 500, headerPanel.getHeight() + headerPanel.getY());
    
    noteLayer->setVisible(false);
    noteLayer->resetState();
    //noteLayer->repaint();
    this->keyboard.setIsDrawn(false);
    //this->keyboard.repaint();

    repaint();

    auto holder = std::make_unique<TreeViewHolder>(treeView.get());
    holder->setOpaque(true);

    juce::CallOutBox::launchAsynchronously(std::move(holder), area1, this);
}

void MainComponent::saveRecordingToFile(double tempo)
{
    if (recordPlayer.getSizeRecorded() <= 1)
    {
        juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::WarningIcon, "Save recording", "No recorded events to play.");
        return;
    }

    fileChooser = std::make_unique<juce::FileChooser>("Save file", juce::File::getCurrentWorkingDirectory(), "*.mid");

    fileChooser->launchAsync(juce::FileBrowserComponent::saveMode | juce::FileBrowserComponent::canSelectFiles,
        [this,tempo](const juce::FileChooser& fc)
        {
            auto file = fc.getResult();
            if (file != juce::File())
            {
                juce::String errorMsg;
                if (!recordPlayer.saveRecordingToFile(file, errorMsg,tempo))
                {
                    juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::WarningIcon, "Save Failed", errorMsg);
                }
                else
                {
                    juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::InfoIcon, "Saved", "Recording saved to:\n" + file.getFullPathName());
                }
            }
            fileChooser.reset();
        });
}

void MainComponent::playRecordingFromFile(double tempo)
{
    fileChooser = std::make_unique<juce::FileChooser>("Play from file", juce::File::getCurrentWorkingDirectory(), "*.mid");
    fileChooser->launchAsync(juce::FileBrowserComponent::saveMode | juce::FileBrowserComponent::canSelectFiles,
        [this, tempo](const juce::FileChooser& fc)
        {
            auto file = fc.getResult();
            if (file != juce::File())
            {
                juce::String errorMsg;
                if (!recordPlayer.parseRecordingFromFile(file, errorMsg))
                    juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::WarningIcon, "Play Failed", errorMsg);
                else
                    recordPlayer.startRecordingFilePlaying();
            }
            fileChooser.reset();
        });
}

void MainComponent::handleIntro(const juce::String& name)
{
    if (display)
        display->handleIntroDisplay(name);
}

void MainComponent::handleEnding(const juce::String& name)
{
    if (name == "Ending 1")
    {

    }
    else if (name == "Ending 2")
    {

    }
    else if (name == "Ending 3")
    {

    }
}

void MainComponent::handleVar(const juce::String& name)
{
    if (name == "Var 1")
    {

    }
    else if (name == "Var 2")
    {

    }
    else if (name == "Var 3")
    {

    }
    else if (name == "Var 4")
    {

    }
}

void MainComponent::handleFill(const juce::String& name)
{
    if (name == "Fill 1")
    {

    }
    else if (name == "Fill 2")
    {

    }
    else if (name == "Fill 3")
    {

    }
    else if (name == "Fill 4")
    {

    }
}

void MainComponent::handleBreak(const juce::String& name)
{

}

SmoothRotarySlider::SmoothRotarySlider()
{
    setSliderStyle(juce::Slider::Rotary);
}

void SmoothRotarySlider::mouseDown(const juce::MouseEvent& e)
{
    dragStartValue = getValue(); 
    dragStartPos = e.position; 
    juce::Slider::mouseDown(e);
}

void SmoothRotarySlider::mouseDrag(const juce::MouseEvent& e)
{
    float dx = e.position.x - dragStartPos.x;
    float dy = dragStartPos.y - e.position.y;

    float sensitivity = 0.005f;

    float delta = dx + dy;

    float newValue = dragStartValue + delta * sensitivity * (getMaximum() - getMinimum());
    setValue(newValue, juce::dontSendNotification);
}

KnobLookAndFeel::KnobLookAndFeel()
{
    rawKnobImage = juce::ImageCache::getFromMemory(BinaryData::Knob_png, BinaryData::Knob_pngSize);
    rotaryStartAngle = juce::MathConstants<float>::pi * 1.25f;
    rotaryEndAngle = juce::MathConstants<float>::pi * 2.75f; 
}

void KnobLookAndFeel::drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height, float sliderPosProportional, float rotaryStart, float rotaryEnd, juce::Slider& slider)
{
    float startAngle = rotaryStartAngle; 
    
    
    float endAngle = rotaryEndAngle;

    sliderPosProportional = juce::jlimit(0.0f, 1.0f, sliderPosProportional);

    float angle = startAngle + sliderPosProportional * (endAngle - startAngle);

    const float cx = x + width * 0.5f;
    const float cy = y + height * 0.5f; 
    const int knobSize = juce::jmin(width, height);

    g.setImageResamplingQuality(juce::Graphics::highResamplingQuality);
    if (rawKnobImage.isValid())
    {
        juce::Image scaledKnob = rawKnobImage.rescaled(knobSize, knobSize);
        juce::AffineTransform transform = juce::AffineTransform::translation(-scaledKnob.getWidth() * 0.5f, -scaledKnob.getHeight() * 0.5f).rotated(angle).translated(cx, cy);
        g.drawImageTransformed(scaledKnob, transform);
    }
    else
    {
        float radius = knobSize * 0.5f - 4.0f;
        g.setColour(juce::Colours::darkgrey); 
        
        g.fillEllipse(cx - radius, cy - radius, radius * 2.0f, radius * 2.0f);

        g.setColour(juce::Colours::orange);

        juce::Point<float> lineStart(cx, cy); 
        juce::Point<float> lineEnd(cx + radius * std::cos(angle), cy + radius * std::sin(angle)); 
        g.drawLine(lineStart.x, lineStart.y, lineEnd.x, lineEnd.y, 3.0f);
    }
}

juce::Font CustomLookAndFeel::getTextButtonFont(juce::TextButton& button, int buttonHeight)
{
    return juce::Font(40.0f, juce::Font::bold);
}

void CustomLookAndFeel::drawButtonBackground(juce::Graphics& g, juce::Button& button, const juce::Colour& backgroundColour, bool isMouseOverButton, bool isButtonDown)
{
    juce::ignoreUnused(button, isMouseOverButton, isButtonDown); g.fillAll(backgroundColour);
}

void MainComponent::Panel::paint(juce::Graphics& g)
{
    juce::Colour translucentGray = juce::Colour::fromRGBA(169, 169, 169, 204);
    juce::Colour option = juce::Colour::fromRGBA(50, 100, 95, 170);
    juce::Colour subtleBorderColor = juce::Colour::fromRGBA(169, 169, 169, 200);
    juce::Colour secondBorder = juce::Colour::fromRGBA(140, 140, 140, 220);

    g.fillAll(option); // Background of the panel
    g.setColour(subtleBorderColor); 
    g.drawRect(getLocalBounds(), 3);
}

void MainComponent::headerPanel::paint(juce::Graphics& g)
{
    juce::Colour startColour = juce::Colour(128, 0, 32);
    juce::Colour endColour = juce::Colour(212, 175, 55); 

    juce::ColourGradient gradient(startColour, 0, 0, endColour, 0, 50, false); 
    g.setGradientFill(gradient); 
    g.fillAll();
}
    