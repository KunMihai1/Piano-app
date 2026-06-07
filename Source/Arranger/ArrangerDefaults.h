#pragma once
#include <JuceHeader.h>
#include "ArrangerStyleFile.h"
#include <vector>

namespace ArrangerDefaults
{
    /** Default windows for a freshly captured recording: one Variation across all bars.
        The user adds Intro/Fill/Ending by dragging on the timeline. */
    std::vector<SectionWindow> defaultWindowsForBars (int totalBars);
}
