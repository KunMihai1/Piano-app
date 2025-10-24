#pragma once
#include <JuceHeader.h> 

class MidiDevice;


/**
 * @class MIDIWindow
 * @brief A window component for managing MIDI input/output devices and basic settings.
 *
 * This class provides a JUCE DocumentWindow for selecting MIDI input/output devices,
 * adjusting volume and reverb sliders, and toggling the visibility of various controls.
 * It automatically refreshes the available MIDI devices using a timer.
 */
class MIDIWindow : public juce::DocumentWindow, public juce::ComboBox::Listener, public juce::Timer
{
public:
	/**
     * @brief Constructs a MIDIWindow with given MIDI device, device lists, and property file.
     * @param mdevice Reference to the MIDI device manager.
     * @param devicesListIN Vector of available MIDI input device names.
     * @param devicesListOUT Vector of available MIDI output device names.
     * @param prop Pointer to a JUCE PropertiesFile for saving settings.
     */
	MIDIWindow(MidiDevice& mdevice, std::vector<std::string>& devicesListIN, std::vector<std::string>& devicesListOUT, juce::PropertiesFile* prop);

	/** @brief Destructor stops any running timers. */
	~MIDIWindow() override;

	/**
     * @brief Sets the volume slider to the given value.
     * @param value Slider value (0.0 - 127.0)
     */
	void volumeSliderSetValue(double value);

	/**
     * @brief Sets the reverb slider to the given value.
     * @param value Slider value (0.0 - 127.0)
     */
	void reverbSliderSetValue(double value);

private:
	/**
     * @brief Called when the window visibility changes.
     * Updates combo boxes and starts/stops the timer.
     */
	void visibilityChanged() override;

	/** @brief Hides the window when the close button is pressed. */
	void closeButtonPressed() override;

	/**
     * @brief Callback for when a combo box selection changes.
     * Updates the selected MIDI device in MIDIDevice.
     * @param comboBoxThatHasChanged Pointer to the combo box that triggered the event.
     */
	void comboBoxChanged(juce::ComboBox* comboBoxThatHasChanged) override;

	/** @brief Timer callback periodically refreshes available MIDI devices. */
	void timerCallback() override;

	/** @brief Toggles the visibility of volume and reverb sliders. */
	void toggleSettingsSliders();

	/** @brief Toggles the visibility of the settings panel. */
	void toggleSettingsPanel();

	/** @brief Toggles the visibility of the MIDI input/output combo boxes and labels. */
	void toggleSettingsCB();

	/** @brief Toggles all settings controls (panel, sliders, and combo boxes). */
	void toggleSettingsAll();

	/** @brief Sets the bounds of all child components within the window. */
	void setBounds_components();

	/** @brief Initializes the settings panel. */
	void panelInit();

	/** @brief Initializes volume and reverb sliders and sets their callbacks. */
	void slidersInit();

	/** @brief Initializes the MIDI input/output combo boxes and labels. */
	void devicesCBinit();

	/** @brief Calls initialization functions for all subcomponents. */
	void allInit();

	/** @brief Populates the MIDI input combo box with the current devices. */
	void populateCBIN();

	/** @brief Populates the MIDI output combo box with the current devices. */
	void populateCBOUT();

	/** @brief Restores the last selected items in the combo boxes. */
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
