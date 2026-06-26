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

ArrangerChord ChordDetector::recognizeSingleFinger (const std::set<int>& notes)
{
    // Korg "One Finger" / Yamaha "Single Finger": the highest held note is the root; keys to its left
    // pick the quality by colour. root alone = Major; + white-left = Dom7; + black-left = Min;
    // + white & black-left = Min7.
    if (notes.empty()) return {};

    const int root   = *notes.rbegin();          // highest held note is the root
    const int rootPc = ((root % 12) + 12) % 12;

    auto isBlack = [] (int n)
    {
        const int pc = ((n % 12) + 12) % 12;
        return pc == 1 || pc == 3 || pc == 6 || pc == 8 || pc == 10;
    };

    bool hasWhiteBelow = false, hasBlackBelow = false;
    for (int n : notes)
        if (n < root) { (isBlack (n) ? hasBlackBelow : hasWhiteBelow) = true; }

    const ChordQuality q = (hasWhiteBelow && hasBlackBelow) ? ChordQuality::Min7
                         : hasBlackBelow                    ? ChordQuality::Min
                         : hasWhiteBelow                    ? ChordQuality::Dom7
                         :                                    ChordQuality::Maj;

    return { rootPc, q, rootPc };   // bassNote = root (no slash in single-finger)
}

void ChordDetector::recompute()
{
    if (mode != ChordMode::FullKeyboard)
    {
        // Split-zone modes: Fingered matches every held note as a chord tone; Single-Finger uses the
        // Korg one-finger convention (1-2 keys). Empty -> no chord.
        recognized = (mode == ChordMode::SingleFinger)
                       ? recognizeSingleFinger (heldNotes)
                       : recognizeSet (heldNotes);
        return;
    }

    // Full-Keyboard mode: only update on >=3 notes forming a known chord; otherwise hold the last
    // recognized chord so melody notes don't lurch the accompaniment.
    if ((int) heldNotes.size() < 3)
        return;   // keep the previously recognized chord (hysteresis / hold)

    // Favour the bass hand: try the lowest 3 (the core triad), then the lowest 4 (a bass 7th), then
    // the full set as a last resort. The first valid match wins, so a melody played on top is ignored.
    // Pure and instant (no debounce); stability comes from order + hysteresis, not from delay.
    auto lowest = [this] (int take)
    {
        std::set<int> s;
        for (int n : heldNotes) { s.insert (n); if ((int) s.size() == take) break; }
        return s;
    };

    for (int take : { 3, 4 })
    {
        ArrangerChord c = recognizeSet (lowest (take));
        if (c.isValid()) { recognized = c; return; }
    }
    ArrangerChord cFull = recognizeSet (heldNotes);
    if (cFull.isValid())
        recognized = cFull;
    // else: hold the previously recognized chord (hysteresis)
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
