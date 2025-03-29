/*
  ==============================================================================

    midiDevicesDB.h
    Created: 26 Mar 2025 2:02:06am
    Author:  Kisuke

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>

class MidiDevicesDataBase {
public:
    MidiDevicesDataBase();
    ~MidiDevicesDataBase();
    void addDeviceJson(const juce::String& vid, const::juce::String& pid, const::juce::String& name, int numKeys);
    int getNrKeysPidVid(const juce::String& vid, const juce::String& pid);
private:
    void jsonEnsureExistance();
    void loadJsonFile();
    void saveJsonFile();
    juce::File getJsonFile();
    juce::var jsonData;
};