#pragma once
#include <JuceHeader.h>

/**
 * @class MyTabbedComponent
 * @brief Custom tabbed component that allows a callback when the current tab changes.
 *
 * Extends `juce::TabbedComponent` and adds a std::function callback
 * `onTabChanged` which is called whenever the current tab is changed.
 */
class MyTabbedComponent : public juce::TabbedComponent, private juce::MouseListener
{
public:

    /**
     * @brief Constructs a MyTabbedComponent with a specified tab orientation.
     * @param orientation Orientation of the tab bar (e.g., TabsAtTop, TabsAtLeft)
     */
    MyTabbedComponent(juce::TabbedButtonBar::Orientation orientation);

    ~MyTabbedComponent();

    /**
     * @brief Called by JUCE when the current tab changes.
     * @param newCurrentTabIndex Index of the newly selected tab
     * @param newTabName Name of the newly selected tab
     */
    void currentTabChanged(int newCurrentTabIndex, const juce::String& newTabName) override;

    /** @brief Callback function triggered whenever the current tab changes */
    std::function<void(int, juce::String)> onTabChanged;

    void mouseDown(const juce::MouseEvent& event) override;

    //JUCE_LEAK_DETECTOR(MyTabbedComponent)
};
