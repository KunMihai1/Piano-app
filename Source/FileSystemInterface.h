/*
  ==============================================================================

    FileSystemInterface.h
    Created: 12 Mar 2026 12:09:48am
    Author:  Kisuke

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>


class IFileSystem
{
public:
    virtual ~IFileSystem() = default;

    // Get a “file” for JSON
    virtual juce::File getJsonFile() = 0;

    // Write text to a file
    virtual void writeFile(const juce::File& file, const juce::String& text) = 0;

    // Read text from a file
    virtual juce::String readFile(const juce::File& file) = 0;
};