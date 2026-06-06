#include <juce_core/juce_core.h>
#include "Arranger/ArrangerDefaults.h"

class ArrangerDefaultsTest : public juce::UnitTest
{
public:
    ArrangerDefaultsTest() : juce::UnitTest ("ArrangerDefaults", "Arranger") {}

    void runTest() override
    {
        beginTest ("default is a single Variation covering the whole recording");
        {
            auto w = ArrangerDefaults::defaultWindowsForBars (8);
            expectEquals ((int) w.size(), 1);
            expect (w[0].type == ArrangerSectionType::Variation);
            expectEquals (w[0].startBar, 1);
            expectEquals (w[0].lengthBars, 8);
            expect (w[0].afterComplete == ArrangerAfterComplete::Loop);
            expect (w[0].id.isNotEmpty());
        }

        beginTest ("zero or negative bars clamps to a 1-bar Variation");
        {
            auto w = ArrangerDefaults::defaultWindowsForBars (0);
            expectEquals ((int) w.size(), 1);
            expectEquals (w[0].lengthBars, 1);
        }
    }
};
static ArrangerDefaultsTest arrangerDefaultsTest;
