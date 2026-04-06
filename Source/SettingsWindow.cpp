#include "SettingsWindow.h"
#include "MidiHandler.h"
#include "AppColours.h"


using namespace AppColours;

//==============================================================================
//  Custom LookAndFeel for themed linear sliders
//==============================================================================
class MidiWindowSliderLnF : public juce::LookAndFeel_V4
{
public:
    MidiWindowSliderLnF()
    {
        setColour(juce::Slider::backgroundColourId,    knobTrack);
        setColour(juce::Slider::trackColourId,         accent1);
        setColour(juce::Slider::thumbColourId,         juce::Colours::white);
        setColour(juce::Slider::textBoxTextColourId,   titleText);
        setColour(juce::Slider::textBoxBackgroundColourId, cardBg);
        setColour(juce::Slider::textBoxOutlineColourId, separator);
    }

    void drawLinearSlider(juce::Graphics& g, int x, int y, int width, int height,
                          float sliderPos, float minSliderPos, float maxSliderPos,
                          juce::Slider::SliderStyle style, juce::Slider& slider) override
    {
        const float trackHeight = 6.0f;
        const float trackY = (float)y + ((float)height - trackHeight) * 0.5f;
        const float cornerSize = 3.0f;

        // Background track
        g.setColour(knobTrack);
        g.fillRoundedRectangle((float)x, trackY, (float)width, trackHeight, cornerSize);

        // Filled portion with gradient
        float filledWidth = sliderPos - (float)x;
        if (filledWidth > 0.0f)
        {
            juce::ColourGradient grad(accent1.darker(0.3f), (float)x, trackY,
                                      accent1, sliderPos, trackY, false);
            g.setGradientFill(grad);
            g.fillRoundedRectangle((float)x, trackY, filledWidth, trackHeight, cornerSize);
        }

        // Thumb glow
        {
            const float glowR = 12.0f;
            juce::ColourGradient glow(accent1.withAlpha(0.3f), sliderPos, trackY + trackHeight * 0.5f,
                                     accent1.withAlpha(0.0f), sliderPos + glowR, trackY + trackHeight * 0.5f + glowR, true);
            g.setGradientFill(glow);
            g.fillEllipse(sliderPos - glowR, trackY + trackHeight * 0.5f - glowR, glowR * 2.0f, glowR * 2.0f);
        }

        // Thumb circle
        const float thumbR = 7.0f;
        g.setColour(juce::Colours::white);
        g.fillEllipse(sliderPos - thumbR, trackY + trackHeight * 0.5f - thumbR,
                      thumbR * 2.0f, thumbR * 2.0f);

        // thin white border on thumb
        g.setColour(accent1.withAlpha(0.5f));
        g.drawEllipse(sliderPos - thumbR, trackY + trackHeight * 0.5f - thumbR,
                      thumbR * 2.0f, thumbR * 2.0f, 1.5f);
    }
};

static MidiWindowSliderLnF& getSliderLnF()
{
    static MidiWindowSliderLnF instance;
    return instance;
}

MIDIWindow::MIDIWindow(MidiDevice& mdevice, std::vector<std::string>& devicesListIN, std::vector<std::string>& devicesListOUT, juce::PropertiesFile* prop)
    : juce::DocumentWindow{ "MIDI Settings", background, DocumentWindow::closeButton },
    MIDIDevice{ mdevice }, devicesListIN{ devicesListIN }, devicesListOUT{ devicesListOUT }, propertyFile{ prop }
{
    setUsingNativeTitleBar(false);
    setResizable(false, false);
    setTitleBarTextCentred(true);
    setColour(juce::DocumentWindow::backgroundColourId, background);
    setColour(juce::DocumentWindow::textColourId, titleText);
    setBounds(200, 150, 480, 380);
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
    volumeSlider.setLookAndFeel(nullptr);
    reverbSlider.setLookAndFeel(nullptr);
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
    const int pad = 16;
    const int rowH = 36;
    int y = pad;

    // Instrument selector — full width
    currentInstrumentSettingsCB.setBounds(pad, y, 200, rowH);
    y = currentInstrumentSettingsCB.getBottom() + pad;

    // --- Sliders section ---
    const int sliderW = 260;
    const int labelW  = 100;
    const int labelGap = 12;

    volumeSlider.setBounds(pad, y, sliderW, rowH);
    volumeLabel.setBounds(volumeSlider.getRight() + labelGap, y + 8, labelW, 20);
    y = volumeSlider.getBottom() + 10;

    reverbSlider.setBounds(pad, y, sliderW, rowH);
    reverbLabel.setBounds(reverbSlider.getRight() + labelGap, y + 8, labelW, 20);
    y = reverbSlider.getBottom() + pad + 4;

    // --- Devices section ---
    const int cbX = 80;
    const int cbW = 480 - cbX - pad * 2;  // fill remaining width

    midiDevicesLabelIN.setBounds(pad, y + 10, 60, 20);
    comboBoxDevicesIN.setBounds(cbX, y, cbW, rowH);
    y = comboBoxDevicesIN.getBottom() + 10;

    midiDevicesLabelOUT.setBounds(pad, y + 10, 70, 20);
    comboBoxDevicesOUT.setBounds(cbX, y, cbW, rowH);
}

void MIDIWindow::panelInit()
{
    settingsPanel.setSize(480, 380);

    setBackgroundColour(panelBg);

    setContentOwned(&settingsPanel, true);
    settingsPanel.setVisible(false);
}

void MIDIWindow::slidersInit()
{
    // --- Reverb slider ---
    reverbSlider.setRange(0.0, 127.0, 1.0);
    reverbSlider.setValue(50.0);
    reverbSlider.setTextValueSuffix(" %");
    reverbSlider.setLookAndFeel(&getSliderLnF());
    reverbSlider.setColour(juce::Slider::textBoxTextColourId, titleText);
    reverbSlider.setColour(juce::Slider::textBoxBackgroundColourId, cardBg);
    reverbSlider.setColour(juce::Slider::textBoxOutlineColourId, separator);
    settingsPanel.addAndMakeVisible(reverbSlider);
    reverbSlider.setVisible(false);

    reverbLabel.setText("Reverb", juce::dontSendNotification);
    reverbLabel.setFont(juce::FontOptions(14.0f));
    reverbLabel.setColour(juce::Label::textColourId, subtitleText);
    settingsPanel.addAndMakeVisible(reverbLabel);
    reverbLabel.setVisible(false);

    // --- Volume slider ---
    volumeSlider.setRange(0.0, 127.0, 1.0);
    volumeSlider.setValue(50.0);
    volumeSlider.setTextValueSuffix(" %");
    volumeSlider.setLookAndFeel(&getSliderLnF());
    volumeSlider.setColour(juce::Slider::textBoxTextColourId, titleText);
    volumeSlider.setColour(juce::Slider::textBoxBackgroundColourId, cardBg);
    volumeSlider.setColour(juce::Slider::textBoxOutlineColourId, separator);
    settingsPanel.addAndMakeVisible(volumeSlider);
    volumeSlider.setVisible(false);

    volumeLabel.setText("Volume", juce::dontSendNotification);
    volumeLabel.setFont(juce::FontOptions(14.0f));
    volumeLabel.setColour(juce::Label::textColourId, subtitleText);
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
    auto styleCombo = [](juce::ComboBox& cb)
    {
        cb.setColour(juce::ComboBox::backgroundColourId, cardBg);
        cb.setColour(juce::ComboBox::outlineColourId,    separator);
        cb.setColour(juce::ComboBox::textColourId,       titleText);
        cb.setColour(juce::ComboBox::arrowColourId,      accent1);
        cb.setMouseCursor(juce::MouseCursor::PointingHandCursor);
    };

    auto styleLabel = [](juce::Label& lbl)
    {
        lbl.setFont(juce::FontOptions(13.0f));
        lbl.setColour(juce::Label::textColourId, accent1);
    };

    comboBoxDevicesIN.addListener(this);
    styleCombo(comboBoxDevicesIN);
    settingsPanel.addAndMakeVisible(this->comboBoxDevicesIN);
    comboBoxDevicesIN.setVisible(false);

    midiDevicesLabelIN.setText("MIDI IN", juce::dontSendNotification);
    styleLabel(midiDevicesLabelIN);
    settingsPanel.addAndMakeVisible(midiDevicesLabelIN);
    midiDevicesLabelIN.setVisible(false);

    comboBoxDevicesOUT.addListener(this);
    styleCombo(comboBoxDevicesOUT);
    settingsPanel.addAndMakeVisible(this->comboBoxDevicesOUT);
    comboBoxDevicesOUT.setVisible(false);

    midiDevicesLabelOUT.setText("MIDI OUT", juce::dontSendNotification);
    styleLabel(midiDevicesLabelOUT);
    settingsPanel.addAndMakeVisible(this->midiDevicesLabelOUT);
    midiDevicesLabelOUT.setVisible(false);
}

void MIDIWindow::instrumentsCBinit()
{
    currentInstrumentSettingsCB.addListener(this);
    currentInstrumentSettingsCB.setColour(juce::ComboBox::backgroundColourId, cardBg);
    currentInstrumentSettingsCB.setColour(juce::ComboBox::outlineColourId,    separator);
    currentInstrumentSettingsCB.setColour(juce::ComboBox::textColourId,       titleText);
    currentInstrumentSettingsCB.setColour(juce::ComboBox::arrowColourId,      accent2);
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
