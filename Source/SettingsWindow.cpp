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

MIDIWindow::MIDIWindow(MidiDevice& mdevice, std::vector<std::string>& devicesListIN, std::vector<std::string>& devicesListOUT
    , std::vector<std::string>& devicesListAudioOUT, juce::PropertiesFile* prop)
    : juce::DocumentWindow{ "MIDI Settings", background, DocumentWindow::closeButton },
    MIDIDevice{ mdevice }, devicesListIN{ devicesListIN }, devicesListOUT{ devicesListOUT }, devicesListAudioOUT{ devicesListAudioOUT }, propertyFile{ prop }
{
    setUsingNativeTitleBar(false);
    setResizable(false, false);
    setTitleBarTextCentred(true);
    setColour(juce::DocumentWindow::backgroundColourId, background);
    setColour(juce::DocumentWindow::textColourId, titleText);
    setBounds(200, 150, 480, 250);
    setVisible(true);
    setContentOwned(&settingsPanel, false);

    addKeyListener(this);
    setBounds_components();
    allInit();
    toggleSettingsAll();
    populateCBIN();
    populateCBOUT();
    populateCBaudioOUT();
	populateCBengine();
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

void MIDIWindow::visibilityChanged()
{
    if (isVisible())
    {
        populateCBIN();  //selected id is set to 1 implicitly
        populateCBOUT(); //selected id is set to 1 implicitly
        restoreCBoxes();
        startTimer(750);
        //grabKeyboardFocus();
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
    else if (comboBoxThatHasChanged == &this->comboBoxOutputEngineType)
    {
		int selectedIndex = comboBoxOutputEngineType.getSelectedId();
		switchBetweenEngineOptions(selectedIndex);
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

    std::vector<std::string> devicesNewAudioOUT;
    MIDIDevice.getAvailableAudioDevicesOUT(devicesNewAudioOUT);
    if (devicesNewAudioOUT != devicesListAudioOUT)
    {
        if (comboBoxAudioDevicesOUT.isPopupActive())
            comboBoxAudioDevicesOUT.hidePopup();

        devicesListAudioOUT = devicesNewAudioOUT;
        populateCBaudioOUT();
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
	toggleSettingsInMIDICB();
	toggleSettingsOutMIDICB();
    toggleSettingsOutputEngineCB();
}

void MIDIWindow::toggleSettingsInMIDICB()
{
    if (this->comboBoxDevicesIN.isVisible()) {
        this->comboBoxDevicesIN.setVisible(false);
        this->midiDevicesLabelIN.setVisible(false);
    }
    else {
        this->midiDevicesLabelIN.setVisible(true);
        this->comboBoxDevicesIN.setVisible(true);
    }
}

void MIDIWindow::toggleSettingsOutMIDICB()
{
    if (this->comboBoxDevicesOUT.isVisible()) {
        this->comboBoxDevicesOUT.setVisible(false);
        this->midiDevicesLabelOUT.setVisible(false);
    }
    else {
        this->comboBoxDevicesOUT.setVisible(true);
        this->midiDevicesLabelOUT.setVisible(true);
    }
}

void MIDIWindow::toggleSettingsOutputEngineCB()
{
    if (this->comboBoxOutputEngineType.isVisible())
    {
        this->comboBoxOutputEngineType.setVisible(false);
        this->outputEngineTypeLabel.setVisible(false);
    }
    else
    {
        this->comboBoxOutputEngineType.setVisible(true);
        this->outputEngineTypeLabel.setVisible(true);
    }
}

void MIDIWindow::toggleSettingsAudioOutCB()
{
    if (this->comboBoxAudioDevicesOUT.isVisible())
    {
        this->comboBoxAudioDevicesOUT.setVisible(false);
        this->audioDevicesLabelOUT.setVisible(false);
    }
    else
    {
        this->comboBoxAudioDevicesOUT.setVisible(true);
        this->audioDevicesLabelOUT.setVisible(true);
    }
}

void MIDIWindow::toggleSettingsAll()
{
    toggleSettingsPanel();
    toggleSettingsCB();
}

void MIDIWindow::setBounds_components()
{
    const int pad = 16;
    const int rowH = 36;
    int y = pad;

    const int labelW = 80;
    const int cbX = 100;
    const int cbW = 260;


    outputEngineTypeLabel.setBounds(pad, y + 8, labelW, 20);
    comboBoxOutputEngineType.setBounds(cbX, y, 180, rowH);

    y += 55;


    midiDevicesLabelIN.setBounds(pad, y + 8, labelW, 20);
    comboBoxDevicesIN.setBounds(cbX, y, cbW, rowH);

    y += 46;

    midiDevicesLabelOUT.setBounds(pad, y + 8, labelW, 20);
    comboBoxDevicesOUT.setBounds(cbX, y, cbW, rowH);

	audioDevicesLabelOUT.setBounds(pad, y + 8, labelW, 20);
	comboBoxAudioDevicesOUT.setBounds(cbX, y, cbW, rowH);
}

void MIDIWindow::panelInit()
{
    settingsPanel.setSize(480, 250);

    setBackgroundColour(panelBg);

    setContentOwned(&settingsPanel, true);
    settingsPanel.setVisible(false);
}

void MIDIWindow::devicesCBinit()
{
	deviceInMIDICBinit();
	deviceOutMIDICBinit();
	deviceAudioOutCBinit();
}

void MIDIWindow::deviceInMIDICBinit()
{
    comboBoxDevicesIN.addListener(this);
    setStyleComboBox(comboBoxDevicesIN);
    settingsPanel.addAndMakeVisible(this->comboBoxDevicesIN);
    comboBoxDevicesIN.setVisible(false);

    midiDevicesLabelIN.setText("MIDI IN", juce::dontSendNotification);
    setStyleLabel(midiDevicesLabelIN);
    settingsPanel.addAndMakeVisible(midiDevicesLabelIN);
    midiDevicesLabelIN.setVisible(false);
}

void MIDIWindow::deviceOutMIDICBinit()
{
    comboBoxDevicesOUT.addListener(this);
    setStyleComboBox(comboBoxDevicesOUT);
    settingsPanel.addAndMakeVisible(this->comboBoxDevicesOUT);
    comboBoxDevicesOUT.setVisible(false);

    midiDevicesLabelOUT.setText("MIDI OUT", juce::dontSendNotification);
    setStyleLabel(midiDevicesLabelOUT);
    settingsPanel.addAndMakeVisible(this->midiDevicesLabelOUT);
    midiDevicesLabelOUT.setVisible(false);
}

void MIDIWindow::outputEngineCBinit()
{
    comboBoxOutputEngineType.addListener(this);
	setStyleComboBox(comboBoxOutputEngineType);
	settingsPanel.addAndMakeVisible(this->comboBoxOutputEngineType);
	comboBoxOutputEngineType.setVisible(false);

	outputEngineTypeLabel.setText("Output Engine", juce::dontSendNotification);
	setStyleLabel(outputEngineTypeLabel);
	settingsPanel.addAndMakeVisible(this->outputEngineTypeLabel);
	outputEngineTypeLabel.setVisible(false);
}

void MIDIWindow::deviceAudioOutCBinit()
{
	comboBoxAudioDevicesOUT.addListener(this);
	setStyleComboBox(comboBoxAudioDevicesOUT);
	settingsPanel.addAndMakeVisible(this->comboBoxAudioDevicesOUT);
	comboBoxAudioDevicesOUT.setVisible(false);

	audioDevicesLabelOUT.setText("Audio OUT", juce::dontSendNotification);
	setStyleLabel(audioDevicesLabelOUT);
	settingsPanel.addAndMakeVisible(this->audioDevicesLabelOUT);
	audioDevicesLabelOUT.setVisible(false);
}

void MIDIWindow::allInit()
{
    panelInit();
    devicesCBinit();
    outputEngineCBinit();
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

void MIDIWindow::populateCBengine()
{
    this->comboBoxOutputEngineType.clear();
    int id = 1;
    for (const auto& engine : { "External MIDI", "Internal SFZ Synth" })
    {
        this->comboBoxOutputEngineType.addItem(juce::String(engine), id);
        id++;
    }
    comboBoxOutputEngineType.setSelectedId(1, juce::dontSendNotification);
}

void MIDIWindow::populateCBaudioOUT()
{
    MIDIDevice.getAvailableAudioDevicesOUT(this->devicesListAudioOUT);

    comboBoxAudioDevicesOUT.clear();

    int id = 1;
    for (const auto& device : devicesListAudioOUT)
    {
        comboBoxAudioDevicesOUT.addItem(juce::String(device), id);
		id++;
    }

    comboBoxAudioDevicesOUT.setSelectedId(1, juce::dontSendNotification);
}


void MIDIWindow::restoreCBoxes()
{
    if(lastIndexIN<=comboBoxDevicesIN.getNumItems())
        comboBoxDevicesIN.setSelectedId(lastIndexIN);
    if(lastIndexOUT<=comboBoxDevicesOUT.getNumItems())
        comboBoxDevicesOUT.setSelectedId(lastIndexOUT);
    if (lastIndexEngineOption <= comboBoxOutputEngineType.getNumItems())
        comboBoxOutputEngineType.setSelectedId(lastIndexEngineOption);
}

void MIDIWindow::setStyleComboBox(juce::ComboBox& cb)
{
    cb.setColour(juce::ComboBox::backgroundColourId, cardBg);
    cb.setColour(juce::ComboBox::outlineColourId, separator);
    cb.setColour(juce::ComboBox::textColourId, titleText);
    cb.setColour(juce::ComboBox::arrowColourId, accent1);
    cb.setMouseCursor(juce::MouseCursor::PointingHandCursor);
}

void MIDIWindow::setStyleLabel(juce::Label& label)
{
    label.setFont(juce::FontOptions(13.0f));
    label.setColour(juce::Label::textColourId, accent1);
}

void MIDIWindow::switchBetweenEngineOptions(int selectedId)
{
    if (selectedId == 1)
    {
		toggleSettingsOutMIDICB();
		toggleSettingsAudioOutCB();
    }
    else if(selectedId == 2)
    {
        toggleSettingsOutMIDICB();
        toggleSettingsAudioOutCB();
	}
}
