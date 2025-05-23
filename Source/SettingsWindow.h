#pragma once
#include <JuceHeader.h> 

class MidiDevice;

class MIDIWindow : public juce::DocumentWindow, public juce::ComboBox::Listener, public juce::Timer
{
public:
	MIDIWindow(MidiDevice& mdevice, std::vector<std::string>& devicesListIN, std::vector<std::string>& devicesListOUT, juce::PropertiesFile* prop);
	~MIDIWindow() override;
	void volumeSliderSetValue(double value);
	void reverbSliderSetValue(double value);

private:
	void visibilityChanged() override;
	void closeButtonPressed() override;
	void comboBoxChanged(juce::ComboBox* comboBoxThatHasChanged) override;
	void timerCallback() override;
	void toggleSettingsSliders();
	void toggleSettingsPanel();
	void toggleSettingsCB();
	void toggleSettingsAll();

	void setBounds_components();

	void panelInit();
	void slidersInit();
	void devicesCBinit();
	void allInit();

	void populateCBIN();
	void populateCBOUT();
	void restoreCBoxes();

	//Back function so i can reopen the window


	MidiDevice& MIDIDevice;
	std::vector<std::string>& devicesListIN;
	std::vector<std::string>& devicesListOUT;
	juce::PropertiesFile* propertyFile;

	juce::Slider reverbSlider;
	juce::Label reverbLabel;
	juce::Slider volumeSlider;
	juce::Label volumeLabel;
	juce::Component settingsPanel;

	juce::Label midiDevicesLabelIN;
	juce::Label midiDevicesLabelOUT;
	juce::ComboBox comboBoxDevicesIN;
	juce::ComboBox comboBoxDevicesOUT;
	int lastIndexIN=0;
	int lastIndexOUT=0;
};