#include <juce_core/juce_core.h>
#include <juce_audio_basics/juce_audio_basics.h>
#include "Arranger/ArrangerScheduler.h"

class ArrangerSchedulerTest : public juce::UnitTest
{
public:
    ArrangerSchedulerTest() : juce::UnitTest ("ArrangerScheduler", "Arranger") {}

    static std::vector<TimedBeatEvent> oneBarKickAndHeld()
    {
        // loop length 4 beats. Kick on beat 0 (off at 0.5). A held note on at beat 3 with NO note-off
        // before the loop end (must be closed at the seam).
        return {
            { 0.0, juce::MidiMessage::noteOn  (10, 36, (juce::uint8) 100) },
            { 0.5, juce::MidiMessage::noteOff (10, 36) },
            { 3.0, juce::MidiMessage::noteOn  (2, 60, (juce::uint8) 90) },
        };
    }

    void runTest() override
    {
        beginTest ("emits events whose phase is inside the swept window");
        {
            ArrangerScheduler s;
            s.setLoop (oneBarKickAndHeld(), 4.0);
            auto out = s.advance (0.0, 1.0);   // sweeps beats [0,1)
            expectEquals ((int) out.size(), 2); // kick on + kick off
            expectWithinAbsoluteError (out[0].beats, 0.0, 1e-9);
            expect (out[0].message.isNoteOn());
            expectWithinAbsoluteError (out[1].beats, 0.5, 1e-9);
            expect (out[1].message.isNoteOff());
        }

        beginTest ("does not emit events outside the window");
        {
            ArrangerScheduler s;
            s.setLoop (oneBarKickAndHeld(), 4.0);
            auto out = s.advance (1.0, 2.0);
            expectEquals ((int) out.size(), 0);
        }

        beginTest ("closes a held note at the loop seam and retriggers next loop");
        {
            ArrangerScheduler s;
            s.setLoop (oneBarKickAndHeld(), 4.0);
            s.advance (2.5, 3.5);            // arm: turns the held note (2,60) on at beat 3.0
            // Now sweep across the seam: from beat 3.5 to beat 4.5.
            auto out = s.advance (3.5, 4.5);
            // Expected, in order: synthetic noteOff for held (2,60) at beat 4.0 (seam),
            // then kick noteOn (10,36) at beat 4.0 (phase 0 of next loop).
            expect (out.size() >= 2);
            expectWithinAbsoluteError (out[0].beats, 4.0, 1e-9);
            expect (out[0].message.isNoteOff());
            expectEquals (out[0].message.getNoteNumber(), 60);
            expectEquals (out[0].message.getChannel(), 2);
            expect (out[1].message.isNoteOn());
            expectEquals (out[1].message.getNoteNumber(), 36); // kick retriggers at seam
        }

        beginTest ("reset clears active-note state (no spurious seam note-off)");
        {
            ArrangerScheduler s;
            s.setLoop (oneBarKickAndHeld(), 4.0);
            s.advance (3.0, 3.5);   // turns the held note (2,60) on
            s.reset();              // forget active notes
            auto out = s.advance (3.5, 4.5); // crossing seam, but nothing is active to close
            for (auto& e : out)
                expect (! (e.message.isNoteOff() && e.message.getNoteNumber() == 60));
        }

        beginTest ("handles a window longer than the loop (multiple wraps)");
        {
            ArrangerScheduler s;
            s.setLoop (oneBarKickAndHeld(), 4.0);
            auto out = s.advance (0.0, 8.0); // two full loops
            int kickOns = 0;
            for (auto& e : out) if (e.message.isNoteOn() && e.message.getNoteNumber() == 36) ++kickOns;
            expectEquals (kickOns, 2);
        }

        beginTest ("flushActiveNotes closes sounding notes and then nothing");
        {
            ArrangerScheduler s;
            // A held note on at beat 0 with no note-off in the bar.
            s.setLoop ({ { 0.0, juce::MidiMessage::noteOn (3, 64, (juce::uint8) 100) } }, 4.0);
            s.advance (0.0, 1.0);                 // turns (3,64) on

            auto flushed = s.flushActiveNotes (1.0);
            expectEquals ((int) flushed.size(), 1);
            expect (flushed[0].message.isNoteOff());
            expectEquals (flushed[0].message.getChannel(), 3);
            expectEquals (flushed[0].message.getNoteNumber(), 64);

            auto again = s.flushActiveNotes (1.0); // nothing left to close
            expectEquals ((int) again.size(), 0);
        }
    }
};

static ArrangerSchedulerTest arrangerSchedulerTest;
