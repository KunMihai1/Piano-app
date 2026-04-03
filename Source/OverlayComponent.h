#pragma once
#include <JuceHeader.h>
#include <functional>

class OverlayComponent : public juce::Component
{
public:
    std::function<void()> onSettingsClick;
    std::function<bool()> windowExists;
    std::function<void()> closeWindow;
    std::function<void()> bringSeparateWindowFront;
    std::function<void()> setWindowFlag;

    OverlayComponent();
    ~OverlayComponent() override = default;

    void paint(juce::Graphics& g) override;
    void resized() override;

    void showOverlay();
    void hideOverlay();

    juce::TextButton& getExitButton();

    juce::TextButton& getSettingsButton();

    void mouseDown(const juce::MouseEvent& ev) override;

    bool keyPressed(const juce::KeyPress& key) override;


    // main component should assign this so overlay can request to be closed
    std::function<void()> onRequestClose;

private:
    juce::Component menuPanel;
    juce::TextButton settingsButton;
    juce::TextButton exitButton;

    // handle keyboard when overlay has focus
    

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(OverlayComponent)
};

/*
* if (!midiWindow)
    {
        midiWindow = std::make_unique<MIDIWindow>(this->MIDIDevice, devicesIN, devicesOUT, propertiesFile);
        loadSettings();
    }
    else {
        midiWindow->setVisible(true);
        midiWindow->toFront(true);
    }
*/