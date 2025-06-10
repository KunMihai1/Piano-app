/*
  ==============================================================================

    displayGUI.h
    Created: 8 Jun 2025 2:44:43am
    Author:  Kisuke

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>

class Track : public juce::Component
{
public:
    Track();

    void resized() override;
    

private:
    juce::Slider volumeSlider;
    juce::Label volumeLabel;
    juce::Label instrumentLabel;
};

class CurrentStyleComponent : public juce::Component
{
public:
    CurrentStyleComponent(const juce::String& name);

    void resized() override;

    void updateName(const juce::String& newName);

    void initializeTracks();

private:
    juce::String name;
    juce::Label nameOfStyle;
    juce::Label selectedTrackLabel, selectedTrackKey, selectedTrackChord;
    juce::OwnedArray<Track> allTracks;
};

class StyleViewComponent : public juce::Component, public juce::MouseListener
{
public:
    StyleViewComponent(const juce::String& styleName);

    void resized() override;

    void mouseUp(const juce::MouseEvent& event) override;

    std::function<void(const juce::String&)> onStyleClicked;

private:
    juce::Label label;
    juce::OwnedArray<juce::TextButton> trackButtons;
};

class StylesListComponent : public juce::Component
{
public:
    StylesListComponent(int nrOfStyles, std::function<void(const juce::String&)> onStyleClicked);

    void resized() override;

private:
    void populate();

    juce::OwnedArray<StyleViewComponent> allStyles;
    std::function<void(const juce::String&)> onStyleClicked;

    int nrOfStyles;
};

class Display: public  juce::Component
{
public:
    Display();
    ~Display() override;

    void paint(juce::Graphics& g) override;

    void resized() override;

    void showCurrentStyleTab(const juce::String& name);

private:
    std::unique_ptr<juce::TabbedComponent> tabComp;
    std::unique_ptr<CurrentStyleComponent> currentStyleComponent;
    bool created = false;
};
