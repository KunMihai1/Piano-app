/*
  ==============================================================================

    addDeviceWindow.cpp
    Created: 29 Oct 2025 11:46:49pm
    Author:  Kisuke

  ==============================================================================
*/

#include "addDeviceWindow.h"

AddDeviceWindow::AddDeviceWindow(const juce::String& vid, const juce::String& pid)
    : DialogWindow("Add MIDI Device", juce::Colours::darkgrey, true),
    vidString{ vid }, pidString{ pid }
{
    setContentOwned(new juce::Component(), true); // create content component
    auto* content = getContentComponent();

    nameEditor = std::make_unique<juce::TextEditor>();
    nameEditor->setText("Device name");
    content->addAndMakeVisible(nameEditor.get());
    nameEditor->setBounds(20, 20, 200, 24);

    keysEditor = std::make_unique<juce::TextEditor>();
    keysEditor->setText("# of keys");
    content->addAndMakeVisible(keysEditor.get());
    keysEditor->setBounds(20, 60, 200, 24);

    addButton = std::make_unique<juce::TextButton>("Add");
    content->addAndMakeVisible(addButton.get());
    addButton->setBounds(20, 100, 100, 24);

    addButton->onClick = [this, vid, pid]()
    {
        juce::String name = nameEditor->getText();
        int keys = keysEditor->getText().getIntValue();
        if (!name.isEmpty() && keys > 0)
        {
            if (onAddDevice)
                onAddDevice(name, keys);
            juce::AlertWindow::showMessageBoxAsync(
                juce::AlertWindow::InfoIcon,
                "Successfull adding",
                "Your device named: " + name + " with: " + juce::String(keys) + " keys has been added sucesffuly!"
            );

            closeButtonPressed();
        }
        else
        {
            juce::AlertWindow::showMessageBoxAsync(
                juce::AlertWindow::WarningIcon,
                "Invalid Input",
                "Please enter a valid device name and number of keys."
            );
        }
    };

    setSize(300, 200);

}

void AddDeviceWindow::closeButtonPressed()
{
    setVisible(false);
}
