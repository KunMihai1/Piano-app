/*
  ==============================================================================

    ButtonGroupComponent.cpp
    Created: 30 Nov 2025 10:08:03pm
    Author:  Kisuke

  ==============================================================================
*/

#include "SectionGroupComponent.h"

SectionGroupComponent::SectionGroupComponent(const juce::String& title, const std::vector<juce::String>& buttonNames, bool toggleButtonBool) : titleLabel{ title,title }
{
    addAndMakeVisible(titleLabel);
    titleLabel.setJustificationType(juce::Justification::centred);
    titleLabel.setColour(juce::Label::textColourId, juce::Colours::white);

    for (auto& name : buttonNames)
    {
        auto btn = std::make_unique<juce::TextButton>(name);
        btn->setButtonText(name);
        btn->setMouseCursor(juce::MouseCursor::PointingHandCursor);

        addAndMakeVisible(*btn);
        buttons.push_back(std::move(btn));
    }

    if (toggleButtonBool)
    {
        toggleButton = std::make_unique<juce::ToggleButton>();
        toggleButton->setMouseCursor(juce::MouseCursor::PointingHandCursor);
        toggleButton->setToggleState(false, juce::dontSendNotification);

        toggleButton->setColour(juce::ToggleButton::tickColourId, juce::Colours::green);      
        toggleButton->setColour(juce::ToggleButton::tickDisabledColourId, juce::Colours::grey);

        toggleButton->onClick = [this]()
        {
            if (toggleToolTip->isVisible())
            {
                if (toggleButton->getToggleState())
                    toggleToolTip->setNewText("Disable Auto Fill");
                else
                    toggleToolTip->setNewText("Enable Auto Fill");
            }
        };

        addAndMakeVisible(toggleButton.get());

        toggleToolTip = std::make_unique<CustomToolTip>("Enable Auto Fill");
        addAndMakeVisible(toggleToolTip.get());
        toggleToolTip->setVisible(false);

        toggleButton->addMouseListener(this, false);
        
    }

    if (title.equalsIgnoreCase("variations"))
    {
        auto btn = std::make_unique<juce::TextButton>("Break");
        btn->setButtonText("Break");
        btn->setMouseCursor(juce::MouseCursor::PointingHandCursor);
        addAndMakeVisible(*btn);
        buttons.push_back(std::move(btn));
    }
}

void SectionGroupComponent::resized()
{

    int labelHeight = (int)titleLabel.getFont().getHeight() + 2;
    auto area = getLocalBounds();
    titleLabel.setBounds(area.removeFromTop(labelHeight));

    area.removeFromTop(5);

    int count = (int)buttons.size();
    if (count == 0)
        return;
    
    int toDivideWith = count;
    if (titleLabel.getText().equalsIgnoreCase("fills"))
        toDivideWith++;

    auto row = area;
    int width = row.getWidth() / toDivideWith;

    for (int i = 0; i < count; i++)
    {
        buttons[i]->setBounds(row.removeFromLeft(width));
    }

    if (titleLabel.getText().equalsIgnoreCase("fills"))
        toggleButton->setBounds(row.removeFromLeft(width));

}

void SectionGroupComponent::mouseEnter(const juce::MouseEvent& e)
{
    if (e.eventComponent == toggleButton.get())
    {
        auto bounds = toggleButton->getBounds();

        int tooltipX = bounds.getX() - toggleToolTip->getWidth(); 
        int tooltipY = bounds.getY() + (bounds.getHeight() - toggleToolTip->getHeight()) / 2 - 20; 

        toggleToolTip->setTopLeftPosition(tooltipX, tooltipY);
        

        toggleToolTip->setVisible(true);
        toggleToolTip->toFront(true);
    }
}

void SectionGroupComponent::mouseExit(const juce::MouseEvent& e)
{
    if (e.eventComponent == toggleButton.get())
    {
        toggleToolTip->setVisible(false);
    }
}

SectionGroupComponent::~SectionGroupComponent()
{
    if (toggleButton)
        toggleButton->removeMouseListener(this);
}

std::vector<std::unique_ptr<juce::TextButton>>& SectionGroupComponent::getButtons()
{
    return buttons;
}
