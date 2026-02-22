#include "SettingsWindow.h"
#include "MidiHandler.h"

MIDIWindow::MIDIWindow(MidiDevice& mdevice, std::vector<std::string>& devicesListIN, std::vector<std::string>& devicesListOUT, juce::PropertiesFile* prop)
    : juce::DocumentWindow{ "MIDI Settings", juce::Colours::darkgrey, DocumentWindow::closeButton },
    MIDIDevice{ mdevice }, devicesListIN{ devicesListIN }, devicesListOUT{ devicesListOUT }, propertyFile{ prop }
{
    setUsingNativeTitleBar(false);
    setResizable(false, false);
    setBounds(200, 150, 450, 300);
    setVisible(true);
    setContentOwned(&settingsPanel, false);

    addKeyListener(this);
    setBounds_components();
    allInit();
    toggleSettingsAll();
    populateCBIN();
    populateCBOUT();
    populateCBinstruments();
}

MIDIWindow::~MIDIWindow()
{
    stopTimer();
    removeKeyListener(this);
}


bool MIDIWindow::keyPressed(const juce::KeyPress& key, juce::Component* comp)
{
    if (key == juce::KeyPress::escapeKey)
    {
        closeButtonPressed();
        return true;          
    }
    return false;
}

void MIDIWindow::volumeSliderSetValue(double value)
{
    this->volumeSlider.setValue(value);
}

void MIDIWindow::reverbSliderSetValue(double value)
{
    this->reverbSlider.setValue(value);
}

void MIDIWindow::visibilityChanged()
{
    if (isVisible())
    {
        populateCBIN();  //selected id is set to 1 implicitly
        populateCBOUT(); //selected id is set to 1 implicitly
        restoreCBoxes();
        startTimer(750);
        grabKeyboardFocus();
    }
    else stopTimer();

}

void MIDIWindow::closeButtonPressed()
{

    setVisible(false);

    if (onWindowClosed)
        onWindowClosed();
}

void MIDIWindow::comboBoxChanged(juce::ComboBox* comboBoxThatHasChanged)
{
    if (comboBoxThatHasChanged == &this->comboBoxDevicesIN)
    {
        int index = comboBoxDevicesIN.getSelectedId() - 1;
        if (index >= 0 && index < this->devicesListIN.size())
        {
            this->MIDIDevice.setDeviceIN(index);
            lastIndexIN = index + 1;
        }
    }
    else if (comboBoxThatHasChanged == &this->comboBoxDevicesOUT)
    {
        int index = comboBoxDevicesOUT.getSelectedId() - 1;
        if (index >= 0 && index < this->devicesListOUT.size())
        {
            this->MIDIDevice.setDeviceOUT(index);
            lastIndexOUT = index + 1;
        }
    }
    else if (comboBoxThatHasChanged == &this->currentInstrumentSettingsCB)
    {
        int channel = getCorrectChannel(currentInstrumentSettingsCB.getText());
        
        volumeSliderSetValue(MIDIDevice.getVolume(channel));
        reverbSliderSetValue(MIDIDevice.getReverb(channel));
    }
}

void MIDIWindow::timerCallback()
{
    std::vector<std::string> devicesNewIN;
    this->MIDIDevice.getAvailableDevicesMidiIN(devicesNewIN);
    if (devicesNewIN != this->devicesListIN)
    {
        if (this->comboBoxDevicesIN.isPopupActive())
            this->comboBoxDevicesIN.hidePopup();
        devicesListIN = devicesNewIN;
        populateCBIN();
    }
    std::vector<std::string> devicesNewOUT;
    this->MIDIDevice.getAvailableDevicesMidiOUT(devicesNewOUT);
    if (devicesNewOUT != this->devicesListOUT)
    {
        if (this->comboBoxDevicesOUT.isPopupActive())
            this->comboBoxDevicesOUT.hidePopup();
        devicesListOUT = devicesNewOUT;
        populateCBOUT();
    }


}

int MIDIWindow::getCorrectChannel(juce::String& text)
{
    if (text.equalsIgnoreCase("First instrument"))
        return 1;
    else if (text.equalsIgnoreCase("Second instrument"))
        return 16;

    return 1;
}

void MIDIWindow::toggleSettingsSliders()
{
    if (this->reverbSlider.isVisible()) {
        reverbSlider.setVisible(false);
        reverbLabel.setVisible(false);
    }
    else {
        reverbSlider.setVisible(true);
        reverbLabel.setVisible(true);
    }
    if (this->volumeSlider.isVisible()) {
        volumeSlider.setVisible(false);
        volumeLabel.setVisible(false);
    }
    else {
        volumeSlider.setVisible(true);
        volumeLabel.setVisible(true);
    }
}

void MIDIWindow::toggleSettingsPanel()
{
    if (this->settingsPanel.isVisible())
        this->settingsPanel.setVisible(false);
    else this->settingsPanel.setVisible(true);
}

void MIDIWindow::toggleSettingsCB()
{
    if (this->comboBoxDevicesIN.isVisible()) {
        this->comboBoxDevicesIN.setVisible(false);
        this->midiDevicesLabelIN.setVisible(false);
    }
    else {
        this->midiDevicesLabelIN.setVisible(true);
        this->comboBoxDevicesIN.setVisible(true);
    }
    if (this->comboBoxDevicesOUT.isVisible()) {
        this->comboBoxDevicesOUT.setVisible(false);
        this->midiDevicesLabelOUT.setVisible(false);
    }
    else {
        this->comboBoxDevicesOUT.setVisible(true);
        this->midiDevicesLabelOUT.setVisible(true);
    }

    if (this->currentInstrumentSettingsCB.isVisible())
        this->currentInstrumentSettingsCB.setVisible(false);
    else this->currentInstrumentSettingsCB.setVisible(true);
}

void MIDIWindow::toggleSettingsAll()
{
    toggleSettingsPanel();
    toggleSettingsSliders();
    toggleSettingsCB();
}

void MIDIWindow::setBounds_components()
{
    int spacing = 20;

    currentInstrumentSettingsCB.setBounds(10, 20, 150, 50);

    reverbSlider.setBounds(10, currentInstrumentSettingsCB.getBottom()+spacing/2, 200, 30);
    reverbLabel.setBounds(reverbSlider.getRight()+spacing, reverbSlider.getY()+spacing/2, 100, 20);

    volumeSlider.setBounds(10, reverbSlider.getBottom()+spacing/2, 200, 30);
    volumeLabel.setBounds(volumeSlider.getRight()+spacing, volumeSlider.getY() + spacing/2, 100, 20);

    comboBoxDevicesIN.setBounds(90, volumeSlider.getBottom()+spacing/2, 300, 50);
    midiDevicesLabelIN.setBounds(10, comboBoxDevicesIN.getY()+spacing/2, 90, 20);

    comboBoxDevicesOUT.setBounds(90, comboBoxDevicesIN.getBottom()+spacing/2, 300, 50);
    midiDevicesLabelOUT.setBounds(10, comboBoxDevicesOUT.getY()+spacing/2, 100, 20);


}

void MIDIWindow::panelInit()
{
    settingsPanel.setSize(400, 300);
    setContentOwned(&settingsPanel, true);
    settingsPanel.setVisible(false);
}

void MIDIWindow::slidersInit()
{
    reverbSlider.setRange(0.0, 127.0, 1.0);
    reverbSlider.setValue(50.0);
    reverbSlider.setTextValueSuffix(" %");
    settingsPanel.addAndMakeVisible(reverbSlider);
    reverbSlider.setVisible(false);

    reverbLabel.setText("Reverb", juce::dontSendNotification);
    settingsPanel.addAndMakeVisible(reverbLabel);
    reverbLabel.setVisible(false);


    volumeSlider.setRange(0.0, 127.0, 1.0);
    volumeSlider.setValue(50.0);
    volumeSlider.setTextValueSuffix(" %");

    settingsPanel.addAndMakeVisible(volumeSlider);
    volumeSlider.setVisible(false);

    volumeLabel.setText("Volume scale", juce::dontSendNotification);
    settingsPanel.addAndMakeVisible(volumeLabel);
    volumeLabel.setVisible(false);

    reverbSlider.setMouseCursor(juce::MouseCursor::PointingHandCursor);
    volumeSlider.setMouseCursor(juce::MouseCursor::PointingHandCursor);

    this->volumeSlider.onValueChange = [this] {

        int channel = getCorrectChannel(currentInstrumentSettingsCB.getText());

        if (propertyFile)
        {
            if (channel==1)
                propertyFile->setValue("midiVolumeFirst", volumeSlider.getValue());
            else propertyFile->setValue("midiVolumeSecond", volumeSlider.getValue());

            propertyFile->saveIfNeeded();
        }
        MIDIDevice.setVolume(volumeSlider.getValue(),channel);

        

        if (isMidiDeviceOpen && isMidiDeviceOpen())
        {
            MIDIDevice.changeVolumeInstrument(channel);
        }
    };

    this->reverbSlider.onValueChange = [this] {
        int channel = getCorrectChannel(currentInstrumentSettingsCB.getText());

        if (propertyFile)
        {
            if(channel==1)
                propertyFile->setValue("midiReverbFirst", reverbSlider.getValue());
            else if(channel==16)
                propertyFile->setValue("midiReverbSecond", reverbSlider.getValue());

            propertyFile->saveIfNeeded();
        }
        MIDIDevice.setReverb(reverbSlider.getValue(),channel);

        if (isMidiDeviceOpen && isMidiDeviceOpen())
        {
            MIDIDevice.changeReverbInstrument(channel);
        }
    };
}

void MIDIWindow::devicesCBinit()
{
    comboBoxDevicesIN.addListener(this);
    settingsPanel.addAndMakeVisible(this->comboBoxDevicesIN);
    comboBoxDevicesIN.setVisible(false);

    midiDevicesLabelIN.setText("MIDI IN", juce::dontSendNotification);
    settingsPanel.addAndMakeVisible(midiDevicesLabelIN);
    midiDevicesLabelIN.setVisible(false);

    comboBoxDevicesOUT.addListener(this);
    settingsPanel.addAndMakeVisible(this->comboBoxDevicesOUT);
    comboBoxDevicesOUT.setVisible(false);

    midiDevicesLabelOUT.setText("MIDI OUT", juce::dontSendNotification);
    settingsPanel.addAndMakeVisible(this->midiDevicesLabelOUT);
    midiDevicesLabelOUT.setVisible(false);

    comboBoxDevicesIN.setMouseCursor(juce::MouseCursor::PointingHandCursor);
    comboBoxDevicesOUT.setMouseCursor(juce::MouseCursor::PointingHandCursor);
}

void MIDIWindow::instrumentsCBinit()
{
    currentInstrumentSettingsCB.addListener(this);
    settingsPanel.addAndMakeVisible(this->currentInstrumentSettingsCB);
    currentInstrumentSettingsCB.setVisible(false);

    currentInstrumentSettingsCB.setMouseCursor(juce::MouseCursor::PointingHandCursor);
}

void MIDIWindow::allInit()
{
    panelInit();
    slidersInit();
    devicesCBinit();

    instrumentsCBinit();
}

void MIDIWindow::populateCBIN()
{
    this->MIDIDevice.getAvailableDevicesMidiIN(this->devicesListIN);
    this->comboBoxDevicesIN.clear();
    int id = 1;
    for (const auto& device : devicesListIN) {
        this->comboBoxDevicesIN.addItem(juce::String(device), id);
        id++;
    }
    comboBoxDevicesIN.setSelectedId(1);
}

void MIDIWindow::populateCBOUT()
{
    this->MIDIDevice.getAvailableDevicesMidiOUT(this->devicesListOUT);
    this->comboBoxDevicesOUT.clear();
    int id = 1;
    for (const auto& device : devicesListOUT) {
        this->comboBoxDevicesOUT.addItem(juce::String(device), id);
        id++;
    }
    comboBoxDevicesOUT.setSelectedId(1);

}

void MIDIWindow::populateCBinstruments()
{
    this->currentInstrumentSettingsCB.clear();

    this->currentInstrumentSettingsCB.addItem("First instrument", 1);
    this->currentInstrumentSettingsCB.addItem("Second instrument", 2);

    this->currentInstrumentSettingsCB.setSelectedId(1);
}

void MIDIWindow::restoreCBoxes()
{
    if(lastIndexIN<=comboBoxDevicesIN.getNumItems())
        comboBoxDevicesIN.setSelectedId(lastIndexIN);
    if(lastIndexOUT<=comboBoxDevicesOUT.getNumItems())
        comboBoxDevicesOUT.setSelectedId(lastIndexOUT);
}
