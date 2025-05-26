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

bool InstrumentTreeItem::isSelectable()
{
    return instrumentName != "Instruments" && instrumentName != "Pianos" && instrumentName != "Basses" && instrumentName != "Guitars" && 
        instrumentName != "WoodWinds" && instrumentName!="Brass" && instrumentName!="Strings" && instrumentName!="Reeds" && instrumentName!="Organs";
}

void InstrumentTreeItem::paintItem(juce::Graphics& g, int width, int height)
{
    if (shouldHiglight)
    {
        if (isSelected() && isSelectable())
            g.setColour(juce::Colours::goldenrod);
        else g.setColour(juce::Colours::black);
    }
    //g.fillRect(0, 0, width, height);


    g.drawText(instrumentName, 4, 0, width - 4, height, juce::Justification::centredLeft);
    if (textBounds.getX() == 0)
    {
        textBounds.setX(4);
        textBounds.setY(0);
        textBounds.setHeight(height);
        textBounds.setWidth(width);
    }
    shouldHiglight = true;
}

int InstrumentTreeItem::getItemWidth() const
{
    juce::Font font;
    int textWidth = font.getStringWidth(instrumentName);
    int padding = 12;
    return textWidth + padding;
}

std::unique_ptr<juce::Component> InstrumentTreeItem::createItemComponent()
{
    return nullptr;
}

juce::String InstrumentTreeItem::getUniqueName() const
{
    return instrumentName;
}

void InstrumentTreeItem::itemClicked(const juce::MouseEvent& e)
{
    auto clickPos = e.getPosition();
    //DBG("ccc" << clickPos.toString() << " " << textBounds.toString());
    if (program >= 0 )
    {
        if (clickPos.getX() >= textBounds.getX() && clickPos.getX() <= textBounds.getX() + textBounds.getWidth() &&
            clickPos.getY() >= textBounds.getY() && clickPos.getY() <= textBounds.getY() + textBounds.getHeight() &&
            onProgramSelected)
        {
            onProgramSelected(program,instrumentName);
            shouldHiglight = true;
        }
        else shouldHiglight = false;
    }
    repaintItem();
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



