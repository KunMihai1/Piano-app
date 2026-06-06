#pragma once
#include <JuceHeader.h>

namespace ArrangerMidiHex
{
    inline juce::String toHex (const juce::MidiMessage& m)
    {
        const juce::uint8* data = m.getRawData();
        const int size = m.getRawDataSize();
        juce::String out;
        for (int i = 0; i < size; ++i)
            out += juce::String::toHexString (data[i]).paddedLeft ('0', 2).toUpperCase();
        return out;
    }

    inline juce::MidiMessage fromHex (const juce::String& hex)
    {
        std::vector<juce::uint8> bytes;
        for (int i = 0; i + 1 < hex.length(); i += 2)
            bytes.push_back ((juce::uint8) hex.substring (i, i + 2).getHexValue32());
        if (bytes.empty())
            return juce::MidiMessage();
        return juce::MidiMessage (bytes.data(), (int) bytes.size());
    }
}
