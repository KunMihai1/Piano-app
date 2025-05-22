#include "SettingsWindow.h"
#include "MidiHandler.h"

MIDIWindow::MIDIWindow(MidiDevice& mdevice, std::vector<std::string>& devicesListIN, std::vector<std::string>& devicesListOUT)
    : juce::DocumentWindow{ "MIDI Settings", juce::Colours::darkgrey, DocumentWindow::closeButton },
    MIDIDevice{ mdevice }, devicesListIN{ devicesListIN }, devicesListOUT{devicesListOUT}
{
    setUsingNativeTitleBar(false);
    setResizable(false, false);
    setBounds(200, 150, 450, 300);
    setVisible(true);
    setContentOwned(&settingsPanel, false);

    setBounds_components();
    allInit();
    toggleSettingsAll();
    populateCBIN();
    populateCBOUT();
}

MIDIWindow::~MIDIWindow()
{
    stopTimer();
}

void MIDIWindow::visibilityChanged()
{
    if (isVisible())
    {
        populateCBIN();  //selected id is set to 1 implicitly
        populateCBOUT(); //selected id i set to 1 implicitly
        restoreCBoxes();
        startTimer(750);
    }
    else stopTimer();

}

void MIDIWindow::closeButtonPressed()
{
    setVisible(false);
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
}

void MIDIWindow::toggleSettingsAll()
{
    toggleSettingsPanel();
    toggleSettingsSliders();
    toggleSettingsCB();
}

void MIDIWindow::setBounds_components()
{
    reverbSlider.setBounds(10, 20, 200, 30);
    reverbLabel.setBounds(220, 25, 100, 20);

    volumeSlider.setBounds(10, 50, 200, 30);
    volumeLabel.setBounds(220, 55, 100, 20);

    comboBoxDevicesIN.setBounds(90, 100, 300, 50);
    midiDevicesLabelIN.setBounds(10, 115, 90, 20);

    comboBoxDevicesOUT.setBounds(90, 160, 300, 50);
    midiDevicesLabelOUT.setBounds(10, 175, 100, 20);


}

void MIDIWindow::panelInit()
{
    settingsPanel.setSize(400, 300);
    setContentOwned(&settingsPanel, true);
    settingsPanel.setVisible(false);
}

void MIDIWindow::slidersInit()
{
    reverbSlider.setRange(0.0, 100.0, 1.0);
    reverbSlider.setValue(50.0);
    reverbSlider.setTextValueSuffix(" %");
    settingsPanel.addAndMakeVisible(reverbSlider);
    reverbSlider.setVisible(false);

    reverbLabel.setText("Reverb", juce::dontSendNotification);
    settingsPanel.addAndMakeVisible(reverbLabel);
    reverbLabel.setVisible(false);


    volumeSlider.setRange(0.0, 100.0, 1.0);
    volumeSlider.setValue(50.0);
    volumeSlider.setTextValueSuffix(" %");

    settingsPanel.addAndMakeVisible(volumeSlider);
    volumeSlider.setVisible(false);

    volumeLabel.setText("Volume scale", juce::dontSendNotification);
    settingsPanel.addAndMakeVisible(volumeLabel);
    volumeLabel.setVisible(false);

    this->volumeSlider.onValueChange = [this] {
        MIDIDevice.setVolume(volumeSlider.getValue());
    };

    this->reverbSlider.onValueChange = [this] {
        MIDIDevice.setReverb(reverbSlider.getValue());
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
}

void MIDIWindow::allInit()
{
    panelInit();
    slidersInit();
    devicesCBinit();
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

void MIDIWindow::restoreCBoxes()
{
    if(lastIndexIN<=comboBoxDevicesIN.getNumItems())
        comboBoxDevicesIN.setSelectedId(lastIndexIN);
    if(lastIndexOUT<=comboBoxDevicesOUT.getNumItems())
        comboBoxDevicesOUT.setSelectedId(lastIndexOUT);
}
