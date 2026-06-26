#include <juce_core/juce_core.h>
#include <juce_audio_basics/juce_audio_basics.h>
#include "Arranger/ChordDetector.h"

class ChordDetectorTest : public juce::UnitTest
{
public:
    ChordDetectorTest() : juce::UnitTest ("ChordDetector", "Arranger") {}

    static ArrangerChord detect (std::initializer_list<int> notes, ChordMode mode = ChordMode::Fingered)
    {
        ChordDetector d; d.setMode (mode);
        for (int n : notes) d.noteOn (n);
        return d.current();
    }

    void runTest() override
    {
        beginTest ("no held notes -> invalid chord");
        {
            ChordDetector d;
            ArrangerChord c = d.current();
            expect (! c.isValid(), "empty detector gave root=" + juce::String (c.root)
                                   + " q=" + toString (c.quality));
        }

        beginTest ("recognizes common triads and sevenths");
        {
            expect (detect({60,64,67}).quality    == ChordQuality::Maj);   // C E G
            expect (detect({60,63,67}).quality    == ChordQuality::Min);   // C Eb G
            expect (detect({60,64,67,70}).quality == ChordQuality::Dom7);  // C E G Bb
            expect (detect({60,64,67,71}).quality == ChordQuality::Maj7);  // C E G B
            expect (detect({60,63,67,70}).quality == ChordQuality::Min7);  // C Eb G Bb
            expect (detect({60,63,66}).quality    == ChordQuality::Dim);   // C Eb Gb
            expect (detect({60,65,67}).quality    == ChordQuality::Sus4);  // C F G
            expect (detect({60,62,67}).quality    == ChordQuality::Sus2);  // C D G
            expectEquals (detect({60,64,67}).root, 0);                     // C
            expectEquals (detect({62,66,69}).root, 2);                     // D F# A -> D major
        }

        beginTest ("inversions resolve to the same chord and root, bass = lowest note");
        {
            // E G C and G C E are both C major, root C
            expect (detect({64,67,72}).quality == ChordQuality::Maj);
            expectEquals (detect({64,67,72}).root, 0);
            expectEquals (detect({64,67,72}).bassNote, 4);   // lowest = E
            expectEquals (detect({67,72,76}).root, 0);
            expectEquals (detect({67,72,76}).bassNote, 7);   // lowest = G
        }

        beginTest ("ambiguous/sparse set -> None (split mode)");
        {
            ChordDetector d; d.noteOn (60); d.noteOn (61);   // C + C#: not a known chord
            expect (! d.current().isValid());
        }

        beginTest ("noteOff removes notes; emptying returns invalid (split)");
        {
            ChordDetector d;
            d.noteOn (60); d.noteOn (64); d.noteOn (67);
            expect (d.current().isValid());
            d.noteOff (60); d.noteOff (64); d.noteOff (67);
            expect (! d.current().isValid());
        }

        beginTest ("single-finger: Korg one-finger mapping");
        {
            using M = ChordMode;
            expect (detect({60},       M::SingleFinger).quality == ChordQuality::Maj);   // C alone
            expect (detect({60,59},    M::SingleFinger).quality == ChordQuality::Dom7);  // C + B  (white left)
            expect (detect({60,58},    M::SingleFinger).quality == ChordQuality::Min);   // C + Bb (black left)
            expect (detect({60,59,58}, M::SingleFinger).quality == ChordQuality::Min7);  // C + B + Bb
            expectEquals (detect({60}, M::SingleFinger).root, 0);                        // root C
            // root is the HIGHEST held note; lower notes are modifiers:
            expect (detect({55,60},    M::SingleFinger).quality == ChordQuality::Dom7);  // root C(60), G(55) white-left
            expectEquals (detect({55,60}, M::SingleFinger).root, 0);
            {
                ChordDetector d; d.setMode (M::SingleFinger);
                expect (! d.current().isValid());
            }
        }

        beginTest ("full-keyboard: bass triad wins, melody on top never lurches");
        {
            ChordDetector d; d.setMode (ChordMode::FullKeyboard);
            for (int n : {48,52,55}) d.noteOn (n);            // C major in the bass hand
            expect (d.current().quality == ChordQuality::Maj);
            d.noteOn (70);                                    // Bb melody on top: full set would read C7
            expect (d.current().quality == ChordQuality::Maj, "lurched to a 7th on a melody note");
            expectEquals (d.current().root, 0);

            // Moving the bass hand DOES change the chord (responsive, no debounce):
            for (int n : {48,52,55}) d.noteOff (n);
            for (int n : {41,45,48}) d.noteOn (n);            // F major in the bass
            expectEquals (d.current().root, 5);               // F
            expect (d.current().quality == ChordQuality::Maj);
        }

        beginTest ("full-keyboard: a chord-tone melody note does not change the chord");
        {
            ChordDetector d; d.setMode (ChordMode::FullKeyboard);
            for (int n : {48,52,55}) d.noteOn (n);   // C major (low)
            ArrangerChord before = d.current();
            expect (before.quality == ChordQuality::Maj);
            d.noteOn (84);                            // high C (a chord tone)
            expect (d.current() == before);
        }

        beginTest ("full-keyboard: requires >=3 notes forming a known chord");
        {
            ChordDetector d; d.setMode (ChordMode::FullKeyboard);
            d.noteOn (60); d.noteOn (84);             // only 2 notes
            ArrangerChord c = d.current();
            expect (! c.isValid(), "2-note full-kb gave root=" + juce::String (c.root)
                                   + " q=" + toString (c.quality));
        }

        beginTest ("full-keyboard: holds the chord when a non-chord melody note is added");
        {
            ChordDetector d; d.setMode (ChordMode::FullKeyboard);
            for (int n : {48,52,55}) d.noteOn (n);    // C major
            d.noteOn (86);                            // D (not a C-major tone) -> hold C major
            expect (d.current().quality == ChordQuality::Maj);
            expectEquals (d.current().root, 0);
        }

        beginTest ("single-finger: roots track the highest note; colour rules with extra keys");
        {
            using M = ChordMode;
            expect (detect({65}, M::SingleFinger).quality == ChordQuality::Maj);   // F alone
            expectEquals (detect({65}, M::SingleFinger).root, 5);                  // F
            expect (detect({65,67}, M::SingleFinger).quality == ChordQuality::Dom7); // G(67)+F(65 white) -> G7
            expectEquals (detect({65,67}, M::SingleFinger).root, 7);              // G
            expectEquals (detect({66}, M::SingleFinger).root, 6);                 // F# major
            // any white AND any black below the root -> minor 7, regardless of how many keys
            expect (detect({72,71,70,68}, M::SingleFinger).quality == ChordQuality::Min7); // C; B(white)+Bb,Ab(black)
        }

        beginTest ("full-keyboard: recognises a bass-hand inversion (E-G-C low)");
        {
            ChordDetector d; d.setMode (ChordMode::FullKeyboard);
            for (int n : {52,55,60}) d.noteOn (n);   // E G C low = C major, first inversion
            expectEquals (d.current().root, 0);
            expect (d.current().quality == ChordQuality::Maj);
        }

        beginTest ("mode switch re-recognises the currently held notes");
        {
            ChordDetector d; d.setMode (ChordMode::Fingered);
            d.noteOn (60); d.noteOn (59);            // C + B: not a Fingered chord
            expect (! d.current().isValid());
            d.setMode (ChordMode::SingleFinger);     // setMode() recomputes from the held notes
            expect (d.current().quality == ChordQuality::Dom7);   // C7 (root C, B is a white key to the left)
            expectEquals (d.current().root, 0);
        }

        beginTest ("key finder picks C major from a C-major-heavy pattern");
        {
            std::vector<TimedBeatEvent> evs;
            for (int n : { 60,64,67, 60,64,67, 60, 67 })
                evs.push_back ({ 0.0, juce::MidiMessage::noteOn (2, n, (juce::uint8)100) });
            ArrangerChord k = detectKeyFromEvents (evs);
            expectEquals (k.root, 0);
            expect (k.quality == ChordQuality::Maj);
        }

        beginTest ("key finder detects minor and falls back to C major on empty");
        {
            std::vector<TimedBeatEvent> minor;
            for (int n : { 57,60,64, 57,60,64, 57 })   // A C E -> A minor
                minor.push_back ({ 0.0, juce::MidiMessage::noteOn (2, n, (juce::uint8)100) });
            ArrangerChord k = detectKeyFromEvents (minor);
            expectEquals (k.root, 9);                  // A
            expect (k.quality == ChordQuality::Min);

            ArrangerChord fb = detectKeyFromEvents ({});
            expectEquals (fb.root, 0);
            expect (fb.quality == ChordQuality::Maj);
        }
    }
};
static ChordDetectorTest chordDetectorTest;
