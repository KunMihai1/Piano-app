#include "OverlayComponent.h"

OverlayComponent::OverlayComponent()
{
    setVisible(false);
    setOpaque(false);

    setWantsKeyboardFocus(true);

    addAndMakeVisible(menuPanel);
    menuPanel.setInterceptsMouseClicks(true, true);

    menuPanel.addAndMakeVisible(settingsButton);
    menuPanel.addAndMakeVisible(exitButton);

    settingsButton.setButtonText("Settings");
    settingsButton.setMouseCursor(juce::MouseCursor::PointingHandCursor);
    settingsButton.setColour(juce::TextButton::textColourOffId, juce::Colours::white);
    settingsButton.onClick = [this]() { 
        if (onSettingsClick)
            onSettingsClick();
    };

    exitButton.setButtonText("Exit");
    exitButton.setMouseCursor(juce::MouseCursor::PointingHandCursor);
    exitButton.setColour(juce::TextButton::textColourOffId, juce::Colours::white);
    exitButton.onClick = [this]()
    {

        juce::JUCEApplication::getInstance()->systemRequestedQuit();
    };
}

void OverlayComponent::paint(juce::Graphics& g)
{

    g.fillAll(juce::Colours::black.withAlpha(0.6f));
}

void OverlayComponent::resized()
{
    auto area = getLocalBounds();
    const int panelWidth = 300;
    const int panelHeight = 200;
    menuPanel.setBounds(area.getCentreX() - panelWidth / 2,
        area.getCentreY() - panelHeight / 2,
        panelWidth, panelHeight);

    auto panelArea = menuPanel.getLocalBounds().reduced(30);
    const int buttonHeight = 40;
    const int spacing = 20;

    settingsButton.setBounds(panelArea.removeFromTop(buttonHeight)
        .withSizeKeepingCentre(panelArea.getWidth(), buttonHeight));
    panelArea.removeFromTop(spacing);
    exitButton.setBounds(panelArea.removeFromTop(buttonHeight)
        .withSizeKeepingCentre(panelArea.getWidth(), buttonHeight));
}

void OverlayComponent::showOverlay()
{
    setVisible(true);
    toFront(true);


    grabKeyboardFocus();

    resized();
    repaint();
}

void OverlayComponent::hideOverlay()
{
    setVisible(false);

}

void OverlayComponent::mouseDown(const juce::MouseEvent& ev)
{
    if (bringSeparateWindowFront)
        bringSeparateWindowFront();
}


bool OverlayComponent::keyPressed(const juce::KeyPress& key)
{
    if (key == juce::KeyPress::escapeKey)
    {
       
        if (onRequestClose)
        {
            onRequestClose();
        }
        else
        {
            if (setWindowFlag)
                setWindowFlag();
            removeFromDesktop();
            setVisible(false);
        }
        return true;
    }
    return false;
}
