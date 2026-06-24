#pragma once
#include "Chord.h"

/**
 * Note Transposition Table (NTT): maps each accompaniment note from the recorded (original) chord
 * onto the correct tone of the played (active) chord, by the note's scale-degree role. Drum/Perc
 * (PartKind::Fixed) pass through untouched. Pure: no JUCE GUI/threads; only read on the engine's
 * timer thread.
 */
class ChordTransposer
{
public:
    void setOriginalChord (ArrangerChord recorded) { original = recorded; }
    void setActiveChord   (ArrangerChord played)   { active = played; }
    void setBassInversion (bool shouldInvert) { bassInversion = shouldInvert; }

    /** Map one note for a part. Fixed parts and invalid chords return the note unchanged. */
    int transpose (int noteNumber, PartKind part) const;

private:
    ArrangerChord original { 0, ChordQuality::Maj, 0 };   // default C major
    ArrangerChord active;
    bool  bassInversion = false;
};
