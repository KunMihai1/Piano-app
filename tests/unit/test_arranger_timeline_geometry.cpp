#include <juce_gui_basics/juce_gui_basics.h>
#include "Arranger/ArrangerTimelineGeometry.h"

class ArrangerTimelineGeometryTest : public juce::UnitTest
{
public:
    ArrangerTimelineGeometryTest() : juce::UnitTest ("ArrangerTimelineGeometry", "Arranger") {}

    void runTest() override
    {
        using namespace ArrangerTimelineGeometry;
        // 8 bars across a 820px-wide view with 10px left/right margins -> 800 usable -> 100px/bar.
        Layout L { 8, 820, 10 };

        beginTest ("bar 1 maps to the left margin, bar 9 (end) to the right edge");
        {
            expectWithinAbsoluteError (barToX (1.0, L), 10.0, 1e-6);
            expectWithinAbsoluteError (barToX (9.0, L), 810.0, 1e-6); // 8 bars later
            expectWithinAbsoluteError (barToX (5.0, L), 410.0, 1e-6); // 4 bars in
        }

        beginTest ("x snaps to the nearest bar line, clamped to [1, totalBars+1]");
        {
            expectEquals (xToSnappedBar (10.0,  L), 1);
            expectEquals (xToSnappedBar (59.0,  L), 1);  // <50px from bar1 line
            expectEquals (xToSnappedBar (61.0,  L), 2);  // >50px -> bar2
            expectEquals (xToSnappedBar (-100.0, L), 1); // clamp low
            expectEquals (xToSnappedBar (9999.0, L), 9); // clamp high (totalBars+1)
        }

        beginTest ("region bounds span its bars");
        {
            auto r = regionBounds (5, 2, L, 40); // startBar 5, len 2, height 40
            expectEquals (r.getX(), 410);
            expectEquals (r.getWidth(), 200); // 2 bars * 100px
            expectEquals (r.getHeight(), 40);
        }

        beginTest ("hit-test detects left edge, right edge, body, and miss");
        {
            auto r = regionBounds (5, 2, L, 40); // x 410..610
            expect (hitTest (412, r) == Hit::LeftEdge);
            expect (hitTest (608, r) == Hit::RightEdge);
            expect (hitTest (500, r) == Hit::Body);
            expect (hitTest (700, r) == Hit::None);
        }
    }
};
static ArrangerTimelineGeometryTest arrangerTimelineGeometryTest;
