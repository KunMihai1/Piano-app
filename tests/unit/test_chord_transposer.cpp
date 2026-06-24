#include <juce_core/juce_core.h>
#include "Arranger/ChordTransposer.h"

class ChordTransposerTest : public juce::UnitTest
{
public:
    ChordTransposerTest() : juce::UnitTest ("ChordTransposer", "Arranger") {}

    void runTest() override
    {
        beginTest ("fixed parts (drums) are never transposed");
        {
            ChordTransposer t;
            t.setOriginalChord ({ 0, ChordQuality::Maj, 0 });
            t.setActiveChord   ({ 5, ChordQuality::Maj, 5 });   // F
            expectEquals (t.transpose (36, PartKind::Fixed), 36);
        }

        beginTest ("invalid active chord -> identity");
        {
            ChordTransposer t;
            t.setOriginalChord ({ 0, ChordQuality::Maj, 0 });
            t.setActiveChord   ({});                            // none
            expectEquals (t.transpose (60, PartKind::Acc), 60);
        }

        beginTest ("identity when active == original");
        {
            ChordTransposer t;
            t.setOriginalChord ({ 0, ChordQuality::Maj, 0 });
            t.setActiveChord   ({ 0, ChordQuality::Maj, 0 });
            expectEquals (t.transpose (64, PartKind::Acc), 64);
            expectEquals (t.transpose (60, PartKind::Bass), 60);
        }

        beginTest ("non-chord tones are PRESERVED at identity (not snapped to chord tones)");
        {
            ChordTransposer t;
            t.setOriginalChord ({ 0, ChordQuality::Maj, 0 });
            t.setActiveChord   ({ 0, ChordQuality::Maj, 0 });   // identity
            expectEquals (t.transpose (62, PartKind::Acc), 62);  // D stays D
            expectEquals (t.transpose (65, PartKind::Acc), 65);  // F stays F
            expectEquals (t.transpose (69, PartKind::Acc), 69);  // A stays A
            expectEquals (t.transpose (70, PartKind::Acc), 70);  // Bb stays Bb
        }

        beginTest ("melodic/passing notes shift by the root interval (kept relative to the root)");
        {
            ChordTransposer t;
            t.setOriginalChord ({ 0, ChordQuality::Maj, 0 });   // C
            t.setActiveChord   ({ 2, ChordQuality::Maj, 2 });   // D (+2)
            expectEquals (t.transpose (62, PartKind::Acc), 64);  // D (2nd) -> E (2nd above D)
            expectEquals (t.transpose (65, PartKind::Acc), 67);  // F (4th) -> G (4th above D)
            expectEquals (t.transpose (69, PartKind::Acc), 71);  // A (6th) -> B (6th above D)
        }

        beginTest ("only the 3rd flips on major->minor; other notes just shift");
        {
            ChordTransposer t;
            t.setOriginalChord ({ 0, ChordQuality::Maj, 0 });   // C major
            t.setActiveChord   ({ 0, ChordQuality::Min, 0 });   // C minor (same root)
            expectEquals (t.transpose (64, PartKind::Acc), 63);  // E (maj 3rd) -> Eb (min 3rd)
            expectEquals (t.transpose (60, PartKind::Acc), 60);  // C (root) unchanged
            expectEquals (t.transpose (67, PartKind::Acc), 67);  // G (5th) unchanged
            expectEquals (t.transpose (62, PartKind::Acc), 62);  // D (2nd passing) unchanged
            expectEquals (t.transpose (65, PartKind::Acc), 65);  // F (4th passing) unchanged
        }

        beginTest ("NTT maps the 3rd differently from root/5th (C maj -> A min)");
        {
            ChordTransposer t;
            t.setOriginalChord ({ 0, ChordQuality::Maj, 0 });   // C E G
            t.setActiveChord   ({ 9, ChordQuality::Min, 9 });   // A C E
            expectEquals (t.transpose (60, PartKind::Acc) % 12, 9);   // root C -> A
            expectEquals (t.transpose (64, PartKind::Acc) % 12, 0);   // 3rd  E -> C (minor 3rd, not C#)
            expectEquals (t.transpose (67, PartKind::Acc) % 12, 4);   // 5th  G -> E
        }

        beginTest ("block-shift case (same quality) shifts by root interval");
        {
            ChordTransposer t;
            t.setOriginalChord ({ 0, ChordQuality::Maj, 0 });   // C
            t.setActiveChord   ({ 2, ChordQuality::Maj, 2 });   // D (+2)
            expectEquals (t.transpose (60, PartKind::Acc) % 12, 2);   // C -> D
            expectEquals (t.transpose (64, PartKind::Acc) % 12, 6);   // E -> F#
            expectEquals (t.transpose (67, PartKind::Acc) % 12, 9);   // G -> A
        }

        beginTest ("result stays in 0..127");
        {
            ChordTransposer t;
            t.setOriginalChord ({ 0, ChordQuality::Maj, 0 });
            t.setActiveChord   ({ 7, ChordQuality::Dom7, 7 });
            for (int n = 0; n <= 127; ++n)
            {
                const int x = t.transpose (n, PartKind::Acc);
                expect (x >= 0 && x <= 127);
            }
        }

        beginTest ("bass inversion targets the played bass note for the bass root");
        {
            ChordTransposer t;
            t.setOriginalChord ({ 0, ChordQuality::Maj, 0 });   // C
            t.setActiveChord   ({ 0, ChordQuality::Maj, 4 });   // C major, bass = E (C/E)
            t.setBassInversion (true);
            expectEquals (t.transpose (36, PartKind::Bass) % 12, 4);   // bass root -> E
            // with inversion off, the bass root stays the chord root C
            t.setBassInversion (false);
            expectEquals (t.transpose (36, PartKind::Bass) % 12, 0);   // -> C
        }
    }
};
static ChordTransposerTest chordTransposerTest;
