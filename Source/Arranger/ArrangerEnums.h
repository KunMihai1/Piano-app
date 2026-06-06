#pragma once
#include <JuceHeader.h>
#include "ArrangerModel.h"

namespace ArrangerEnums
{
    inline juce::String toString (ArrangerSectionType t)
    {
        switch (t)
        {
            case ArrangerSectionType::Intro:     return "Intro";
            case ArrangerSectionType::Variation: return "Variation";
            case ArrangerSectionType::Fill:      return "Fill";
            case ArrangerSectionType::Break:     return "Break";
            case ArrangerSectionType::Ending:    return "Ending";
            case ArrangerSectionType::CountIn:   return "CountIn";
        }
        return "Variation";
    }

    inline ArrangerSectionType sectionTypeFromString (const juce::String& s)
    {
        if (s == "Intro")   return ArrangerSectionType::Intro;
        if (s == "Fill")    return ArrangerSectionType::Fill;
        if (s == "Break")   return ArrangerSectionType::Break;
        if (s == "Ending")  return ArrangerSectionType::Ending;
        if (s == "CountIn") return ArrangerSectionType::CountIn;
        return ArrangerSectionType::Variation;
    }

    inline juce::String toString (ArrangerAfterComplete a)
    {
        switch (a)
        {
            case ArrangerAfterComplete::Loop:        return "Loop";
            case ArrangerAfterComplete::FallThrough: return "FallThrough";
            case ArrangerAfterComplete::Stop:        return "Stop";
        }
        return "Loop";
    }

    inline ArrangerAfterComplete afterFromString (const juce::String& s)
    {
        if (s == "FallThrough") return ArrangerAfterComplete::FallThrough;
        if (s == "Stop")        return ArrangerAfterComplete::Stop;
        return ArrangerAfterComplete::Loop;
    }

    inline juce::String toString (ArrangerPartType p)
    {
        switch (p)
        {
            case ArrangerPartType::Drum: return "Drum";
            case ArrangerPartType::Perc: return "Perc";
            case ArrangerPartType::Bass: return "Bass";
            case ArrangerPartType::Acc:  return "Acc";
        }
        return "Acc";
    }

    inline ArrangerPartType partFromString (const juce::String& s)
    {
        if (s == "Drum") return ArrangerPartType::Drum;
        if (s == "Perc") return ArrangerPartType::Perc;
        if (s == "Bass") return ArrangerPartType::Bass;
        return ArrangerPartType::Acc;
    }
}
