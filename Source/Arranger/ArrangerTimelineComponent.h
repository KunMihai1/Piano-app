#pragma once
#include <JuceHeader.h>
#include "ArrangerStyleFile.h"
#include "ArrangerTimelineGeometry.h"
#include <vector>
#include <functional>

/** Bar ruler with draggable/resizable colored section regions. Holds a copy of the
    windows; emits a callback whenever they change. Knows nothing about files or the engine. */
class ArrangerTimelineComponent : public juce::Component, public juce::TooltipClient
{
public:
    std::function<void (const std::vector<SectionWindow>&)> onWindowsChanged;
    std::function<void (int sectionIndex)> onSectionSelected;

    /** Tooltip naming the section under the mouse (e.g. "Intro 1  (bars 1-2)"). */
    juce::String getTooltip() override;

    void setTotalBars (int bars);
    void setWindows (const std::vector<SectionWindow>& windows);
    const std::vector<SectionWindow>& getWindows() const { return windows; }

    void setPlayheadBar (double bar);   // for the live preview cursor (1-based, fractional)
    int  getSelectedIndex() const { return selectedIndex; }

    /** Fixed bar width: the component is made this wide per bar and scrolled in a Viewport,
        so bar labels stay readable no matter how many bars there are. */
    static constexpr int kPixelsPerBar = 56;
    int getPreferredWidth() const;     // marginPx*2 + totalBars * kPixelsPerBar
    int xForBar (double bar) const;    // pixel x of a (fractional, 1-based) bar in this component

    void paint (juce::Graphics& g) override;
    void mouseDown (const juce::MouseEvent& e) override;
    void mouseDrag (const juce::MouseEvent& e) override;
    void mouseUp   (const juce::MouseEvent& e) override;

private:
    ArrangerTimelineGeometry::Layout layout() const;
    juce::Colour colourForType (ArrangerSectionType t) const;

    std::vector<SectionWindow> windows;
    int    totalBars   = 1;
    int    marginPx    = 10;
    double playheadBar = 1.0;

    int  selectedIndex = -1;
    ArrangerTimelineGeometry::Hit dragMode = ArrangerTimelineGeometry::Hit::None;
    int  dragGrabOffsetBars = 0;   // bars between the grab point and the region start (body drag)
    int  dragStartLenValue = 1;
    bool windowsChangedByDrag = false;  // true once a drag actually moved/resized a window this gesture
};
