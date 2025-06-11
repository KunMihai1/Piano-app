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
    std::function<void()> onChange;

    Track();

    void resized() override;
    
    void paint(juce::Graphics& g) override;

    juce::DynamicObject* getJson() const;

    juce::var loadJson(const juce::File& file);

    void setVolumeSlider(double value);
    void setVolumeLabel(const juce::String& value);
    void setNameLabel(const juce::String& name);


private:
    juce::Slider volumeSlider;
    juce::Label volumeLabel;
    juce::Label nameLabel;
};

class CurrentStyleComponent : public juce::Component
{
public:
    std::function<void()> anyTrackChanged;

    CurrentStyleComponent(const juce::String& name);

    void resized() override;

    void updateName(const juce::String& newName);

    juce::DynamicObject* getJson() const;

    void loadJson(const juce::var& styleVar);

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

    void initializeAllStyles();
    void loadAllStyles();
    void updateStyleInJson(const juce::String& name);

    void resized() override;
    const juce::var& getJsonVar();

    void showCurrentStyleTab(const juce::String& name);

private:
    juce::HashMap<juce::String, std::unique_ptr<juce::DynamicObject>> styleDataCache;
    std::unique_ptr<juce::TabbedComponent> tabComp;
    std::unique_ptr<CurrentStyleComponent> currentStyleComponent;
    bool created = false;

    juce::var allStylesJsonVar;
};
