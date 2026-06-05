#include <juce_core/juce_core.h>
#include "Arranger/ArrangerTime.h"

class ArrangerTimeTest : public juce::UnitTest
{
public:
    ArrangerTimeTest() : juce::UnitTest ("ArrangerTime", "Arranger") {}

    void runTest() override
    {
        beginTest ("seconds <-> beats at 120 BPM");
        {
            expectWithinAbsoluteError (ArrangerTime::secondsToBeats (1.0, 120.0), 2.0, 1e-9);
            expectWithinAbsoluteError (ArrangerTime::beatsToSeconds (2.0, 120.0), 1.0, 1e-9);
        }

        beginTest ("beats per bar");
        {
            expectWithinAbsoluteError (ArrangerTime::beatsPerBar (4, 4), 4.0, 1e-9);
            expectWithinAbsoluteError (ArrangerTime::beatsPerBar (3, 4), 3.0, 1e-9);
            expectWithinAbsoluteError (ArrangerTime::beatsPerBar (6, 8), 3.0, 1e-9); // 6 * 4/8
        }

        beginTest ("bars for beats rounds up to whole bars");
        {
            expectEquals (ArrangerTime::barsForBeats (8.0, 4.0), 2);
            expectEquals (ArrangerTime::barsForBeats (8.0001, 4.0), 3);
            expectEquals (ArrangerTime::barsForBeats (0.0, 4.0), 0);
            expectEquals (ArrangerTime::barsForBeats (7.9999, 4.0), 2); // tolerance swallows fp noise
        }
    }
};

static ArrangerTimeTest arrangerTimeTest;
