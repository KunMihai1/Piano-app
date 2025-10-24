/*
  ==============================================================================

    midiDevicesDB.h
    Created: 26 Mar 2025 2:02:06am
    Author:  Kisuke

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>


/**
 * @brief Handles storage and management of known MIDI devices.
 *
 * This class maintains a JSON database of MIDI devices (VID/PID, name, number of keys)
 * and can populate default devices. It also provides functions to query the number of
 * keys for a given VID/PID combination.
 */
class MidiDevicesDataBase {
public:

    /**
     * @brief Constructs the MIDI devices database and ensures JSON file exists.
     */
    MidiDevicesDataBase();

    /**
     * @brief Destructor.
     */
    ~MidiDevicesDataBase();

    /**
     * @brief Adds a MIDI device entry to the JSON database if it does not already exist.
     * @param vid Vendor ID of the MIDI device.
     * @param pid Product ID of the MIDI device.
     * @param name Human-readable name of the device.
     * @param numKeys Number of keys on the MIDI device.
     */
    void addDeviceJson(const juce::String& vid, const::juce::String& pid, const::juce::String& name, int numKeys);

    /**
     * @brief Retrieves the number of keys for a device by VID and PID.
     * @param vid Vendor ID of the device.
     * @param pid Product ID of the device.
     * @return Number of keys, or -1 if the device is not found.
     */
    int getNrKeysPidVid(const juce::String& vid, const juce::String& pid);

    /**
     * @brief Returns the application-specific folder for storing the JSON database.
     * @return Juce File object pointing to the app data folder.
     */
    juce::File getAppDataFolder();

private:

    /**
     * @brief Populates the JSON database with a set of default MIDI devices.
     */
    void populateInitialDevices();

    /**
     * @brief Ensures the JSON file exists, creates it if not, and initializes data.
     */
    void jsonEnsureExistance();

    /**
     * @brief Loads JSON data from the file into memory.
     */
    void loadJsonFile();

    /**
     * @brief Saves the current JSON data to the file.
     */
    void saveJsonFile();

    /**
     * @brief Returns the File object pointing to the JSON file in app data.
     * @return Juce File object for the JSON file.
     */
    juce::File getJsonFile();
    juce::var jsonData;
};
