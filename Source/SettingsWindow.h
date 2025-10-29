/*
  ==============================================================================

    MIDIWindow.h
    Created: 2025
    Author: Kisuke

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#include "MidiHandler.h"

/**
 * @class MIDIWindow
 * @brief Window for configuring MIDI input/output devices and volume/reverb settings.
 *
 * Provides a user interface for:
 *   - Selecting MIDI input and output devices
 *   - Adjusting MIDI volume and reverb
 *   - Storing and restoring user settings
 *   - Automatically updating device lists when changes occur
 *
 * Inherits from juce::DocumentWindow and implements ComboBox::Listener and Timer.
 */
class MIDIWindow : public juce::DocumentWindow,
                   private juce::ComboBox::Listener,
                   private juce::Timer
{
public:
    /**
     * @brief Constructor.
     * @param mdevice Reference to the MIDI device object.
     * @param devicesListIN Reference to a vector containing available MIDI input devices.
     * @param devicesListOUT Reference to a vector containing available MIDI output devices.
     * @param prop Optional pointer to a JUCE PropertiesFile for storing settings.
     *
     * Initializes the window, its sliders, ComboBoxes, and panel, and sets default visibility.
     */
    MIDIWindow(MidiDevice& mdevice, std::vector<std::string>& devicesListIN,
               std::vector<std::string>& devicesListOUT, juce::PropertiesFile* prop);

    /** @brief Destructor stops the update timer and cleans up. */
    ~MIDIWindow() override;

    /** @brief Sets the volume slider to a specific value. */
    void volumeSliderSetValue(double value);

    /** @brief Sets the reverb slider to a specific value. */
    void reverbSliderSetValue(double value);

    /** @brief Called when the window visibility changes. */
    void visibilityChanged() override;

    /** @brief Called when the window close button is pressed. */
    void closeButtonPressed() override;

    /** @brief Handles ComboBox selection changes for MIDI devices. */
    void comboBoxChanged(juce::ComboBox* comboBoxThatHasChanged) override;

    /** @brief Timer callback to update device lists if they change. */
    void timerCallback() override;

private:
    /** @brief Toggles visibility of sliders (volume/reverb). */
    void toggleSettingsSliders();

    /** @brief Toggles visibility of the settings panel. */
    void toggleSettingsPanel();

    /** @brief Toggles visibility of device ComboBoxes. */
    void toggleSettingsCB();

    /** @brief Toggles visibility of all settings components. */
    void toggleSettingsAll();

    /** @brief Sets bounds of all window components manually. */
    void setBounds_components();

    /** @brief Initializes the main panel. */
    void panelInit();

    /** @brief Initializes sliders and attaches their callbacks. */
    void slidersInit();

    /** @brief Initializes device ComboBoxes. */
    void devicesCBinit();

    /** @brief Calls all initialization functions. */
    void allInit();

    /** @brief Populates the input device ComboBox. */
    void populateCBIN();

    /** @brief Populates the output device ComboBox. */
    void populateCBOUT();

    /** @brief Restores ComboBox selections from previously saved indices. */
    void restoreCBoxes();

    // Member variables
    MidiDevice& MIDIDevice;                   /**< Reference to the MIDI device object */
    std::vector<std::string>& devicesListIN; /**< Vector of available MIDI input devices */
    std::vector<std::string>& devicesListOUT;/**< Vector of available MIDI output devices */
    juce::PropertiesFile* propertyFile = nullptr; /**< Optional storage for user settings */

    juce::Component settingsPanel;            /**< Panel containing all settings components */

    juce::Slider volumeSlider;                /**< Slider for adjusting MIDI volume */
    juce::Slider reverbSlider;                /**< Slider for adjusting MIDI reverb */
    juce::Label volumeLabel;                  /**< Label for the volume slider */
    juce::Label reverbLabel;                  /**< Label for the reverb slider */

    juce::ComboBox comboBoxDevicesIN;         /**< ComboBox for selecting MIDI input devices */
    juce::ComboBox comboBoxDevicesOUT;        /**< ComboBox for selecting MIDI output devices */
    juce::Label midiDevicesLabelIN;           /**< Label for input devices */
    juce::Label midiDevicesLabelOUT;          /**< Label for output devices */

    int lastIndexIN = 1;                       /**< Last selected input device index */
    int lastIndexOUT = 1;                      /**< Last selected output device index */
};
