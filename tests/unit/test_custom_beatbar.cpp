#include <juce_core/juce_core.h>
#include "CustomBeatBar.h"

class BeatBarTest : public juce::UnitTest
{
public:
    BeatBarTest() : juce::UnitTest("BeatBar", "Unit") {}

    void runTest() override
    {
        beginTest("Default constructor values");
        {
            BeatBar bar;
            expect(bar.getNumerator() == 4);
            expect(bar.getDenominator() == 4);
        }

        beginTest("Set and get numerator");
        {
            BeatBar bar;

            bar.setNumerator(3);
            expect(bar.getNumerator() == 3);

            bar.setNumerator(7);
            expect(bar.getNumerator() == 7);
        }

        beginTest("Set and get denominator");
        {
            BeatBar bar;

            bar.setDenominator(8);
            expect(bar.getDenominator() == 8);

            bar.setDenominator(4);
            expect(bar.getDenominator() == 4);
        }

        beginTest("Sub-beats per beat calculation");
        {
            BeatBar bar;

            bar.setDenominator(4);
            expect(bar.getSubBeatsPerBeat() == 1.0f);

            bar.setDenominator(8);
            expect(bar.getSubBeatsPerBeat() == 2.0f);

            bar.setDenominator(16);
            expect(bar.getSubBeatsPerBeat() == 4.0f);
        }

        beginTest("Total subdivisions calculation");
        {
            BeatBar bar;

            bar.setNumerator(4);
            bar.setDenominator(4);
            expect(bar.getTotalSubdivisions() == 4);

            bar.setNumerator(3);
            bar.setDenominator(4);
            expect(bar.getTotalSubdivisions() == 3);

            bar.setNumerator(7);
            bar.setDenominator(8);
            expect(bar.getTotalSubdivisions() == 14);

            bar.setNumerator(6);
            bar.setDenominator(8);
            expect(bar.getTotalSubdivisions() == 12);
        }

        beginTest("Current subdivision in 4/4");
        {
            BeatBar bar;

            bar.setNumerator(4);
            bar.setDenominator(4);

            bar.setCurrentBeatsElapsed(0.0);
            expect(bar.getCurrentSubdivision() == 0);

            bar.setCurrentBeatsElapsed(1.2);
            expect(bar.getCurrentSubdivision() == 1);

            bar.setCurrentBeatsElapsed(2.9);
            expect(bar.getCurrentSubdivision() == 2);
        }

        beginTest("Subdivision wraps correctly across bars");
        {
            BeatBar bar;

            bar.setNumerator(4);
            bar.setDenominator(4);

            bar.setCurrentBeatsElapsed(4.0);
            expect(bar.getCurrentSubdivision() == 0);

            bar.setCurrentBeatsElapsed(5.0);
            expect(bar.getCurrentSubdivision() == 1);
        }

        beginTest("Negative beat values wrap correctly");
        {
            BeatBar bar;

            bar.setNumerator(4);
            bar.setDenominator(4);

            bar.setCurrentBeatsElapsed(-1.0);
            expect(bar.getCurrentSubdivision() == 3);
        }

        beginTest("Subdivision calculation in 7/8");
        {
            BeatBar bar;

            bar.setNumerator(7);
            bar.setDenominator(8);

            bar.setCurrentBeatsElapsed(0.0);
            expect(bar.getCurrentSubdivision() == 0);

            bar.setCurrentBeatsElapsed(1.0);
            expect(bar.getCurrentSubdivision() == 2);

            bar.setCurrentBeatsElapsed(3.0);
            expect(bar.getCurrentSubdivision() == 6);
        }

        beginTest("Paint executes without crash - playing off");
        {
            BeatBar bar;

            juce::Image img(juce::Image::ARGB, 200, 50, true);
            juce::Graphics g(img);

            bar.paint(g);

            expect(true);
        }

        beginTest("Paint executes red branch - first beat playing");
        {
            BeatBar bar;

            bar.setCurrentBeatsElapsed(0.0);
            bar.setNumerator(4);
            bar.setDenominator(4);     
            

            bar.isPlayingCheck = []() { return true; };

            juce::Image img(juce::Image::ARGB, 200, 50, true);
            juce::Graphics g(img);

            bar.paint(g);

            expect(true);
        }
    }
};

static BeatBarTest beatBarTest;