#include <juce_core/juce_core.h>
#include "Arranger/ArrangerSectionSequencer.h"

class ArrangerSectionSequencerTest : public juce::UnitTest
{
public:
    ArrangerSectionSequencerTest() : juce::UnitTest ("ArrangerSectionSequencer", "Arranger") {}

    static ArrangerSection makeSection (const juce::String& id, const juce::String& name,
                                        ArrangerSectionType type, int bars,
                                        ArrangerAfterComplete after)
    {
        ArrangerSection s;
        s.id = id; s.name = name; s.type = type; s.lengthBars = bars; s.afterComplete = after;
        return s;
    }

    static ArrangerStyle twoVariations()
    {
        ArrangerStyle st; st.timeSigNum = 4; st.timeSigDenom = 4;
        st.sections.push_back (makeSection ("var_1", "Variation 1", ArrangerSectionType::Variation, 1, ArrangerAfterComplete::Loop));
        st.sections.push_back (makeSection ("var_2", "Variation 2", ArrangerSectionType::Variation, 1, ArrangerAfterComplete::Loop));
        return st;
    }

    void runTest() override
    {
        beginTest ("starts on the first variation");
        {
            ArrangerSectionSequencer seq;
            seq.setStyle (twoVariations());
            expectEquals (seq.getActiveIndex(), 0);
        }

        beginTest ("queued switch applies at the next bar boundary, not mid-bar");
        {
            ArrangerSectionSequencer seq;
            seq.setStyle (twoVariations());
            expect (seq.queue (ArrangerSectionType::Variation, "Variation 2"));

            auto a = seq.advance (0.0, 2.0);   // still inside bar 1
            expectEquals (seq.getActiveIndex(), 0);
            for (auto& s : a.segments) expectEquals (s.sectionIndex, 0);

            auto b = seq.advance (2.0, 5.0);   // crosses the bar line at beat 4
            expectEquals (seq.getActiveIndex(), 1);
            // last segment is the new section, flagged as a change
            expectEquals (b.segments.back().sectionIndex, 1);
            expect (b.segments.back().sectionChanged);
            expectWithinAbsoluteError (b.segments.back().localFromBeats, 0.0, 1e-9); // new section enters at its bar 0
        }

        beginTest ("double-press before the boundary keeps only the last request");
        {
            ArrangerSectionSequencer seq;
            ArrangerStyle st = twoVariations();
            st.sections.push_back (makeSection ("var_3", "Variation 3", ArrangerSectionType::Variation, 1, ArrangerAfterComplete::Loop));
            seq.setStyle (st);
            seq.queue (ArrangerSectionType::Variation, "Variation 2");
            seq.queue (ArrangerSectionType::Variation, "Variation 3"); // replaces the pending one
            seq.advance (0.0, 5.0);
            expectEquals (seq.getActiveIndex(), 2);
        }

        beginTest ("a Loop section never auto-transitions");
        {
            ArrangerSectionSequencer seq;
            seq.setStyle (twoVariations());
            auto out = seq.advance (0.0, 100.0); // 25 bars
            expectEquals (seq.getActiveIndex(), 0);
            expect (! out.stopRequested);
        }

        beginTest ("Intro plays once then falls into the variation");
        {
            ArrangerStyle st; st.timeSigNum = 4; st.timeSigDenom = 4;
            st.sections.push_back (makeSection ("intro_1", "Intro 1", ArrangerSectionType::Intro, 1, ArrangerAfterComplete::FallThrough));
            st.sections.push_back (makeSection ("var_1", "Variation 1", ArrangerSectionType::Variation, 1, ArrangerAfterComplete::Loop));

            ArrangerSectionSequencer seq;
            seq.setStyle (st);
            seq.startAt (0);                 // begin on the intro
            expectEquals (seq.getActiveIndex(), 0);

            auto out = seq.advance (0.0, 4.0); // exactly the intro's length
            expectEquals (seq.getActiveIndex(), 1); // fell through to the variation
            for (auto& s : out.segments) expectEquals (s.sectionIndex, 0); // intro played the whole bar
        }

        beginTest ("Fill plays once then returns to the variation it was triggered from");
        {
            ArrangerStyle st; st.timeSigNum = 4; st.timeSigDenom = 4;
            st.sections.push_back (makeSection ("var_1", "Variation 1", ArrangerSectionType::Variation, 1, ArrangerAfterComplete::Loop));
            st.sections.push_back (makeSection ("var_2", "Variation 2", ArrangerSectionType::Variation, 1, ArrangerAfterComplete::Loop));
            st.sections.push_back (makeSection ("fill_1", "Fill 1", ArrangerSectionType::Fill, 1, ArrangerAfterComplete::FallThrough));

            ArrangerSectionSequencer seq;
            seq.setStyle (st);
            seq.queue (ArrangerSectionType::Variation, "Variation 2");
            seq.advance (0.0, 5.0);          // now on Variation 2
            expectEquals (seq.getActiveIndex(), 1);

            seq.queue (ArrangerSectionType::Fill, "Fill 1");
            seq.advance (5.0, 9.0);          // boundary at 8 -> enter fill
            expectEquals (seq.getActiveIndex(), 2);

            seq.advance (9.0, 13.0);         // boundary at 12 -> fill done -> back to Variation 2
            expectEquals (seq.getActiveIndex(), 1);
        }

        beginTest ("Ending plays once then requests stop");
        {
            ArrangerStyle st; st.timeSigNum = 4; st.timeSigDenom = 4;
            st.sections.push_back (makeSection ("var_1", "Variation 1", ArrangerSectionType::Variation, 1, ArrangerAfterComplete::Loop));
            st.sections.push_back (makeSection ("ending_1", "Ending 1", ArrangerSectionType::Ending, 1, ArrangerAfterComplete::Stop));

            ArrangerSectionSequencer seq;
            seq.setStyle (st);
            seq.queue (ArrangerSectionType::Ending, "Ending 1");
            seq.advance (0.0, 5.0);          // boundary at 4 -> enter ending
            expectEquals (seq.getActiveIndex(), 1);

            auto out = seq.advance (5.0, 9.0); // boundary at 8 -> ending complete
            expect (out.stopRequested);
            auto none = seq.advance (9.0, 13.0); // stopped: no further segments
            expect (none.segments.empty());
        }

        beginTest ("a multi-bar window resolves transitions in order");
        {
            ArrangerStyle st; st.timeSigNum = 4; st.timeSigDenom = 4;
            st.sections.push_back (makeSection ("intro_1", "Intro 1", ArrangerSectionType::Intro, 1, ArrangerAfterComplete::FallThrough));
            st.sections.push_back (makeSection ("var_1", "Variation 1", ArrangerSectionType::Variation, 1, ArrangerAfterComplete::Loop));

            ArrangerSectionSequencer seq;
            seq.setStyle (st);
            seq.startAt (0);
            auto out = seq.advance (0.0, 12.0);  // intro bar, then 2 variation bars, all in one call
            expect (out.segments.size() >= 2);
            expectEquals (out.segments.front().sectionIndex, 0); // starts on intro
            expectEquals (out.segments.back().sectionIndex, 1);  // ends on variation
            expectEquals (seq.getActiveIndex(), 1);
        }
    }
};

static ArrangerSectionSequencerTest arrangerSectionSequencerTest;
