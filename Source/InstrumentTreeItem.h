/*
  ==============================================================================

    InstrumentTreeItem.h
    Created: 23 May 2025 11:08:19pm
    Author:  Kisuke

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

class TreeViewHolder: public juce::Component {
public:
    explicit TreeViewHolder(juce::TreeView* treeToShow);

    void paint(juce::Graphics& g) override;

    void resized() override;

private:
    juce::TreeView* toShow;
};

class InstrumentTreeItem : public juce::TreeViewItem
{
public:
    std::function<void(int, juce::String)> onProgramSelected;

    InstrumentTreeItem(const juce::Image& img, const juce::String& name="", int program = -1);
    InstrumentTreeItem(const juce::String& name = "", int program = -1);

    //int InstrumentTreeItem::getPreferredWidth() const override;
    bool mightContainSubItems() override;
    void paintItem(juce::Graphics& g, int width, int height) override;
    int getItemWidth() const override;

    std::unique_ptr<juce::Component> createItemComponent() override;
    juce::String getUniqueName() const override;
    void itemClicked(const juce::MouseEvent& e) override;
    ~InstrumentTreeItem() override;

private:
    bool isSelectable();
    bool shouldHiglight=true;
    int program;
    juce::Image img;
    juce::String instrumentName;
    juce::Rectangle<int> textBounds;
};
