#include "ChordTransposer.h"
#include <juce_core/juce_core.h>
#include <vector>
#include <cmath>

namespace
{
    // Index (in the chord's interval list) of the chord tone nearest the given pitch class.
    int nearestChordToneIndex (int pc, int root, const std::vector<int>& iv)
    {
        int best = 0, bestDist = 99;
        for (int k = 0; k < (int) iv.size(); ++k)
        {
            const int tonePc = (root + iv[k]) % 12;
            int d = std::abs (tonePc - pc);
            d = std::min (d, 12 - d);
            if (d < bestDist) { bestDist = d; best = k; }
        }
        return best;
    }
}

int ChordTransposer::transpose (int noteNumber, PartKind part) const
{
    if (part == PartKind::Fixed || ! active.isValid() || ! original.isValid())
        return noteNumber;

    const auto origIv = chordIntervals (original.quality);
    const auto actIv  = chordIntervals (active.quality);
    const int pc = noteNumber % 12;

    // The note's role = which chord tone of the ORIGINAL chord it is closest to.
    const int role = nearestChordToneIndex (pc, original.root, origIv);

    int targetPc;
    if (part == PartKind::Bass && bassInversion && role == 0 && active.bassNote >= 0)
    {
        // Bass inversion: the bass root follows the played bass note (slash chords).
        targetPc = active.bassNote;
    }
    else
    {
        const int actIndex = juce::jmin (role, (int) actIv.size() - 1);
        targetPc = (active.root + actIv[actIndex]) % 12;
    }

    // Reattach the original register, then move by whole octaves to land nearest the source note.
    int candidate = (noteNumber / 12) * 12 + targetPc;
    while (candidate - noteNumber >  6) candidate -= 12;
    while (noteNumber - candidate >  6) candidate += 12;
    while (candidate < 0)   candidate += 12;
    while (candidate > 127) candidate -= 12;
    return candidate;
}
