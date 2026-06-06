#include <juce_core/juce_core.h>
#include <juce_audio_basics/juce_audio_basics.h>
#include "Arranger/ArrangerMidiHex.h"

class ArrangerMidiHexTest : public juce::UnitTest
{
public:
    ArrangerMidiHexTest() : juce::UnitTest ("ArrangerMidiHex", "Arranger") {}

    void runTest() override
    {
        beginTest ("note-on round-trips through hex");
        {
            auto m = juce::MidiMessage::noteOn (10, 36, (juce::uint8) 100);
            auto back = ArrangerMidiHex::fromHex (ArrangerMidiHex::toHex (m));
            expect (back.isNoteOn());
            expectEquals (back.getChannel(), 10);
            expectEquals (back.getNoteNumber(), 36);
            expectEquals ((int) back.getVelocity(), 100);
        }

        beginTest ("note-off round-trips through hex");
        {
            auto m = juce::MidiMessage::noteOff (2, 60);
            auto back = ArrangerMidiHex::fromHex (ArrangerMidiHex::toHex (m));
            expect (back.isNoteOff());
            expectEquals (back.getChannel(), 2);
            expectEquals (back.getNoteNumber(), 60);
        }

        beginTest ("hex string is uppercase bytes with no separators");
        {
            auto m = juce::MidiMessage::noteOn (1, 0x24, (juce::uint8) 0x64); // 90 24 64
            expectEquals (ArrangerMidiHex::toHex (m), juce::String ("902464"));
        }
    }
};
static ArrangerMidiHexTest arrangerMidiHexTest;
