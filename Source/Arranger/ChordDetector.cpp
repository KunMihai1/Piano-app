#include "ChordDetector.h"
#include <array>

namespace
{
    struct Template { ChordQuality q; std::vector<int> intervals; };

    // Richer (4-note) templates first so a 4-note chord wins over its 3-note subset.
    const std::vector<Template>& templates()
    {
        static const std::vector<Template> t = {
            { ChordQuality::Maj7,    chordIntervals (ChordQuality::Maj7)    },
            { ChordQuality::Dom7,    chordIntervals (ChordQuality::Dom7)    },
            { ChordQuality::Min7,    chordIntervals (ChordQuality::Min7)    },
            { ChordQuality::HalfDim, chordIntervals (ChordQuality::HalfDim) },
            { ChordQuality::Maj,     chordIntervals (ChordQuality::Maj)     },
            { ChordQuality::Min,     chordIntervals (ChordQuality::Min)     },
            { ChordQuality::Dim,     chordIntervals (ChordQuality::Dim)     },
            { ChordQuality::Aug,     chordIntervals (ChordQuality::Aug)     },
            { ChordQuality::Sus2,    chordIntervals (ChordQuality::Sus2)    },
            { ChordQuality::Sus4,    chordIntervals (ChordQuality::Sus4)    },
        };
        return t;
    }

    bool matches (const std::set<int>& pcs, int root, const std::vector<int>& intervals)
    {
        if ((int) pcs.size() != (int) intervals.size()) return false;
        for (int iv : intervals)
            if (pcs.find ((root + iv) % 12) == pcs.end()) return false;
        return true;
    }
}

void ChordDetector::noteOn  (int n) { heldNotes.insert (n); recompute(); }
void ChordDetector::noteOff (int n) { heldNotes.erase  (n); recompute(); }

void ChordDetector::reset()
{
    heldNotes.clear();
    recognized = ArrangerChord{};
}

ArrangerChord ChordDetector::recognizeSet (const std::set<int>& notes)
{
    if (notes.empty()) return {};

    std::set<int> pcs;
    for (int n : notes) pcs.insert (n % 12);
    const int lowestPc = (*notes.begin()) % 12;   // std::set is ascending

    // Some pitch-class sets name two chords that are inversions of each other (e.g. C-F-G is both
    // Csus4 and Fsus2). Prefer the interpretation whose root IS the lowest (bass) note â€” the Fingered
    // convention. If no match is rooted on the bass (a true inversion like E-G-C = C major), fall back
    // to the highest-priority template match.
    ArrangerChord firstMatch { -1, ChordQuality::None, lowestPc };
    for (const auto& tpl : templates())
        for (int root = 0; root < 12; ++root)
            if (matches (pcs, root, tpl.intervals))
            {
                if (root == lowestPc)
                    return { root, tpl.q, lowestPc };
                if (! firstMatch.isValid())
                    firstMatch = { root, tpl.q, lowestPc };
            }

    return firstMatch;   // invalid if nothing matched
}

void ChordDetector::recompute()
{
    if (! fullKeyboard)
    {
        // Split mode: every held note is a chord tone -> match directly. Empty -> no chord.
        recognized = recognizeSet (heldNotes);
        return;
    }

    // Full-Keyboard mode: only update on >=3 notes forming a known chord; otherwise hold the last
    // recognized chord so melody notes don't lurch the accompaniment.
    if ((int) heldNotes.size() < 3)
        return;   // keep the previously recognized chord (hysteresis / hold)

    // Try the full set, then the lowest 4, then the lowest 3 (the chord is usually in the lower hand).
    ArrangerChord c = recognizeSet (heldNotes);
    if (! c.isValid())
    {
        for (int take : { 4, 3 })
        {
            std::set<int> lowest;
            for (int n : heldNotes) { lowest.insert (n); if ((int) lowest.size() == take) break; }
            c = recognizeSet (lowest);
            if (c.isValid()) break;
        }
    }

    if (c.isValid())
        recognized = c;   // else: hold the previous chord
}

ArrangerChord detectKeyFromEvents (const std::vector<TimedBeatEvent>& events)
{
    std::array<int, 12> hist {};
    for (const auto& e : events)
        if (e.message.isNoteOn() && e.message.getVelocity() > 0)
            hist[e.message.getNoteNumber() % 12]++;

    int total = 0, root = 0, best = -1;
    for (int i = 0; i < 12; ++i) { total += hist[i]; if (hist[i] > best) { best = hist[i]; root = i; } }
    if (total == 0) return { 0, ChordQuality::Maj, 0 };   // fallback C major

    const int majThird = hist[(root + 4) % 12];
    const int minThird = hist[(root + 3) % 12];
    const ChordQuality q = (minThird > majThird) ? ChordQuality::Min : ChordQuality::Maj;
    return { root, q, root };
}
