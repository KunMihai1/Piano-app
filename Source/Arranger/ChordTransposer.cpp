#include "ChordTransposer.h"
#include <juce_core/juce_core.h>

namespace
{
    // Semitones of the chord's third above the root: 3 = minor, 4 = major, 0 = no third (sus / none).
    int thirdSemitones (ChordQuality q)
    {
        switch (q)
        {
            case ChordQuality::Min: case ChordQuality::Min7:
            case ChordQuality::Dim: case ChordQuality::HalfDim: return 3;
            case ChordQuality::Maj: case ChordQuality::Dom7:
            case ChordQuality::Maj7: case ChordQuality::Aug:    return 4;
            default: return 0;   // Sus2, Sus4, None — no third to flip
        }
    }
}

int ChordTransposer::transpose (int noteNumber, PartKind part) const
{
    if (part == PartKind::Fixed || ! active.isValid() || ! original.isValid())
        return noteNumber;

    // Bass inversion (slash chords), the Korg/Yamaha way: re-base the WHOLE bass line onto the played
    // bass note. The bass pattern's shape is preserved, just rooted on the lowest fingered note instead
    // of the chord root — so a C/E voicing moves the entire bass line down to E.
    if (part == PartKind::Bass && bassInversion && active.bassNote >= 0)
    {
        int b = noteNumber + (active.bassNote - original.root);
        while (b < 0)   b += 12;
        while (b > 127) b -= 12;
        return b;
    }

    // Base: shift every note by the root interval. This preserves the recording's melody, passing
    // tones and voicing EXACTLY — only the pitch level moves. (Snapping notes to chord tones, as a
    // naive NTT would, destroys all non-chord-tone content; we deliberately do not do that.)
    int shifted = noteNumber + (active.root - original.root);

    // Quality adjustment: when the chord's third changes (major <-> minor), move ONLY the notes that
    // land on the third degree, so a major recording sounds correctly minor (and vice versa) without
    // disturbing the rest of the line. Chords with no third (sus) skip this.
    const int origThird = thirdSemitones (original.quality);
    const int actThird  = thirdSemitones (active.quality);
    if (origThird != 0 && actThird != 0 && origThird != actThird)
    {
        const int deg = ((shifted - active.root) % 12 + 12) % 12;   // semitones above the played root
        if (deg == origThird)
            shifted += (actThird - origThird);
    }

    // Keep within the valid MIDI range, preserving octave.
    while (shifted < 0)   shifted += 12;
    while (shifted > 127) shifted -= 12;
    return shifted;
}
