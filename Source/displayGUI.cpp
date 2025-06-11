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

    loadAllStyles();
    initializeAllStyles();
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
    currentStyleComponent->anyTrackChanged = [this,name]()
    {
        updateStyleInJson(name);
    };
        
    if (allStylesJsonVar.isObject())
    {
        auto stylesArray = allStylesJsonVar["styles"];
        if (stylesArray.isArray())
        {
            for (const auto& style : *stylesArray.getArray())
            {
                if (auto* obj = style.getDynamicObject())
                {
                    if (obj->getProperty("name").toString() == name)
                    {
                        currentStyleComponent->loadJson(style);
                        break;
                    }
                }
            }
        }
    }

    tabComp->setCurrentTabIndex(tabComp->getNumTabs() - 1);
}

void Display::initializeAllStyles()
{
    juce::File appDataFolder = juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
        .getChildFile("Piano Synth2");

    if (!appDataFolder.exists())
        appDataFolder.createDirectory();

    auto file = appDataFolder.getChildFile("allStyles.json");

    if (file.exists())
        return;

    juce::Array<juce::var> stylesArray;

    for (int i = 0; i < 10; ++i)
    {
        auto* styleObj = new juce::DynamicObject{};
        juce::String styleName = "Style " + juce::String(i + 1);
        styleObj->setProperty("name", styleName);

        juce::Array<juce::var> tracksArray;
        for (int t = 0; t < 8; ++t)
        {
            auto* trackObj = new juce::DynamicObject{};
            trackObj->setProperty("name", "None");
            trackObj->setProperty("volume", 0.0f);
            tracksArray.add(trackObj);
        }
        styleObj->setProperty("tracks", tracksArray);
        stylesArray.add(styleObj);
    }

    auto* rootObj = new juce::DynamicObject{};
    rootObj->setProperty("styles", stylesArray);

    juce::var jsonVar(rootObj);  // var now owns rootObj pointer

    juce::String jsonString = juce::JSON::toString(jsonVar); 

    file.replaceWithText(jsonString);
}

void Display::loadAllStyles()
{
    juce::File appDataFolder = juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
        .getChildFile("Piano Synth2");

    auto file = appDataFolder.getChildFile("allStyles.json");

    if (!file.exists())
    {
        allStylesJsonVar = juce::var();
        return;
    }

    juce::String jsonString = file.loadFileAsString();
    allStylesJsonVar = juce::JSON::parse(jsonString);
}

void Display::updateStyleInJson(const juce::String& name)
{
    if (!allStylesJsonVar.isObject())
        return;

    auto stylesArray = allStylesJsonVar["styles"];

    if (!stylesArray.isArray())
        return;

    for (int i = 0; i < stylesArray.getArray()->size(); ++i)
    {
        auto& styleVar = stylesArray.getArray()->getReference(i);
        if (auto* obj = styleVar.getDynamicObject())
        {
            if (obj->getProperty("name").toString() == name)
            {
                styleVar = currentStyleComponent->getJson();
                break;
            }
        }
    }

    juce::File appDataFolder = juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
        .getChildFile("Piano Synth2");
    auto file = appDataFolder.getChildFile("allStyles.json");

    juce::String jsonString = juce::JSON::toString(allStylesJsonVar);
    file.replaceWithText(jsonString);
}

void Display::resized()
{
    tabComp->setBounds(getLocalBounds());
}

const juce::var& Display::getJsonVar()
{
    return allStylesJsonVar;
}

StyleViewComponent::StyleViewComponent(const juce::String& styleName)
{
    label.setText(styleName, juce::dontSendNotification);
    label.setColour(juce::Label::textColourId, juce::Colours::cyan);
    label.setInterceptsMouseClicks(true, false);
    label.setMouseCursor(juce::MouseCursor::PointingHandCursor);

    addAndMakeVisible(label);

    label.addMouseListener(this, false);

}

void StyleViewComponent::resized()
{
    label.setBounds(10, 10, getWidth() - 30, 30);

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
        newTrack->onChange = [this]()
        {
            if (anyTrackChanged)
                anyTrackChanged();
        };
        addAndMakeVisible(newTrack);
        allTracks.add(newTrack);
    }
}

void CurrentStyleComponent::resized()
{
    int labelWidth = 50;
    nameOfStyle.setBounds((getWidth()-labelWidth) / 2, 0, getWidth() / 6, 20);

    float width = getWidth() / 8.0f, height = 80.0f;
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

juce::DynamicObject* CurrentStyleComponent::getJson() const
{
    auto* styleObj = new juce::DynamicObject{};

    styleObj->setProperty("name", name);

    juce::Array<juce::var> tracksArray;

    for (auto* track : allTracks)
    {
        tracksArray.add(track->getJson());
    }

    styleObj->setProperty("tracks", tracksArray);

    return styleObj;
}

void CurrentStyleComponent::loadJson(const juce::var& styleVar)
{
    if (!styleVar.isObject())
        return;


    auto* obj = styleVar.getDynamicObject();
    if (!obj)
        return;

    updateName(obj->getProperty("name").toString());

    auto tracksVar = obj->getProperty("tracks");

    if (!tracksVar.isArray())
        return;

    auto* tracksArray = tracksVar.getArray();

    for (int i = 0; i < allTracks.size() && i < tracksArray->size(); ++i)
    {
        auto trackVar = tracksArray->getReference(i);
        if (!trackVar.isObject())
            continue;

        auto* trackObj = trackVar.getDynamicObject();
        if (!trackObj)
            continue;

        auto name = trackObj->getProperty("name").toString();
        double volume = static_cast<double>(trackObj->getProperty("volume"));

        allTracks[i]->setNameLabel(name);
        allTracks[i]->setVolumeSlider(volume);
        allTracks[i]->setVolumeLabel(juce::String(volume));
    }
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

    volumeSlider.onDragEnd = [this]()
    {
        if (onChange)
            onChange();
    };

    addAndMakeVisible(volumeSlider);


    volumeLabel.setText(juce::String(volumeSlider.getValue()), juce::dontSendNotification);
    volumeLabel.setColour(juce::Label::textColourId,juce::Colours::crimson);
    addAndMakeVisible(volumeLabel);

    nameLabel.setText("None", juce::dontSendNotification);
    nameLabel.setColour(juce::Label::textColourId,juce::Colours::white);
    addAndMakeVisible(nameLabel);
}

Track::~Track()
{
    volumeSlider.onDragEnd = nullptr;
    volumeSlider.onValueChange = nullptr; //in case of alt f4 it will crash otherwise since these listeners will propagate the onChange
}

void Track::resized()
{
    auto heightOfSlider = getHeight() / 2 + 20;
    auto startY = (getHeight() - heightOfSlider)/2;
    volumeSlider.setBounds(0, startY-10, 15, heightOfSlider);
    volumeLabel.setBounds(7, 0, 30, 40);
    nameLabel.setBounds((getWidth() - 30) / 2, volumeLabel.getHeight()+volumeLabel.getY() + 20, getWidth(), 20);
}

void Track::paint(juce::Graphics& g)
{
    g.setColour(juce::Colours::lightgrey);
    g.drawRect(getLocalBounds(), 1);
}

juce::DynamicObject* Track::getJson() const
{

    auto* obj = new juce::DynamicObject{};

    obj->setProperty("name", nameLabel.getText());
    obj->setProperty("volume", volumeSlider.getValue());

    return obj;
}

juce::var Track::loadJson(const juce::File& file)
{
    if (file.exists())
        return juce::var{};

    juce::String jsonString = file.loadFileAsString();
    juce::var jsonVar = juce::JSON::parse(jsonString);

    if (jsonVar.isUndefined())
        return juce::var{};

    return jsonVar;



}

void Track::setVolumeSlider(double value)
{
    this->volumeSlider.setValue(value);
}

void Track::setVolumeLabel(const juce::String& value)
{
    volumeLabel.setText(value, juce::dontSendNotification);
}

void Track::setNameLabel(const juce::String& name)
{
    nameLabel.setText(name, juce::dontSendNotification);
}
