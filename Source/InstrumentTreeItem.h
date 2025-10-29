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
 * @class TreeViewHolder
 * @brief Wrapper component to display a TreeView.
 */
class TreeViewHolder: public juce::Component {
public:
    /**
     * @brief Constructor
     * @param treeToShow Pointer to the TreeView to display
     */
    explicit TreeViewHolder(juce::TreeView* treeToShow);

    /** @brief Paints the background of the component */
    void paint(juce::Graphics& g) override;

    /** @brief Lays out the TreeView within this component */
    void resized() override;

private:
    juce::TreeView* toShow; ///< Pointer to the TreeView
};

/**
 * @class InstrumentTreeItem
 * @brief Represents an item in the instrument selection tree.
 *
 * Allows selection of instruments by program number and optionally displays an image.
 */
class InstrumentTreeItem : public juce::TreeViewItem
{
public:
    std::function<void(int, juce::String)> onProgramSelected; ///< Callback when a program is selected

    /**
     * @brief Constructor with image
     * @param img Image to display with the item
     * @param name Instrument name
     * @param program MIDI program number (-1 if not selectable)
     */
    InstrumentTreeItem(const juce::Image& img, const juce::String& name="", int program = -1);

    /**
     * @brief Constructor without image
     * @param name Instrument name
     * @param program MIDI program number (-1 if not selectable)
     */
    InstrumentTreeItem(const juce::String& name = "", int program = -1);

    /** @brief Returns true if the item has sub-items */
    bool mightContainSubItems() override;

    /** @brief Paints the item */
    void paintItem(juce::Graphics& g, int width, int height) override;

    /** @brief Returns the width of the item in pixels */
    int getItemWidth() const override;

    /** @brief Creates a custom component for the item (not used here) */
    std::unique_ptr<juce::Component> createItemComponent() override;

    /** @brief Returns a unique name for the item */
    juce::String getUniqueName() const override;

    /** @brief Handles mouse clicks on the item */
    void itemClicked(const juce::MouseEvent& e) override;

    /** @brief Destructor */
    ~InstrumentTreeItem() override;

private:
    /** @brief Returns true if this item is selectable */
    bool isSelectable();

    bool shouldHiglight = true; ///< Whether to highlight the item
    int program; ///< MIDI program number
    juce::Image img; ///< Optional image
    juce::String instrumentName; ///< Name of the instrument
    juce::Rectangle<int> textBounds; ///< Bounds of the text for click detection
};
