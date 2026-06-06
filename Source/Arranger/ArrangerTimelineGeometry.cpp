#include "ArrangerTimelineGeometry.h"
#include <cmath>

namespace ArrangerTimelineGeometry
{
    double pixelsPerBar (const Layout& L)
    {
        const int usable = juce::jmax (1, L.widthPx - 2 * L.marginPx);
        return (double) usable / (double) juce::jmax (1, L.totalBars);
    }

    double barToX (double bar, const Layout& L)
    {
        return (double) L.marginPx + (bar - 1.0) * pixelsPerBar (L);
    }

    int xToSnappedBar (double x, const Layout& L)
    {
        const double bar = (x - (double) L.marginPx) / pixelsPerBar (L) + 1.0;
        const int snapped = (int) std::floor (bar + 0.5);
        return juce::jlimit (1, L.totalBars + 1, snapped);
    }

    juce::Rectangle<int> regionBounds (int startBar, int lengthBars, const Layout& L, int heightPx)
    {
        const int x  = (int) std::lround (barToX ((double) startBar, L));
        const int x2 = (int) std::lround (barToX ((double) (startBar + juce::jmax (1, lengthBars)), L));
        return juce::Rectangle<int> (x, 0, juce::jmax (1, x2 - x), heightPx);
    }

    Hit hitTest (int x, const juce::Rectangle<int>& region)
    {
        if (x < region.getX() - kEdgeGrabPx || x > region.getRight() + kEdgeGrabPx)
            return Hit::None;
        if (std::abs (x - region.getX())     <= kEdgeGrabPx) return Hit::LeftEdge;
        if (std::abs (x - region.getRight())  <= kEdgeGrabPx) return Hit::RightEdge;
        if (x >= region.getX() && x <= region.getRight())     return Hit::Body;
        return Hit::None;
    }
}
