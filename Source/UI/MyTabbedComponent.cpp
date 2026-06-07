#include "displayGUI.h"

MyTabbedComponent::MyTabbedComponent(juce::TabbedButtonBar::Orientation orientation) : juce::TabbedComponent(orientation)
{
    getTabbedButtonBar().addMouseListener(this, true);

}

MyTabbedComponent::~MyTabbedComponent()
{
    getTabbedButtonBar().removeMouseListener(this);
}

void MyTabbedComponent::currentTabChanged(int newCurrentTabIndex, const juce::String& newTabName)
{
    if (onTabChanged)
        onTabChanged(newCurrentTabIndex, newTabName);
}

void MyTabbedComponent::mouseDown(const juce::MouseEvent& event)
{
    if (!event.mods.isRightButtonDown()) return;

    int numTabs = getNumTabs();
    if (numTabs <= 1) return;

    int tabWidth = getWidth() / numTabs;
    int clickedIndex = event.x / tabWidth;

    if (clickedIndex > 0 && clickedIndex < numTabs)
    {
        juce::PopupMenu menu;
        menu.addItem("Close Tab", [this, clickedIndex]() { removeTab(clickedIndex); });
        menu.showMenuAsync(juce::PopupMenu::Options());
    }
}
