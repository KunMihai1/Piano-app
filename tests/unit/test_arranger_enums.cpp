#include <juce_core/juce_core.h>
#include "Arranger/ArrangerEnums.h"

class ArrangerEnumsTest : public juce::UnitTest
{
public:
    ArrangerEnumsTest() : juce::UnitTest ("ArrangerEnums", "Arranger") {}

    void runTest() override
    {
        beginTest ("section type round-trips through string");
        {
            for (auto t : { ArrangerSectionType::Intro, ArrangerSectionType::Variation,
                            ArrangerSectionType::Fill, ArrangerSectionType::Break,
                            ArrangerSectionType::Ending, ArrangerSectionType::CountIn })
                expect (ArrangerEnums::sectionTypeFromString (ArrangerEnums::toString (t)) == t);
        }

        beginTest ("afterComplete round-trips through string");
        {
            for (auto a : { ArrangerAfterComplete::Loop, ArrangerAfterComplete::FallThrough,
                            ArrangerAfterComplete::Stop })
                expect (ArrangerEnums::afterFromString (ArrangerEnums::toString (a)) == a);
        }

        beginTest ("partType round-trips through string");
        {
            for (auto p : { ArrangerPartType::Drum, ArrangerPartType::Perc,
                            ArrangerPartType::Bass, ArrangerPartType::Acc })
                expect (ArrangerEnums::partFromString (ArrangerEnums::toString (p)) == p);
        }

        beginTest ("unknown strings fall back to safe defaults");
        {
            expect (ArrangerEnums::sectionTypeFromString ("nope") == ArrangerSectionType::Variation);
            expect (ArrangerEnums::afterFromString ("nope")       == ArrangerAfterComplete::Loop);
            expect (ArrangerEnums::partFromString ("nope")        == ArrangerPartType::Acc);
        }
    }
};
static ArrangerEnumsTest arrangerEnumsTest;
