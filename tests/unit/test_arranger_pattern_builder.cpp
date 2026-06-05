#include <juce_core/juce_core.h>
#include <juce_audio_basics/juce_audio_basics.h>
#include "Arranger/ArrangerPatternBuilder.h"
#include "TrackEntry.h"

class ArrangerPatternBuilderTest : public juce::UnitTest
{
public:
    ArrangerPatternBuilderTest() : juce::UnitTest ("ArrangerPatternBuilder", "Arranger") {}

    void runTest() override
    {
        beginTest ("seconds convert to beats at the reference BPM");
        {
            juce::MidiMessageSequence seq;
            seq.addEvent (juce::MidiMessage::noteOn (1, 60, (juce::uint8) 100), 0.0);
            seq.addEvent (juce::MidiMessage::noteOff (1, 60), 1.0); // 1.0s -> 2 beats @120

            auto evts = ArrangerPatternBuilder::buildBeatEvents (seq, 120.0, 5);
            expectEquals ((int) evts.size(), 2);
            expectWithinAbsoluteError (evts[0].beats, 0.0, 1e-9);
            expectWithinAbsoluteError (evts[1].beats, 2.0, 1e-9);
            // channel is overridden to the supplied value
            expectEquals (evts[0].message.getChannel(), 5);
            expectEquals (evts[1].message.getChannel(), 5);
        }

        beginTest ("non-note events are skipped");
        {
            juce::MidiMessageSequence seq;
            seq.addEvent (juce::MidiMessage::controllerEvent (1, 7, 100), 0.0);
            seq.addEvent (juce::MidiMessage::noteOn (1, 60, (juce::uint8) 100), 0.0);
            auto evts = ArrangerPatternBuilder::buildBeatEvents (seq, 120.0, 2);
            expectEquals ((int) evts.size(), 1);
            expect (evts[0].message.isNoteOn());
        }

        beginTest ("single-section style: channel mapping and loop length");
        {
            // melodic track: a note ending at 2.0s -> 4 beats @120 -> exactly 1 bar (4/4).
            TrackEntry melodic;
            melodic.type = TrackType::Melodic;
            melodic.originalBPM = 120.0;
            melodic.sequence.addEvent (juce::MidiMessage::noteOn  (1, 60, (juce::uint8) 100), 0.0);
            melodic.sequence.addEvent (juce::MidiMessage::noteOff (1, 60), 2.0);

            // percussion track: a hit ending at 3.0s -> 6 beats -> rounds up to 2 bars.
            TrackEntry perc;
            perc.type = TrackType::Percussion;
            perc.originalBPM = 120.0;
            perc.sequence.addEvent (juce::MidiMessage::noteOn  (1, 36, (juce::uint8) 100), 0.0);
            perc.sequence.addEvent (juce::MidiMessage::noteOff (1, 36), 3.0);

            std::vector<TrackEntry> tracks { melodic, perc };
            ArrangerStyle style = ArrangerPatternBuilder::buildSingleSectionStyle (tracks, 4, 4);

            expectEquals ((int) style.sections.size(), 1);
            auto& sec = style.sections[0];
            expectEquals ((int) sec.tracks.size(), 2);

            // melodic -> channel 2; percussion -> channel 10
            expectEquals (sec.tracks[0].channel, 2);
            expect (sec.tracks[0].partType != ArrangerPartType::Drum);
            expectEquals (sec.tracks[1].channel, 10);
            expect (sec.tracks[1].partType == ArrangerPartType::Drum);

            // loop length = max(1 bar, 2 bars) = 2 bars
            expectEquals (sec.lengthBars, 2);
        }

        beginTest ("originalBPM <= 0 falls back to 120");
        {
            TrackEntry t;
            t.type = TrackType::Melodic;
            t.originalBPM = 0.0;
            t.sequence.addEvent (juce::MidiMessage::noteOn  (1, 60, (juce::uint8) 100), 0.0);
            t.sequence.addEvent (juce::MidiMessage::noteOff (1, 60), 2.0); // 2s -> 4 beats @120
            std::vector<TrackEntry> tracks { t };
            ArrangerStyle style = ArrangerPatternBuilder::buildSingleSectionStyle (tracks, 4, 4);
            expectEquals (style.sections[0].lengthBars, 1);
        }
    }
};

static ArrangerPatternBuilderTest arrangerPatternBuilderTest;
