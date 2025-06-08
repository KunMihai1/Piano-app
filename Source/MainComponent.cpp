#include "MainComponent.h"

//==============================================================================
MainComponent::MainComponent()
{
    settingsInit();
    playButtonInit();
    headerPanelInit();
    togglePlayButton();
    initalizeSaveFileForUser();
    loadSettings();
    cachedImageMainWindow = juce::ImageFileFormat::loadFrom(BinaryData::MainWindow_png, BinaryData::MainWindow_pngSize);
    playBackground = juce::ImageFileFormat::loadFrom(BinaryData::playingBackground_png, BinaryData::playingBackground_pngSize);
    currentBackground = cachedImageMainWindow;

    displayInit();

    setBounds(0, 0, juce::Desktop::getInstance().getDisplays().getPrimaryDisplay()->totalArea.getWidth(),
        juce::Desktop::getInstance().getDisplays().getPrimaryDisplay()->totalArea.getHeight() - 40);
    this->addKeyListener(&this->keyListener);
    addKeyListener(this);

    midiHandler.addListener(&recordPlayer);
    midiHandler.addListener(&keyboard);

    MIDIDevice.getAvailableDevicesMidiIN(this->devicesIN);
    MIDIDevice.getAvailableDevicesMidiOUT(this->devicesOUT);
    this->toFront(true);

}

MainComponent::~MainComponent()
{
    if (this->MIDIDevice.isOpenIN())
        this->MIDIDevice.deviceCloseIN();
    if (this->MIDIDevice.isOpenOUT())
        this->MIDIDevice.deviceCloseOUT();
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
    helpIcon.setBounds(210, 20, 100, 20);

    headerPanel.setBounds(0, 0, getWidth(), 50);
    homeButton.setBounds(10, 10, 75, 30);
    colourSelectorButton.setBounds(90, 10, 100, 30);
    instrumentSelectorButton.setBounds(195, 10, 100, 30);

    startPlayback.setBounds(getWidth() - 30 - 35, 10, 30, 30);
    stopRecording.setBounds(startPlayback.getX()-30-10, 10, 30, 30);
    startRecording.setBounds(stopRecording.getX()-30-10, 10, 30, 30);

    saveRecordingButton.setBounds(startRecording.getX()-140-10, 10, 140, 30);
    playRecordingFileButton.setBounds(305, 10, 100, 30);

    //volumeKnob.setBounds(getWidth() / 2, 0, 70, 50);

    display->setBounds(getWidth() / 2, 0, 100, 50);

    if (noteLayer)
    {
        noteLayer->toFront(false);
        noteLayer->setAlwaysOnTop(true);
        noteLayer->setOpaque(true);
        keyboard.toFront(false);
        keyboard.setAlwaysOnTop(true);
        keyboard.setOpaque(true);
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
            this->keyboard.repaint();
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
            this->keyboard.repaint();
        }
    }
    return false;
}

void MainComponent::initalizeSaveFileForUser()
{
    juce::PropertiesFile::Options options;
    options.applicationName= "Synth Piano 2";
    options.filenameSuffix = "settings";
    options.folderName="Piano Enjoyers";
    options.osxLibrarySubFolder = "Application Support";
    options.commonToAllUsers = false;

    appProperties.setStorageParameters(options);
    propertiesFile = appProperties.getUserSettings();
}

void MainComponent::loadSettings()
{
    if (propertiesFile)
    {
        double savedVolume = propertiesFile->getDoubleValue("midiVolume", 100.0);
        double savedReverb = propertiesFile->getDoubleValue("midiReverb", 100.0);
        if (midiWindow)
        {
            this->midiWindow->volumeSliderSetValue(savedVolume);
            this->midiWindow->reverbSliderSetValue(savedReverb);
        }
        this->MIDIDevice.setVolume(savedVolume);
        this->MIDIDevice.setReverb(savedReverb);
    }
}

void MainComponent::focusGained(FocusChangeType)
{
    if (noteLayer)
    {
        noteLayer->toFront(false);
    }
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
}

void MainComponent::settingsInit()
{
    addAndMakeVisible(settingsButton);
    settingsButton.setMouseCursor(juce::MouseCursor::PointingHandCursor);
    settingsButton.onClick = [this] {  
        settingsButtonOnClick();
    };
    panelInit();
    midiSettingsInit();
    midiIconInit();
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

    colourSelectorButton.onClick = [this] {
        showColourSelector();
    };
    headerPanel.addAndMakeVisible(colourSelectorButton);
    colourSelectorButton.setVisible(false);
}

void MainComponent::instrumentSelectorButtonInit()
{
    instrumentSelectorButton.setButtonText("Instruments");

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
        recordPlayer.handleIncomingMessage(juce::MidiMessage::programChange(1, midiHandler.getProgramNumber()));

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
        midiHandler.setProgramNumber(recordPlayer.getProgram());
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
    display = std::make_unique<Display>();
    headerPanel.addAndMakeVisible(display.get());
    display->setVisible(false);
}

void MainComponent::keyBoardUIinit(int min, int max)
{
    keyboardInitialized = true;
    addAndMakeVisible(keyboard);
    int keyboardHeight = (int)getHeight() * 0.2;

    //keyboard.setBounds(0, getHeight() - 200, getWidth(), 200);
    keyboard.setBounds(0, getHeight() - keyboardHeight, getWidth(), keyboardHeight);

    this->keyboard.set_min_and_max(min, max);
    noteLayer = std::make_unique<NoteLayer>(this->keyboard);
    midiHandler.addListener(noteLayer.get());
    //noteLayer->setBounds(0, 50, getWidth(), getHeight() - 200 - 50);
    noteLayer->setBounds(0, 50, getWidth(), getHeight() - keyboardHeight - 50);
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
    knobsInit();
}

void MainComponent::homeButtonInit()
{
    homeButton.setButtonText("Home");

    homeButton.onClick = [this] {
        homeButtonOnClick();
    };

    headerPanel.addAndMakeVisible(homeButton);
    homeButton.setVisible(false);

}

void MainComponent::settingsButtonOnClick()
{
    toggleSettingsPanel(); toggleMIDIButton(); toggleMIDIsettingsIcon();
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
    toggleSettingsButton();
    keyboard.setVisible(false);
    this->noteLayer->setVisible(false);
    this->noteLayer->resetState();
    if(this->recordPlayer.getIsPlaying())
        this->recordPlayer.stopPlayBack();
}

void MainComponent::playButtonOnClick()
{
    MIDIDevice.getAvailableDevicesMidiIN(devicesIN);
    MIDIDevice.getAvailableDevicesMidiOUT(devicesOUT);
    if (openingDevicesForPlay()) {
        midiHandler.handlePlayableRange(MIDIDevice.extractVID(MIDIDevice.get_identifier()), MIDIDevice.extractPID(MIDIDevice.get_identifier()));
        this->recordPlayer.setOutputDevice(MIDIDevice.getDeviceOUT());
        if(midiWindow)
            this->midiWindow->setVisible(false);
        currentBackground = playBackground;
        repaint();
        toggleHPanel();
        MIDIDevice.changeVolumeInstrument();
        MIDIDevice.changeReverbInstrument();
        recordPlayer.setReverb(MIDIDevice.getReverb());

        if (!keyboardInitialized)
        {
            keyBoardUIinit(MIDIDevice.get_minNote(), MIDIDevice.get_maxNote());
            midiHandler.setProgramNumber(0);
            recordPlayer.setProgarmNumber(0);
        }
        else {
            keyboard.setVisible(true);
            this->noteLayer->setVisible(true);
        }

        if (this->keyListener.getIsKeyboardInput())
            this->keyboard.set_min_and_max(keyListener.getStartNoteKeyboardInput(), keyListener.getFinishNoteKeyboardInput());
        else
            this->keyboard.set_min_and_max(MIDIDevice.get_minNote(), MIDIDevice.get_maxNote());

        //DBG("Min note:" << this->keyboard.get_min());
        //DBG("Max note:" << this->keyboard.get_max());
        keyboard.setIsDrawn(false);
        keyboard.repaint();
        toggleForPlaying();
    }
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
        midiHandler.setProgramNumber(programNumber,name); //setting the actual sound to the new program for both midiHandler and also the recordPlayer by listener
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
        this->deviceOpenedIN = &this->MIDIDevice.getDeviceIN();
    }
    result = this->MIDIDevice.deviceOpenOUT(indexOUT);
    if (!result)
    {
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
    auto area1 = juce::Rectangle<int>(0, 0, 500, 50);
    auto area = juce::Rectangle<int>(colourSelectorButton.getX() - 25, 0, 500, getHeight() - 50 - 200);
    this->keyboard.setIsDrawn(false);
    juce::CallOutBox::launchAsynchronously(std::unique_ptr<juce::ColourSelector>(colourSelector), area1, this);
    this->keyboard.repaint();
    noteLayer->resetState();
    noteLayer->repaint();
    noteLayer->setVisible(false);
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
    auto area1 = juce::Rectangle<int>(0, 0, 500, 50);
    
    auto holder = std::make_unique<TreeViewHolder>(treeView.get());
    holder->setOpaque(true);
    this->keyboard.setIsDrawn(false);
    juce::CallOutBox::launchAsynchronously(std::move(holder), area1, this);
    this->keyboard.repaint();
    noteLayer->resetState();
    noteLayer->repaint();
    noteLayer->setVisible(false);
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
