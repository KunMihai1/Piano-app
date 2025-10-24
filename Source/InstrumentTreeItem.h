/*
  ==============================================================================

    InstrumentTreeItem.h
    Created: 23 May 2025 11:08:19pm
    Author:  Kisuke

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

/**
 * @brief A simple container for displaying a TreeView in a Component.
 *
 * @details Wraps a juce::TreeView pointer and handles basic resizing and painting.
 */
class TreeViewHolder: public juce::Component {
public:

    /** 
     * @brief Construct a TreeViewHolder for the given TreeView.
     * @param treeToShow Pointer to the TreeView to display.
     */
    explicit TreeViewHolder(juce::TreeView* treeToShow);


     /** @brief Paints the background of the holder. */
    void paint(juce::Graphics& g) override;


     /** @brief Resizes the TreeView to fill this component. */
    void resized() override;

private:
    juce::TreeView* toShow;
};


/**
 * @brief A custom TreeViewItem for displaying instruments and categories.
 *
 * @details Supports optional images, highlighting, and program selection callback.
 */
class InstrumentTreeItem : public juce::TreeViewItem
{
public:

    /** Callback called when a MIDI program is selected. */
    std::function<void(int, juce::String)> onProgramSelected;


    /** Constructor with image. */
    InstrumentTreeItem(const juce::Image& img, const juce::String& name="", int program = -1);

    /** Constructor without image. */
    InstrumentTreeItem(const juce::String& name = "", int program = -1);


    /** @brief Returns true if this item has sub-items. */
    bool mightContainSubItems() override;

    /** @brief Paints the item. */
    void paintItem(juce::Graphics& g, int width, int height) override;

    /** @brief Returns the width of the item. */
    int getItemWidth() const override;


    /** @brief Returns a component to display for this item (nullptr by default). */
    std::unique_ptr<juce::Component> createItemComponent() override;

    /** @brief Returns a unique name for the item (used internally by TreeView). */
    juce::String getUniqueName() const override;

    /** @brief Called when the item is clicked. Triggers program selection if applicable. */
    void itemClicked(const juce::MouseEvent& e) override;

    /** @brief Destructor */
    ~InstrumentTreeItem() override;

private:

    /** @brief Determines if the item is selectable (not a category). */
    bool isSelectable();

    bool shouldHiglight=true;
    int program;
    juce::Image img;
    juce::String instrumentName;
    juce::Rectangle<int> textBounds;
};
