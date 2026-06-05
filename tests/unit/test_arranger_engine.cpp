#include <juce_core/juce_core.h>
#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_audio_devices/juce_audio_devices.h>
#include "Arranger/ArrangerEngine.h"
#include "Arranger/ArrangerModel.h"

class ArrangerEngineTest : public juce::UnitTest
{
public:
    ArrangerEngineTest() : juce::UnitTest ("ArrangerEngine", "Arranger") {}

    static ArrangerStyle makeStyle()
    {
        ArrangerTrack t;
        t.channel = 2;
        t.partType = ArrangerPartType::Acc;
        t.pattern = {
            { 0.0, juce::MidiMessage::noteOn  (2, 60, (juce::uint8) 100) },
            { 0.5, juce::MidiMessage::noteOff (2, 60) },
        };
        ArrangerSection s; s.lengthBars = 1; s.tracks.push_back (t);
        ArrangerStyle style; style.timeSigNum = 4; style.timeSigDenom = 4; style.originalTempo = 120.0;
        style.sections.push_back (s);
        return style;
    }

    void runTest() override
    {
        beginTest ("renderRange dispatches scheduled events through onMidiMessage");
        {
            ArrangerEngine engine (std::weak_ptr<juce::MidiOutput>{});
            std::vector<juce::MidiMessage> captured;
            engine.onMidiMessage = [&] (const juce::MidiMessage& m) { captured.push_back (m); };

            engine.setStyle (makeStyle());
            engine.renderRange (0.0, 1.0);   // beats [0,1): note on then off

            expectEquals ((int) captured.size(), 2);
            expect (captured[0].isNoteOn());
            expectEquals (captured[0].getNoteNumber(), 60);
            expect (captured[1].isNoteOff());
        }

        beginTest ("loop length in beats reflects the section bars");
        {
            ArrangerEngine engine (std::weak_ptr<juce::MidiOutput>{});
            engine.setStyle (makeStyle());
            expectWithinAbsoluteError (engine.getLoopLengthBeats(), 4.0, 1e-9); // 1 bar of 4/4
        }

        beginTest ("stop sends all-notes-off and is reflected in isPlaying");
        {
            ArrangerEngine engine (std::weak_ptr<juce::MidiOutput>{});
            int noteOffCount = 0;
            engine.onMidiMessage = [&] (const juce::MidiMessage& m)
            {
                if (m.isAllNotesOff()) ++noteOffCount;
            };
            engine.setStyle (makeStyle());
            engine.stop();
            expect (! engine.isPlaying());
            expect (noteOffCount >= 1);
        }
    }
};

static ArrangerEngineTest arrangerEngineTest;
