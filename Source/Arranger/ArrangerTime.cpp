#include "ArrangerTime.h"
#include <cmath>

namespace ArrangerTime
{
    double secondsToBeats (double seconds, double bpm)
    {
        if (bpm <= 0.0) bpm = 120.0;
        return seconds * (bpm / 60.0);
    }

    double beatsToSeconds (double beats, double bpm)
    {
        if (bpm <= 0.0) bpm = 120.0;
        return beats * (60.0 / bpm);
    }

    double beatsPerBar (int timeSigNum, int timeSigDenom)
    {
        if (timeSigNum <= 0)   timeSigNum = 4;
        if (timeSigDenom <= 0) timeSigDenom = 4;
        return (double) timeSigNum * (4.0 / (double) timeSigDenom);
    }

    int barsForBeats (double beats, double beatsPerBarValue)
    {
        if (beatsPerBarValue <= 0.0 || beats <= 0.0) return 0;
        const double bars = beats / beatsPerBarValue;
        return (int) std::ceil (bars - 1e-6);
    }
}
