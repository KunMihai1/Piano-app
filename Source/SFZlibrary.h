/*
  ==============================================================================

    SFZlibrary.h
    Created: 29 May 2026 4:13:50pm
    Author:  Oricum

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

struct SFZLibraryEntry
{
    juce::String id;
    juce::String name;
    juce::String sfzPath;

    juce::File getFile() const { return juce::File(sfzPath); }
};

using SFZInstrumentMap = std::unordered_map<int, juce::String>;
using SFZStyleMappings = std::unordered_map<juce::String, SFZInstrumentMap>;

struct SFZLibraryData
{
    std::vector<SFZLibraryEntry> library;
    SFZStyleMappings styleMappings;
};

class SFZLibraryManager
{
public:
    void load(const juce::File& file);
    void save(const juce::File& file) const;

    const SFZLibraryData& getData() const;
    const std::vector<SFZLibraryEntry>& getEntries() const;

    juce::String addFile(const juce::File& sfzFile);
    void removeEntry(const juce::String& entryId);

    void assignToStyleInstrument(const juce::String& styleId, int instrumentNumber, const juce::String& entryId);
    void clearStyleInstrumentAssignment(const juce::String& styleId, int instrumentNumber);
    void importMappingsFromStyle(const juce::String& sourceStyleId, const juce::String& targetStyleId);

    const SFZLibraryEntry* getEntryById(const juce::String& entryId) const;
    juce::File getSfzForStyleInstrument(const juce::String& styleId, int instrumentNumber) const;

private:
    SFZLibraryData data;
};
