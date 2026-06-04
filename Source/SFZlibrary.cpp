/*
  ==============================================================================

    SFZlibrary.cpp
    Created: 29 May 2026 4:13:50pm
    Author:  Oricum

  ==============================================================================
*/

#include "SFZlibrary.h"
#include "IOHelper.h"

void SFZLibraryManager::load(const juce::File& file)
{
    SFZLibraryIOHelper::loadFromFile(file, data);
}

void SFZLibraryManager::save(const juce::File& file) const
{
    SFZLibraryIOHelper::saveToFile(file, data);
}

const SFZLibraryData& SFZLibraryManager::getData() const
{
    return data;
}

const std::vector<SFZLibraryEntry>& SFZLibraryManager::getEntries() const
{
    return data.library;
}

juce::String SFZLibraryManager::addFile(const juce::File& sfzFile)
{
    if (!sfzFile.existsAsFile())
        return {};

    const auto fullPath = sfzFile.getFullPathName();

    for (const auto& entry : data.library)
    {
        if (entry.sfzPath == fullPath)
            return entry.id;
    }

    SFZLibraryEntry entry;
    entry.id = juce::Uuid().toString();
    entry.name = sfzFile.getFileNameWithoutExtension();
    entry.sfzPath = fullPath;

    data.library.push_back(entry);
    return entry.id;
}

void SFZLibraryManager::removeEntry(const juce::String& entryId)
{
    data.library.erase(std::remove_if(data.library.begin(), data.library.end(),
                                      [&entryId](const SFZLibraryEntry& entry)
                                      {
                                          return entry.id == entryId;
                                      }),
                       data.library.end());

    for (auto styleIt = data.styleMappings.begin(); styleIt != data.styleMappings.end();)
    {
        auto& instrumentMap = styleIt->second;
        for (auto it = instrumentMap.begin(); it != instrumentMap.end();)
        {
            if (it->second == entryId)
                it = instrumentMap.erase(it);
            else
                ++it;
        }
        if (instrumentMap.empty())
            styleIt = data.styleMappings.erase(styleIt);
        else
            ++styleIt;
    }
}

void SFZLibraryManager::importMappingsFromStyle(const juce::String& sourceStyleId, const juce::String& targetStyleId)
{
    if (sourceStyleId == targetStyleId) return;

    const auto sourceIt = data.styleMappings.find(sourceStyleId);
    if (sourceIt == data.styleMappings.end()) return;

    auto& targetMap = data.styleMappings[targetStyleId];
    for (const auto& pair : sourceIt->second)
        targetMap[pair.first] = pair.second;
}

void SFZLibraryManager::assignToStyleInstrument(const juce::String& styleId, int instrumentNumber, const juce::String& entryId)
{
    if (styleId.isEmpty() || instrumentNumber < 0 || instrumentNumber > 127 || getEntryById(entryId) == nullptr)
        return;

    data.styleMappings[styleId][instrumentNumber] = entryId;
}

void SFZLibraryManager::clearStyleInstrumentAssignment(const juce::String& styleId, int instrumentNumber)
{
    auto styleIt = data.styleMappings.find(styleId);
    if (styleIt == data.styleMappings.end())
        return;

    styleIt->second.erase(instrumentNumber);

    if (styleIt->second.empty())
        data.styleMappings.erase(styleIt);
}

const SFZLibraryEntry* SFZLibraryManager::getEntryById(const juce::String& entryId) const
{
    for (const auto& entry : data.library)
    {
        if (entry.id == entryId)
            return &entry;
    }

    return nullptr;
}

juce::File SFZLibraryManager::getSfzForStyleInstrument(const juce::String& styleId, int instrumentNumber) const
{
    const auto styleIt = data.styleMappings.find(styleId);
    if (styleIt == data.styleMappings.end())
        return {};

    const auto instrumentIt = styleIt->second.find(instrumentNumber);
    if (instrumentIt == styleIt->second.end())
        return {};

    if (const auto* entry = getEntryById(instrumentIt->second))
        return entry->getFile();

    return {};
}
