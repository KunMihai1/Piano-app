#include "MainComponent.h"
#include "MidiHandler.h"
#include "KeyboardUI.h"

//==============================================================================
MainComponent::MainComponent()
{
    settingsInit();
    playButtonInit();
    togglePlayButton();
    cachedImageMainWindow = juce::ImageFileFormat::loadFrom(BinaryData::MainWindow_png, BinaryData::MainWindow_pngSize);
    playBackground = juce::ImageFileFormat::loadFrom(BinaryData::playingBackground_png, BinaryData::playingBackground_pngSize);
    currentBackground = cachedImageMainWindow;

    setBounds(0, 0, juce::Desktop::getInstance().getDisplays().getPrimaryDisplay()->totalArea.getWidth(),
        juce::Desktop::getInstance().getDisplays().getPrimaryDisplay()->totalArea.getHeight() - 40);



    MIDIDevice.getAvailableDevicesMidiIN(this->devicesIN);
    MIDIDevice.getAvailableDevicesMidiOUT(this->devicesOUT);

}

MainComponent::~MainComponent()
{
    if (this->MIDIDevice.isOpenIN())
        this->MIDIDevice.deviceCloseIN();
    if (this->MIDIDevice.isOpenOUT())
        this->MIDIDevice.deviceCloseOUT();
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


    }

    g.setFont(juce::FontOptions(16.0f));
    g.setColour(juce::Colours::white);

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
}

void MainComponent::focusGained(FocusChangeType)
{
    resized();  // Force layout update
    repaint();  // Redraw everything
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
}

void MainComponent::toggleHomeButton()
{
    if (homeButton.isVisible())
        homeButton.setVisible(false);
    else homeButton.setVisible(true);
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
            midiHandler.handlePlayableRange(MIDIDevice.extractVID(MIDIDevice.get_identifier()), MIDIDevice.extractPID(MIDIDevice.get_identifier()));
            currentBackground = playBackground;
            repaint();
            headerPanelInit();
            toggleHPanel();
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
    //addAndMakeVisible(tooltipWindow);
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
            midiWindow = { std::make_unique<MIDIWindow>(this->MIDIDevice, this->devicesIN, this->devicesOUT) };
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
}

void MainComponent::homeButtonInit()
{
    homeButton.setButtonText("Home");

    homeButton.onClick = [this] {
        currentBackground = cachedImageMainWindow;
        repaint();

        toggleHPanel();
        toggleHomeButton();

        togglePlayButton();
        toggleSettingsButton();
        keyboard.setVisible(false);
        this->noteLayer->setVisible(false);
    };

    headerPanel.addAndMakeVisible(homeButton);
    homeButton.setVisible(false);

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



    result = this->MIDIDevice.deviceOpenIN(indexIN, &midiHandler);
    if (!result)
    {
        juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::InfoIcon, "ERROR", "Failed to open input device.", "OK");
        return false;
    }
    this->deviceOpenedIN = &this->MIDIDevice.getDeviceIN();
    result = this->MIDIDevice.deviceOpenOUT(indexOUT);
    if (!result)
    {
        juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::InfoIcon, "ERROR", "Failed to open output device.", "OK");
        return false;
    }
    this->deviceOpenedOUT = &this->MIDIDevice.getDeviceOUT();
    return true;
}
