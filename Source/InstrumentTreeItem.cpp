/*
  ==============================================================================

    InstrumentTreeItem.cpp
    Created: 23 May 2025 11:08:19pm
    Author:  Kisuke

  ==============================================================================
*/

#include "InstrumentTreeItem.h"

InstrumentTreeItem::InstrumentTreeItem(const juce::Image& img, const juce::String& name, int program) : instrumentName{name}, program{program}, img{img}
{
}

InstrumentTreeItem::InstrumentTreeItem(const juce::String& name, int program): instrumentName{name}, program{program}
{
}

bool InstrumentTreeItem::mightContainSubItems()
{
    return getNumSubItems() > 0;
}

void InstrumentTreeItem::paintItem(juce::Graphics& g, int width, int height)
{
    if (isSelected())
        g.setColour(juce::Colours::goldenrod);
    else g.setColour(juce::Colours::black);
    g.drawText(instrumentName, 4, 0, width - 4, height, juce::Justification::centredLeft);
}

std::unique_ptr<juce::Component> InstrumentTreeItem::createItemComponent()
{
    if (img.isValid())
        return std::make_unique<CustomTreeItemComponent>(img, instrumentName,this);
    else
        return nullptr;
}

juce::String InstrumentTreeItem::getUniqueName() const
{
    return instrumentName;
}

void InstrumentTreeItem::itemClicked(const juce::MouseEvent& e)
{
    if (program >= 0)
    {
        if(onProgramSelected)
            onProgramSelected(program);
    }
}

InstrumentTreeItem::~InstrumentTreeItem()
{
    //clearSubItems();
}

TreeViewHolder::TreeViewHolder(juce::TreeView* treeToShow): toShow{treeToShow}
{
    addAndMakeVisible(toShow);
    setSize(600, 300);
}

void TreeViewHolder::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::white);
}

void TreeViewHolder::resized()
{
    toShow->setBounds(getLocalBounds());
    toShow->setIndentSize(12);
}

CustomTreeItemComponent::CustomTreeItemComponent(const juce::Image& img, const juce::String& text, juce::TreeViewItem* ownerItem): owner{ownerItem}
{
    icon.setImage(img.rescaled(16, 16, juce::Graphics::ResamplingQuality::highResamplingQuality));
    addAndMakeVisible(icon);
    nameLabel.setText("   "+ text, juce::dontSendNotification);
    addAndMakeVisible(nameLabel);
    nameLabel.setJustificationType(juce::Justification::centredRight);
}

void CustomTreeItemComponent::mouseDown(const juce::MouseEvent& e)
{

    if (owner)
    {
        owner->setSelected(true, true);
        owner->itemClicked(e);
    }
}

void CustomTreeItemComponent::resized()
{
    DBG("CustomTreeItemComponent width: " << getWidth());
    const int iconWidth = 16;
    const int padding = 4;

    icon.setBounds(0, 4, 16, 16);

    nameLabel.setBounds(150, 0, getWidth() - (iconWidth + padding), getHeight());
}
