/*
  ==============================================================================

    displayGUI.cpp
    Created: 8 Jun 2025 2:44:43am
    Author:  Kisuke

  ==============================================================================
*/

#include "displayGUI.h"

Display::Display()
{
    tabComp = std::make_unique<juce::TabbedComponent>(juce::TabbedButtonBar::TabsAtLeft);
    addAndMakeVisible(tabComp.get());
    //tabComp->addTab("Styles", juce::Colours::lightgrey);

    auto* list = new StylesListComponent{ 10, [this](const juce::String& name) {
        showCurrentStyleTab(name); }
    };

    auto* scrollableView = new juce::Viewport();
    scrollableView->setScrollBarsShown(true, false); // only vertical
    scrollableView->setViewedComponent(list, true);
    tabComp->addTab("Styles", juce::Colour::fromRGB(10, 15, 10), scrollableView, true);


    list->resized();
    scrollableView->getViewedComponent()->resized();
}

Display::~Display()
{
}

void Display::showCurrentStyleTab(const juce::String& name)
{
    if (!created)
    {
        currentStyleComponent = std::make_unique<CurrentStyleComponent>(name);
        tabComp->addTab(name, juce::Colour::fromRGB(10, 15, 10), currentStyleComponent.get(), true);
        created = true;
    }
    else
    {
        int index = tabComp->getNumTabs() - 1;
        tabComp->getTabbedButtonBar().setTabName(index, name);
        currentStyleComponent->updateName(name);
    }
    tabComp->setCurrentTabIndex(tabComp->getNumTabs() - 1);
}

void Display::paint(juce::Graphics& g)
{
}

void Display::resized()
{
    tabComp->setBounds(getLocalBounds());
}

StyleViewComponent::StyleViewComponent(const juce::String& styleName)
{
    label.setText(styleName, juce::dontSendNotification);
    label.setColour(juce::Label::textColourId, juce::Colours::cyan);
    label.setInterceptsMouseClicks(true, false);
    label.setMouseCursor(juce::MouseCursor::PointingHandCursor);

    addAndMakeVisible(label);

    label.addMouseListener(this, false);
    
    /*
    for (int i = 0; i < 8; i++)
    {
        auto* trackButton = new juce::TextButton("Track " + juce::String(i + 1));
        addAndMakeVisible(trackButtons.add(trackButton));
        trackButton->setVisible(false);
    }
    */
}

void StyleViewComponent::resized()
{
    label.setBounds(10, 10, getWidth() - 30, 30);

    auto area = getLocalBounds().reduced(10).withTop(50);
    for (auto* button : trackButtons)
    {
        button->setBounds(area.removeFromTop(30).reduced(0, 5));
    }
}

void StyleViewComponent::mouseUp(const juce::MouseEvent& event)
{
    if (event.eventComponent == &label && onStyleClicked)
    {
        onStyleClicked(label.getText());
    }
}

StylesListComponent::StylesListComponent(int nrOfStyles, std::function<void(const juce::String&)> onStyleClicked): nrOfStyles{nrOfStyles}, onStyleClicked{onStyleClicked}
{
    populate();
}

void StylesListComponent::resized()
{
    //DBG("DADADADA" + juce::String(getWidth() / 2 - 10) +" " + juce::String(getHeight() / 2));
    const int itemWidth = getWidth()/2-10;   // Half of 300 width - with spacing
    const int itemHeight =50;
    const int spacing = 10;
    const int columns = 2;

    int x = spacing;
    int y = spacing;
    int count = 0;

    for (auto* style : allStyles)
    {
        style->setBounds(x, y, itemWidth, itemHeight);

        count++;
        if (count % columns == 0)
        {
            x = spacing;
            y += itemHeight + spacing;
        }
        else
        {
            x += itemWidth + spacing;
        }
    }
}

void StylesListComponent::populate()
{
    const int itemHeight = 50;
    const int spacing = 10;

    for (int i = 0; i < nrOfStyles; i++)
    {
        auto* newStyle = new StyleViewComponent{ "Style " + juce::String(i + 1) };
        newStyle->onStyleClicked = this->onStyleClicked;
        allStyles.add(newStyle);
        addAndMakeVisible(newStyle);
    }

    // Very narrow width; Viewport will stretch it later
    const int totalHeight = nrOfStyles * (itemHeight + spacing) + spacing;
    setSize(300, totalHeight/2);  // key fix: force vertical scrolling

    DBG("Populated with " + juce::String(allStyles.size()) + " styles.");
}


CurrentStyleComponent::CurrentStyleComponent(const juce::String& name) : name{ name }
{
    nameOfStyle.setText(name, juce::dontSendNotification);
    addAndMakeVisible(nameOfStyle);


    for (int i = 0; i < 8; i++)
    {
        auto* newTrack = new Track{};
        addAndMakeVisible(newTrack);
        allTracks.add(newTrack);
    }
}

void CurrentStyleComponent::resized()
{
    int labelWidth = 50;
    nameOfStyle.setBounds((getWidth()-labelWidth) / 2, 5, getWidth() / 6, 20);

    float width = getWidth() / 8.0f, height = 50.0f;
    float initialX = 0, y = getHeight() - height;

    for (int i = 0; i < allTracks.size(); i++)
    {
        allTracks[i]->setBounds(juce::Rectangle<int>(
            static_cast<int>(i * width),
            static_cast<int>(y),
            static_cast<int>(width),
            static_cast<int>(height)
            ));
    }
}

void CurrentStyleComponent::updateName(const juce::String& newName)
{
    name = newName;
    nameOfStyle.setText(name, juce::dontSendNotification);
}

void CurrentStyleComponent::initializeTracks()
{
}

Track::Track()
{
    volumeSlider.setSliderStyle(juce::Slider::LinearVertical);
    volumeSlider.setRange(0, 100, 1);
    volumeSlider.onValueChange = [this]()
    {
        volumeLabel.setText(juce::String(volumeSlider.getValue()), juce::dontSendNotification);
    };

    addAndMakeVisible(volumeSlider);


    volumeLabel.setText(juce::String(volumeSlider.getValue()), juce::dontSendNotification);
    volumeLabel.setColour(juce::Label::textColourId,juce::Colours::crimson);
    addAndMakeVisible(volumeLabel);
}

void Track::resized()
{
    auto heightOfSlider = getHeight() / 2 + 20;
    auto startY = (getHeight() - heightOfSlider)/2;
    volumeSlider.setBounds(0, startY, 15, heightOfSlider);
    volumeLabel.setBounds(8, 20, 30, 40);
}
