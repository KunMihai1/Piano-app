#include "ArrangerDefaults.h"

namespace ArrangerDefaults
{
    std::vector<SectionWindow> defaultWindowsForBars (int totalBars)
    {
        SectionWindow v;
        v.id            = juce::Uuid().toString();
        v.name          = "Variation 1";
        v.type          = ArrangerSectionType::Variation;
        v.startBar      = 1;
        v.lengthBars    = juce::jmax (1, totalBars);
        v.afterComplete = ArrangerAfterComplete::Loop;
        return { v };
    }
}
