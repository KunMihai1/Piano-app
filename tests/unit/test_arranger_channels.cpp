#include <juce_core/juce_core.h>
#include "Arranger/ArrangerChannels.h"

class ArrangerChannelsTest : public juce::UnitTest
{
public:
    ArrangerChannelsTest() : juce::UnitTest ("ArrangerChannels", "Arranger") {}

    void runTest() override
    {
        beginTest ("melodic channels are 2..9 then 11..13, skipping reserved");
        {
            const int expected[] = { 2, 3, 4, 5, 6, 7, 8, 9, 11, 12, 13 };
            for (int i = 0; i < 11; ++i)
                expectEquals (ArrangerChannels::melodicChannelForIndex (i), expected[i]);
        }

        beginTest ("never returns a reserved channel");
        {
            for (int i = 0; i < 11; ++i)
            {
                const int ch = ArrangerChannels::melodicChannelForIndex (i);
                expect (ch != 1 && ch != 10 && ch != 14 && ch != 15 && ch != 16);
            }
        }

        beginTest ("overflow past 11 melodic tracks returns -1");
        {
            expectEquals (ArrangerChannels::melodicChannelForIndex (11), -1);
            expectEquals (ArrangerChannels::melodicChannelForIndex (50), -1);
        }
    }
};
static ArrangerChannelsTest arrangerChannelsTest;
