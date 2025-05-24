#pragma once

#include <JuceHeader.h>
#include "settingsWindow.h"
#include "BinaryData.h"
#include "MidiHandler.h"
#include "KeyboardUI.h"
#include "NoteLayer.h"
#include "audioProcessor.h"
#include "keyListener.h"
#include "InstrumentTreeItem.h"

class CustomLookAndFeel : public juce::LookAndFeel_V4
{
public:
    juce::Font getTextButtonFont(juce::TextButton& button, int buttonHeight) override
    {
        juce::ignoreUnused(button, buttonHeight);
        return juce::Font(30.0f, juce::Font::bold);
    }

    void drawButtonBackground(juce::Graphics& g,
        juce::Button& button,
        const juce::Colour& backgroundColour,
        bool isMouseOverButton,
        bool isButtonDown) override 
    {
        juce::ignoreUnused(button, isMouseOverButton, isButtonDown);
        g.fillAll(backgroundColour);
    }
};

//==============================================================================
/*
    This component lives inside our window, and this is where you should put all
    your controls and content.
*/
class MainComponent : public juce::Component, public juce::ChangeListener, public juce::KeyListener
{
public:
    //==============================================================================
    MainComponent();
    ~MainComponent() override;

    //==============================================================================
    void paint(juce::Graphics&) override;
    void resized() override;
    bool isMouseDownInsideLabel = false;
    void changeListenerCallback(juce::ChangeBroadcaster* source) override;
    bool keyPressed(const juce::KeyPress& key, juce::Component*) override;

private:
    int initialWidth = 0;
    int initialHeight = 0;

    int xPlay = 750, yPlay = 690;

    bool hasBeenResized = false;
    void initalizeSaveFileForUser();
    void loadSettings();


    void focusGained(FocusChangeType) override;
    void toggleSettingsPanel();
    void toggleSettingsButton();
    void toggleMIDIButton();
    void togglePlayButton();
    void toggleMIDIsettingsIcon();
    void toggleHPanel();
    void toggleHomeButton();
    void toggleForPlaying();
    void toggleColourSelectorButton();
    void toggleInstrumentSelectorButton();

    void settingsInit();
    void playButtonInit();
    void keyBoardUIinit(int min, int max);
    void panelInit();
    void midiIconInit();
    void midiSettingsInit();
    void headerPanelInit();
    void homeButtonInit();
    void colourSelectorButtonInit();
    void instrumentSelectorButtonInit();
    
    void settingsButtonOnClick();
    void midiButtonOnClick();
    void homeButtonOnClick();
    void playButtonOnClick();

    juce::Image getImageForInstruments(const std::string& type);
    void buildTree();
    InstrumentTreeItem* createInstrumentItem(const juce::Image& img, const juce::String& name, int program);
    bool openingDevicesForPlay();
   
    void showColourSelector();
    void showInstrumentSelector();

    //==============================================================================
    // Your private member variables go here...
    juce::ApplicationProperties appProperties;
    juce::PropertiesFile* propertiesFile = nullptr;

    CustomLookAndFeel customLookAndFeel;
    juce::Image cachedImageMainWindow;
    juce::Image playBackground;
    juce::Image currentBackground;
    juce::Label helpIcon;
    juce::TooltipWindow tooltipWindow{ this, 200 };


    const juce::MidiInput* deviceOpenedIN = nullptr;
    const juce::MidiOutput* deviceOpenedOUT = nullptr;
    MidiDevice MIDIDevice{};
    MidiHandler midiHandler{ MIDIDevice };
    KeyboardListener keyListener{ midiHandler };
    //ReverbProcessor revProcessor{ midiHandler };

    std::vector<std::string> devicesIN;
    std::vector<std::string> devicesOUT;

    //UI->refresh button+label of AVAILABLE DEVICES

    //UI->settings
    juce::TextButton settingsButton{ "Settings" };
    juce::TextButton midiButton{ "MIDI Settings" };
    juce::TextButton playButton{ "Play" };
    juce::TextButton homeButton{ "Home" };
    juce::TextButton colourSelectorButton{ "Select colour" };
    juce::TextButton instrumentSelectorButton{ "Select instrument" };
    juce::ColourSelector* colourSelector;
    std::unique_ptr<juce::TreeView> treeView=nullptr;


    //UI->windows

    std::unique_ptr<MIDIWindow> midiWindow = { nullptr };

    KeyboardUI keyboard{ midiHandler };
    std::unique_ptr<NoteLayer> noteLayer;
    bool keyboardInitialized = false;

    juce::AudioDeviceManager audioDeviceManager;
    juce::AudioProcessorPlayer audioProcessorPlayer;

    bool usingKeyboardInput = false;



private:
    struct Panel : public juce::Component
    {
        void paint(juce::Graphics& g) override
        {
            juce::Colour translucentGray = juce::Colour::fromRGBA(169, 169, 169, 204); // Semi-transparent gray (80% opacity)
            juce::Colour option = juce::Colour::fromRGBA(50, 100, 95, 170);

            juce::Colour subtleBorderColor = juce::Colour::fromRGBA(169, 169, 169, 200);
            juce::Colour secondBorder = juce::Colour::fromRGBA(140, 140, 140, 220);

            g.fillAll(option); // Background of the panel
            g.setColour(subtleBorderColor);
            g.drawRect(getLocalBounds(), 3);

        }
    };

    Panel settingsPanel;

    struct headerPanel : public::juce::Component
    {
        void paint(juce::Graphics& g) override {
            juce::Colour startColour = juce::Colour(128, 0, 32);   // Deep Burgundy
            juce::Colour endColour = juce::Colour(212, 175, 55);  // Metallic Gold
            //juce::Colour startColour = juce::Colour(160, 40, 60); // Softer burgundy
            //juce::Colour endColour = juce::Colour(210, 165, 100); // Lighter gold
            juce::ColourGradient gradient(startColour, 0, 0, endColour, 0, 50, false);
            g.setGradientFill(gradient);
            g.fillAll();
        }
    };

    headerPanel headerPanel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
};

