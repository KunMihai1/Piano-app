#pragma once

/** Pure musical-time conversions. A "beat" is one quarter note. */
namespace ArrangerTime
{
    double secondsToBeats (double seconds, double bpm);
    double beatsToSeconds (double beats, double bpm);

    /** Quarter-note beats per bar for a time signature, e.g. 6/8 -> 3.0. */
    double beatsPerBar (int timeSigNum, int timeSigDenom);

    /** Whole bars needed to contain `beats`, rounded up (with fp tolerance). 0 -> 0. */
    int barsForBeats (double beats, double beatsPerBar);
}
