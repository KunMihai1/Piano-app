#include <juce_core/juce_core.h>
#include <juce_audio_basics/juce_audio_basics.h>
#include "Arranger/ArrangerModel.h"

class ArrangerModelTest : public juce::UnitTest
{
public:
    ArrangerModelTest() : juce::UnitTest ("ArrangerModel", "Arranger") {}

    void runTest() override
    {
        beginTest ("defaults and assembly");
        {
            ArrangerStyle style;
            expect (style.schemaVersion == ARRANGER_SCHEMA_VERSION);
            expect (style.sections.empty());

            ArrangerTrack track;
            track.partType = ArrangerPartType::Bass;
            track.channel  = 2;
            track.pattern.push_back ({ 0.0, juce::MidiMessage::noteOn (2, 60, (juce::uint8) 100) });

            ArrangerSection section;
            section.type = ArrangerSectionType::Variation;
            section.lengthBars = 2;
            section.tracks.push_back (track);

            style.sections.push_back (section);

            expectEquals ((int) style.sections.size(), 1);
            expectEquals (style.sections[0].lengthBars, 2);
            expectEquals ((int) style.sections[0].tracks[0].pattern.size(), 1);
            expect (style.sections[0].tracks[0].partType == ArrangerPartType::Bass);
        }

        beginTest ("schema v2 and after-complete defaults");
        {
            expectEquals (ARRANGER_SCHEMA_VERSION, 2);

            ArrangerSection s;
            expect (s.afterComplete == ArrangerAfterComplete::Loop); // sections loop unless told otherwise

            s.afterComplete = ArrangerAfterComplete::Stop;
            expect (s.afterComplete == ArrangerAfterComplete::Stop);
        }
    }
};

static ArrangerModelTest arrangerModelTest;
