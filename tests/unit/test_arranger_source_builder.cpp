#include <juce_audio_basics/juce_audio_basics.h>
#include "Arranger/ArrangerSourceBuilder.h"

class ArrangerSourceBuilderTest : public juce::UnitTest
{
public:
    ArrangerSourceBuilderTest() : juce::UnitTest ("ArrangerSourceBuilder", "Arranger") {}

    void runTest() override
    {
        beginTest ("melodic tracks get 2,3,...; percussion gets 10; events convert to beats");
        {
            TrackEntry m1; m1.type = TrackType::Melodic; m1.displayName = "Keys";
            m1.instrumentAssociated = 4; m1.volumeAssociated = 90.0;
            m1.sequence.addEvent (juce::MidiMessage::noteOn  (1, 60, (juce::uint8) 100), 0.0);
            m1.sequence.addEvent (juce::MidiMessage::noteOff (1, 60), 1.0); // 1s -> 2 beats @120

            TrackEntry m2; m2.type = TrackType::Melodic; m2.displayName = "Bass";
            m2.sequence.addEvent (juce::MidiMessage::noteOn  (1, 40, (juce::uint8) 100), 0.0);
            m2.sequence.addEvent (juce::MidiMessage::noteOff (1, 40), 1.0);

            TrackEntry d; d.type = TrackType::Percussion; d.displayName = "Drums";
            d.sequence.addEvent (juce::MidiMessage::noteOn  (1, 36, (juce::uint8) 100), 0.0);
            d.sequence.addEvent (juce::MidiMessage::noteOff (1, 36), 0.5);

            std::vector<TrackEntry> tracks { m1, d, m2 };
            auto src = ArrangerSourceBuilder::fromTrackEntries (tracks, 120.0);

            expectEquals ((int) src.size(), 3);
            // order preserved; channels allocated by type
            expectEquals (src[0].channel, 2);   // first melodic
            expect (src[0].partType != ArrangerPartType::Drum);
            expectEquals (src[1].channel, 10);  // percussion
            expect (src[1].partType == ArrangerPartType::Drum);
            expectEquals (src[2].channel, 3);   // second melodic

            // part-type classification: "Keys" -> Acc, "Bass" -> Bass (name-based), "Drums" -> Drum
            expect (src[0].partType == ArrangerPartType::Acc);
            expect (src[2].partType == ArrangerPartType::Bass);

            // events converted + channel overridden
            expectEquals ((int) src[0].events.size(), 2);
            expectWithinAbsoluteError (src[0].events[1].beats, 2.0, 1e-9);
            expectEquals (src[0].events[0].message.getChannel(), 2);
            expectEquals (src[0].instrument, 4);
            expectWithinAbsoluteError (src[0].volume, 90.0, 1e-9);
        }

        beginTest ("more than 11 melodic tracks: overflow tracks are dropped, not crashed");
        {
            std::vector<TrackEntry> tracks;
            for (int i = 0; i < 13; ++i)
            {
                TrackEntry t; t.type = TrackType::Melodic; t.displayName = "M" + juce::String (i);
                t.sequence.addEvent (juce::MidiMessage::noteOn  (1, 60, (juce::uint8) 100), 0.0);
                t.sequence.addEvent (juce::MidiMessage::noteOff (1, 60), 0.5);
                tracks.push_back (t);
            }
            auto src = ArrangerSourceBuilder::fromTrackEntries (tracks, 120.0);
            expectEquals ((int) src.size(), 11); // 11 melodic channels available
        }
    }
};
static ArrangerSourceBuilderTest arrangerSourceBuilderTest;
