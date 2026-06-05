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
            ArrangerStyle style = ArrangerPatternBuilder::buildSingleSectionStyle (tracks, 4, 4, 120.0);

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
            ArrangerStyle style = ArrangerPatternBuilder::buildSingleSectionStyle (tracks, 4, 4, 0.0);
            expectEquals (style.sections[0].lengthBars, 1);
        }

        beginTest ("demo multi-section style: four sections with the right transport rules");
        {
            // melodic note ends at 2.0s -> 4 beats @120 -> 1 bar; perc hit later -> 2 bars.
            TrackEntry melodic;
            melodic.type = TrackType::Melodic;
            melodic.originalBPM = 120.0;
            melodic.sequence.addEvent (juce::MidiMessage::noteOn  (1, 60, (juce::uint8) 100), 0.0);
            melodic.sequence.addEvent (juce::MidiMessage::noteOff (1, 60), 2.0);

            TrackEntry perc;
            perc.type = TrackType::Percussion;
            perc.originalBPM = 120.0;
            perc.sequence.addEvent (juce::MidiMessage::noteOn  (1, 36, (juce::uint8) 100), 5.0); // 10 beats -> bar 3 area
            perc.sequence.addEvent (juce::MidiMessage::noteOff (1, 36), 5.5);

            std::vector<TrackEntry> tracks { melodic, perc };
            ArrangerStyle style = ArrangerPatternBuilder::buildDemoMultiSectionStyle (tracks, 4, 4, 120.0);

            expectEquals (style.schemaVersion, 2);
            expectEquals ((int) style.sections.size(), 4);

            // order: Intro, Variation, Fill, Ending
            expect (style.sections[0].type == ArrangerSectionType::Intro);
            expect (style.sections[1].type == ArrangerSectionType::Variation);
            expect (style.sections[2].type == ArrangerSectionType::Fill);
            expect (style.sections[3].type == ArrangerSectionType::Ending);

            expect (style.sections[0].afterComplete == ArrangerAfterComplete::FallThrough); // intro
            expect (style.sections[1].afterComplete == ArrangerAfterComplete::Loop);        // variation
            expect (style.sections[2].afterComplete == ArrangerAfterComplete::FallThrough); // fill
            expect (style.sections[3].afterComplete == ArrangerAfterComplete::Stop);        // ending

            // Intro and Fill are one-bar slices; Ending is the last min(4, loopBars) bars; Variation is the full loop.
            // perc note-off at 5.5s -> 11 beats @120 -> 3 bars, so the Variation (and the ending window) span 3 bars.
            expectEquals (style.sections[1].lengthBars, 3); // variation = full loop
            expectEquals (style.sections[0].lengthBars, 1); // intro is one bar
            expectEquals (style.sections[2].lengthBars, 1); // fill is one bar
            expectEquals (style.sections[3].lengthBars, 3); // ending = last min(4, loopBars) = 3 bars

            // Variation keeps the Phase-1 channel mapping: melodic -> 2, perc -> 10.
            expectEquals (style.sections[1].tracks[0].channel, 2);
            expectEquals (style.sections[1].tracks[1].channel, 10);
        }

        beginTest ("fill slices the loop's last bar and rebases it to beat 0");
        {
            // single 2-bar loop: a melodic hit in the last bar (beat ~6) must reappear in the fill near beat 2.
            TrackEntry t;
            t.type = TrackType::Melodic;
            t.originalBPM = 120.0;
            t.sequence.addEvent (juce::MidiMessage::noteOn  (1, 72, (juce::uint8) 100), 3.0); // 6 beats @120 -> bar 2
            t.sequence.addEvent (juce::MidiMessage::noteOff (1, 72), 3.5);

            std::vector<TrackEntry> tracks { t };
            ArrangerStyle style = ArrangerPatternBuilder::buildDemoMultiSectionStyle (tracks, 4, 4, 120.0);

            expectEquals (style.sections[1].lengthBars, 2);          // variation is 2 bars
            auto& fillTrack = style.sections[2].tracks[0];
            expect (! fillTrack.pattern.empty());
            // event at beat 6 in a 2-bar (8-beat) loop -> fill (last bar starts at beat 4) -> beat 2.
            expectWithinAbsoluteError (fillTrack.pattern.front().beats, 2.0, 1e-9);
        }
    }
};

static ArrangerPatternBuilderTest arrangerPatternBuilderTest;
