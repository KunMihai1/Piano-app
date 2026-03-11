/*
  ==============================================================================

    midiDevicesDB.h
    Created: 26 Mar 2025 2:02:06am
    Author:  Kisuke

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#include "FileSystemInterface.h"

/**
 * @class MidiDevicesDataBase
 * @brief Manages a JSON database of MIDI devices.
 *
 * Allows storing device information (VID, PID, name, number of keys) in a JSON
 * file located in the user’s application data folder. Supports querying and
 * populating initial devices.
 */
class MidiDevicesDataBase {
public:
    /** @brief Constructor. Ensures JSON exists and loads it. */
    MidiDevicesDataBase();

    MidiDevicesDataBase(IFileSystem& fs);

    /** @brief Destructor */
    ~MidiDevicesDataBase();

    /**
     * @brief Adds a MIDI device to the JSON database
     * @param vid Vendor ID of the device
     * @param pid Product ID of the device
     * @param name Device name
     * @param numKeys Number of keys on the device
     */
    void addDeviceJson(const juce::String& vid, const juce::String& pid, const juce::String& name, int numKeys);


    void updateDeviceJson(const juce::String& vid, const juce::String pid, const juce::String name, int numKeys);

    bool deviceExists(const juce::String& VID, const juce::String& PID);

    /**
     * @brief Returns the number of keys for a device matching VID and PID
     * @param vid Vendor ID
     * @param pid Product ID
     * @return Number of keys if found, -1 if not found
     */
    int getNrKeysPidVid(const juce::String& vid, const juce::String& pid);

    juce::String getDeviceName(const juce::String& vid, const juce::String& pid);

    /**
     * @brief Returns the application data folder for this application
     * @return File representing the app data folder
     */
    juce::File getAppDataFolder();



protected:
    juce::var jsonData; ///< Holds the parsed JSON data in memory

    /** @brief Loads the JSON file into memory. */
    virtual void loadJsonFile();

    /** @brief Saves the in-memory JSON back to file. */
    virtual void saveJsonFile();

private:
    /** @brief Populates the database with a set of initial devices. */
    void populateInitialDevices();

    /** @brief Ensures the JSON file exists, creating it if necessary. */
    void jsonEnsureExistance();


    /** @brief Returns the file object for the JSON file. */
    juce::File getJsonFile();

    IFileSystem* fileSystem = nullptr;

    
};
