#pragma once
#include <juce_gui_basics/juce_gui_basics.h>

namespace ArrangerTimelineGeometry
{
    struct Layout
    {
        int totalBars = 1;
        int widthPx   = 0;
        int marginPx  = 0;   // equal left/right margin
    };

    enum class Hit { None, Body, LeftEdge, RightEdge };

    constexpr int kEdgeGrabPx = 6;

    double pixelsPerBar (const Layout& L);
    double barToX (double bar, const Layout& L);            // bar is 1-based, may be fractional
    int    xToSnappedBar (double x, const Layout& L);       // nearest bar line, clamp [1, totalBars+1]
    juce::Rectangle<int> regionBounds (int startBar, int lengthBars, const Layout& L, int heightPx);
    Hit    hitTest (int x, const juce::Rectangle<int>& region);
}
