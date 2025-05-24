#include "MainComponent.h"

//==============================================================================
MainComponent::MainComponent()
{
    settingsInit();
    playButtonInit();
    togglePlayButton();
    initalizeSaveFileForUser();
    loadSettings();
    cachedImageMainWindow = juce::ImageFileFormat::loadFrom(BinaryData::MainWindow_png, BinaryData::MainWindow_pngSize);
    playBackground = juce::ImageFileFormat::loadFrom(BinaryData::playingBackground_png, BinaryData::playingBackground_pngSize);
    currentBackground = cachedImageMainWindow;

    setBounds(0, 0, juce::Desktop::getInstance().getDisplays().getPrimaryDisplay()->totalArea.getWidth(),
        juce::Desktop::getInstance().getDisplays().getPrimaryDisplay()->totalArea.getHeight() - 40);
    this->addKeyListener(&this->keyListener);

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
        DBG("Deleting root item dest main: " << treeView->getRootItem()->getUniqueName());
        treeView->deleteRootItem();
        treeView->deleteRootItem();
    }
    else
        DBG("No root item to delete destructor main");
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


    playButton.setBounds(getWidth() / 2 - 210, getHeight() / 2 + 170, 75, 35);
    settingsButton.setBounds(0, 0, 250, 70);
    settingsPanel.setBounds(0, 70, 250, getHeight());
    midiButton.setBounds(0, 5, 200, 50);
    helpIcon.setBounds(210, 20, 100, 20);
    headerPanel.setBounds(0, 0, getWidth(), 50);
    homeButton.setBounds(10, 10, 75, 30);
    colourSelectorButton.setBounds(90, 10, 100, 30);
    instrumentSelectorButton.setBounds(195, 10, 100, 30);
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
        DBG("Colour changed to: " << newColor.toString());
        this->noteLayer->setColourParticle(newColor);
    }
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
        double savedVolume = propertiesFile->getDoubleValue("midiVolume", 1.0);
        double savedReverb = propertiesFile->getDoubleValue("midiReverb", 1.0);
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
    settingsButton.onClick = [this] { toggleSettingsPanel(); toggleMIDIButton(); toggleMIDIsettingsIcon(); };
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
        MIDIDevice.getAvailableDevicesMidiIN(devicesIN);
        MIDIDevice.getAvailableDevicesMidiOUT(devicesOUT);
        if (openingDevicesForPlay()) {
            //audioDeviceManager.initialiseWithDefaultDevices(0, 2);
            //audioProcessorPlayer.setProcessor(&revProcessor);
            //audioDeviceManager.addAudioCallback(&audioProcessorPlayer);
            midiHandler.handlePlayableRange(MIDIDevice.extractVID(MIDIDevice.get_identifier()), MIDIDevice.extractPID(MIDIDevice.get_identifier()));
            currentBackground = playBackground;
            repaint();
            headerPanelInit();
            toggleHPanel();
            inWhichState = false;
            MIDIDevice.changeVolumeInstrument();
            MIDIDevice.changeReverbInstrument();
            midiHandler.setProgramNumber(37);
            //float normalized = MIDIDevice.getReverb() / 100.0f;
            //midiHandler.setReverbAudioEng(normalized);
            if (!keyboardInitialized)
                keyBoardUIinit(MIDIDevice.get_minNote(), MIDIDevice.get_maxNote());
            else {
                keyboard.setVisible(true);
                this->noteLayer->setVisible(true);
            }
            toggleForPlaying();
        }
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

void MainComponent::keyBoardUIinit(int min, int max)
{
    keyboardInitialized = true;
    addAndMakeVisible(keyboard);
    keyboard.setBounds(0, getHeight() - 200, getWidth(), 200);

    this->keyboard.set_min_and_max(min, max);
    noteLayer = std::make_unique<NoteLayer>(this->keyboard);
    noteLayer->setBounds(0, 50, getWidth(), getHeight() - 200 - 50);
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
        if (!midiWindow)
        {
            midiWindow =  std::make_unique<MIDIWindow>(this->MIDIDevice,devicesIN,devicesOUT,propertiesFile);
            loadSettings();
        }
        else {
            midiWindow->setVisible(true);
            midiWindow->toFront(true);
        }
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
}

void MainComponent::homeButtonInit()
{
    homeButton.setButtonText("Home");

    homeButton.onClick = [this] {
        currentBackground = cachedImageMainWindow;
        repaint();

        toggleHPanel();
        toggleHomeButton();
        inWhichState = true;
        togglePlayButton();
        toggleSettingsButton();
        keyboard.setVisible(false);
        this->noteLayer->setVisible(false);
        this->noteLayer->resetState();
    };

    headerPanel.addAndMakeVisible(homeButton);
    homeButton.setVisible(false);

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
    
    dummyRoot->setOpen(true);
    treeView->setRootItem(dummyRoot.release());
    
}

InstrumentTreeItem* MainComponent::createInstrumentItem(const juce::Image& img, const juce::String& name, int program)
{
    auto item = std::make_unique<InstrumentTreeItem>(img,name,program);
    item->onProgramSelected = [&](int programNumber)
    {
        midiHandler.setProgramNumber(programNumber);
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
        this->MIDIDevice.set_minNote(60);
        this->MIDIDevice.set_maxNote(72);
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
        DBG("deleting root from show");
    }
    else DBG("not deleting root from show");
    buildTree();
    treeView->setColour(juce::TreeView::backgroundColourId, juce::Colours::white);
    auto area1 = juce::Rectangle<int>(0, 0, 500, 50);
    
    auto holder = std::make_unique<TreeViewHolder>(treeView.get());
    holder->setOpaque(true);
    this->keyboard.setIsDrawn(false);
    juce::CallOutBox::launchAsynchronously(std::move(holder), area1, this);
    noteLayer->resetState();
    noteLayer->repaint();
    noteLayer->setVisible(false);
}
