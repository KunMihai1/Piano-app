/*
  ==============================================================================

    displayGUI.cpp
    Created: 8 Jun 2025 2:44:43am
    Author:  Kisuke

  ==============================================================================
*/

#include "displayGUI.h"

Display::Display(int widthForList)
{
    createUserTracksFolder();
    tabComp = std::make_unique<MyTabbedComponent>(juce::TabbedButtonBar::TabsAtLeft);
    addAndMakeVisible(tabComp.get());

    auto* list = new StylesListComponent{ 10, [this](const juce::String& name) {
        showCurrentStyleTab(name); },widthForList
    };

    auto* scrollableView = new juce::Viewport();
    scrollableView->setScrollBarsShown(true, false); // only vertical
    scrollableView->setViewedComponent(list, true);
    tabComp->addTab("Styles", juce::Colour::fromRGB(10, 15, 10), scrollableView, true);

    list->resized();
    scrollableView->getViewedComponent()->resized();

    loadAllStyles();
    initializeAllStyles();

    tabComp->onTabChanged = [this](int newIndex, juce::String tabName)
    {
        if (createdTracksTab && tabName != "My tracks")
        {
            int myTracksIndex = tabComp->getNumTabs() - 1;
            tabComp->removeTab(myTracksIndex);
            createdTracksTab = false;
        }
    };

    auto appDataFolder = juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
        .getChildFile("Piano Synth2");

    auto jsonFile = appDataFolder.getChildFile("myTracks.json");
    if (!jsonFile.exists())
    {
        juce::var emptyArray = juce::Array<juce::var>{};
        juce::String jsonString = juce::JSON::toString(emptyArray);

        jsonFile.replaceWithText(jsonString);
    }

    trackListComp = std::make_unique<TrackListComponent>(
        availableTracksFromFolder,
        [](int) {}
    );
    trackListComp->loadFromFile(jsonFile);
}

Display::~Display()
{
}

void Display::changeListenerCallback(juce::ChangeBroadcaster* source)
{
    auto* tabbedCast = dynamic_cast<juce::TabbedComponent*>(source);
    if (tabbedCast==tabComp.get())
    {
        int currentIndex = tabComp->getCurrentTabIndex();
        int tracksTabIndex = getTabIndexByName("My tracks");

        if (tracksTabIndex >= 0 && currentIndex != tracksTabIndex)
        {
            tabComp->removeTab(tracksTabIndex);
        }
    }
}

int Display::getTabIndexByName(const juce::String& name)
{
    auto& tabBar = tabComp->getTabbedButtonBar();

    for (int i = 0; i < tabComp->getNumTabs(); ++i)
    {
        if (tabBar.getTabButton(i)->getName() == name)
            return i;
    }
    return -1;
}

void Display::showCurrentStyleTab(const juce::String& name)
{
    if (!created)
    {
        currentStyleComponent = std::make_unique<CurrentStyleComponent>(name);
        currentStyleComponent->onRequestTrackSelectionFromTrack = [this](std::function<void(const juce::String&)> trackChosenCallback)
        {
            showListOfTracksToSelectFrom(trackChosenCallback);
        };
        tabComp->addTab(name, juce::Colour::fromRGB(10, 15, 10), currentStyleComponent.get(), true); //release, get potential issue
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

void Display::showListOfTracksToSelectFrom(std::function<void(const juce::String&)> onTrackSelected)
{
    trackListComp = std::make_unique<TrackListComponent>(availableTracksFromFolder,
        [this, onTrackSelected](int index)
        {
            onTrackSelected(availableTracksFromFolder[index].getDisplayName());
            int in = tabComp->getNumTabs() - 2;
            if (in >= 0)
            {
                updateStyleInJson(currentStyleComponent->getName());
                tabComp->setCurrentTabIndex(in);
            }
        });

     tabComp->addTab("My tracks", juce::Colour::fromRGB(10, 15, 10), trackListComp.release(), true);
     tabComp->setCurrentTabIndex(tabComp->getNumTabs() - 1);

     createdTracksTab = true;
}

void Display::createUserTracksFolder()
{
    auto userTracksFolder = juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
        .getChildFile("Piano Synth2")
        .getChildFile("UserTracks");

    if (!userTracksFolder.exists())
    {
        userTracksFolder.createDirectory();
    }
}

std::vector<TrackEntry> Display::getAvailableTracksFromFolder(const juce::File& folder)
{
    std::vector<TrackEntry> tracks;

    if (!folder.exists() || !folder.isDirectory())
        return tracks;

    juce::DirectoryIterator iter(folder, false, "*.mid", juce::File::findFiles);

    while (iter.next())
    {
        juce::File trackFile = iter.getFile();
        TrackEntry tr;
        tr.file = trackFile;
        tracks.push_back(tr);
    }

    return tracks;
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
                styleVar = juce::var(currentStyleComponent->getJson());
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
    label.setFont(juce::Font(18.0f, juce::Font::bold));
    label.setText(styleName, juce::dontSendNotification);
    label.setColour(juce::Label::textColourId, juce::Colours::cyan);
    label.setInterceptsMouseClicks(true, false);
    label.setMouseCursor(juce::MouseCursor::PointingHandCursor);

    addAndMakeVisible(label);

    label.addMouseListener(this, false);

}

void StyleViewComponent::resized()
{
    auto font = label.getFont();
    int textWidth = static_cast<int>(font.getStringWidth(label.getText()));
    int textHeight = static_cast<int>(font.getHeight());

    label.setBounds(10, 10, textWidth + 10, textHeight + 6);
}

void StyleViewComponent::mouseUp(const juce::MouseEvent& event)
{
    if (event.eventComponent == &label && onStyleClicked)
    {
        onStyleClicked(label.getText());
    }
}

StylesListComponent::StylesListComponent(int nrOfStyles, std::function<void(const juce::String&)> onStyleClicked, int widthSize): nrOfStyles{nrOfStyles}, onStyleClicked{onStyleClicked}
{
    this->widthSize = widthSize;
    populate();
}

void StylesListComponent::resized()
{
    this->widthSize = getWidth();
    const int itemWidth = getWidth()/2-10;   // Half of parent initial parent width - with spacing
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

void StylesListComponent::setWidthSize(const int newWidth)
{
    this->widthSize = newWidth;
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
    setSize(widthSize, totalHeight/2);  // key fix: force vertical scrolling
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

        newTrack->onRequestTrackSelection = [this](std::function<void(const juce::String&)> trackChosenCallback)
        {
            if (onRequestTrackSelectionFromTrack)
                onRequestTrackSelectionFromTrack(trackChosenCallback);
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

juce::String CurrentStyleComponent::getName()
{
    return name;
}

juce::DynamicObject* CurrentStyleComponent::getJson() const
{
    auto* styleObj = new juce::DynamicObject{};

    styleObj->setProperty("name", name);

    juce::Array<juce::var> tracksArray;

    for (auto* track : allTracks)
    {
        tracksArray.add(juce::var(track->getJson()));
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
        //int instrumentNumber = static_cast<int>(trackObj->getProperty("instrumentNumber")); //TO DO when choosing instruments

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

    nameLabel.setInterceptsMouseClicks(true, false);
    nameLabel.addMouseListener(this, false);
    nameLabel.setMouseCursor(juce::MouseCursor::PointingHandCursor);

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
    nameLabel.setBounds((getWidth() - 40) / 2, volumeLabel.getHeight()+volumeLabel.getY() + 20, getWidth(), 20);
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
    //obj->setProperty("instrumentNumber")  //TO DO when instrument selecting is possible

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

void Track::mouseDown(const juce::MouseEvent& event)
{
    if (event.eventComponent == &nameLabel)
    {
        if (onRequestTrackSelection)
        {
            onRequestTrackSelection([this](const juce::String& selectedTrack)
                {
                    setNameLabel(selectedTrack);
                }
            );
        }
    }
}

TrackListComponent::TrackListComponent(std::vector<TrackEntry>& tracks, std::function<void(int)> onTrackChosen): availableTracks{tracks}, trackChosenCallBack{onTrackChosen}
{
    addAndMakeVisible(listBox);
    addAndMakeVisible(addButton);
    addAndMakeVisible(removeButton);
    addAndMakeVisible(sortComboBox);
    listBox.setModel(this);
    listBox.updateContent();
    sortComboBox.addListener(this);

    addButton.onClick = [this] {

        addToTrackList();
    };

    removeButton.onClick = [this]
    {
        removeFromTrackList();
    };

    sortComboBox.setMouseCursor(juce::MouseCursor::PointingHandCursor);
    addButton.setMouseCursor(juce::MouseCursor::PointingHandCursor);

    removeButton.setMouseCursor(juce::MouseCursor::PointingHandCursor);

    listBox.setMultipleSelectionEnabled(true);
    sortComboBox.addItem("Sort by Name(ascending)", 1);
    sortComboBox.addItem("Sort by Last Modified (Newest)", 2);
    sortComboBox.addItem("Sort by Last Modified (Oldest)", 3);

    sortComboBox.setSelectedId(1);
}

void TrackListComponent::resized()
{
    auto area = getLocalBounds();

    int controlBarHeight = 36;
    auto controlBarArea = area.removeFromTop(controlBarHeight);
    sortComboBox.setBounds(controlBarArea.removeFromLeft(150).reduced(5));

    addButton.setBounds(controlBarArea.removeFromLeft(80).reduced(5));

    removeButton.setBounds(controlBarArea.removeFromLeft(80).reduced(5));

    listBox.setBounds(area);
}

int TrackListComponent::getNumRows()
{
    return (int)availableTracks.size();
}

void TrackListComponent::comboBoxChanged(juce::ComboBox* comboBoxThatHasChanged)
{
    if (comboBoxThatHasChanged == &sortComboBox)
    {
        int id = sortComboBox.getSelectedId();
        if (id == 1)
        {
            std::sort(availableTracks.begin(), availableTracks.end(), [](const TrackEntry& first, const TrackEntry& second)
                {
                    return first.getDisplayName().toLowerCase() < second.getDisplayName().toLowerCase();
                });
        }
        else if (id == 2)
        {
            std::sort(availableTracks.begin(), availableTracks.end(), [](const TrackEntry& first, const TrackEntry& second)
                {
                    return first.file.getLastModificationTime() > second.file.getLastModificationTime();
                });
        }
        else if (id == 3)
        {
            std::sort(availableTracks.begin(), availableTracks.end(), [](const TrackEntry& first, const TrackEntry& second)
                {
                    return first.file.getLastModificationTime() < second.file.getLastModificationTime();
                });
        }

        listBox.updateContent();
        listBox.repaint();
    }
}

void TrackListComponent::paintListBoxItem(int rowNumber, juce::Graphics& g, int width, int height, bool rowIsSelected)
{
    if (rowIsSelected)
        g.fillAll(juce::Colours::lightblue);
    else
        g.fillAll(juce::Colours::white);
    g.setColour(juce::Colours::black);
    g.drawText(availableTracks[rowNumber].getDisplayName(), 5, 0, width - 10, height, juce::Justification::centredLeft);

    g.setColour(juce::Colours::lightgrey);
    g.drawRect(0, 0, width, height, 1);
}

void TrackListComponent::listBoxItemClicked(int row, const juce::MouseEvent& event)
{
    if (event.getNumberOfClicks() == 2)
    {
        if (trackChosenCallBack)
            trackChosenCallBack(row);
    }
    else
    {

    }
}

void TrackListComponent::addToTrackList()
{
    auto chooser = std::make_shared<juce::FileChooser>("Select a MIDI file to add", juce::File{}, "*.mid");

    chooser->launchAsync(
        juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
        [this, chooser](const juce::FileChooser& fc)
        {
            auto file = fc.getResult();
            if (file.existsAsFile())
            {
                juce::FileInputStream inputStream(file);
                if (!inputStream.openedOk())
                {
                    juce::AlertWindow::showMessageBoxAsync(
                        juce::AlertWindow::WarningIcon,
                        "Failed parsing",
                        "Could not open the selected file");
                    return;
                }

                juce::MidiFile midiFile;
                if (!midiFile.readFrom(inputStream))
                {
                    juce::AlertWindow::showMessageBoxAsync(
                        juce::AlertWindow::WarningIcon,
                        "Failed parsing",
                        "Could not read the selected file");
                    return;
                }

                midiFile.convertTimestampTicksToSeconds();

                int totalTracks = midiFile.getNumTracks();
                int addedTracks = 0;
                int duplicateTracks = 0;

                for (int i = 0; i < totalTracks; ++i)
                {
                    auto* newTrackSeq = midiFile.getTrack(i);
                    bool duplicate = false;
                    for (const auto& trackEntry : availableTracks)
                    {
                        if (trackEntry.file == file && trackEntry.trackIndex == i)
                        {
                            duplicate = true;
                            break;
                        }
                    }
                    if (duplicate)
                    {
                        ++duplicateTracks;
                        continue;
                    }

                    auto* trackSequence = midiFile.getTrack(i);
                    juce::String displayName = extractDisplayNameFromTrack(*trackSequence);

                    TrackEntry newEntry;
                    newEntry.file = file;
                    newEntry.trackIndex = i;
                    newEntry.displayName = displayName;

                    availableTracks.push_back(newEntry);
                    ++addedTracks;
                }

                listBox.updateContent();
                listBox.repaint();
                auto appDataFolder = juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
                    .getChildFile("Piano Synth2");
                auto jsonFile = appDataFolder.getChildFile("myTracks.json");
                saveToFile(jsonFile);

                juce::AlertWindow::showMessageBoxAsync(
                    juce::AlertWindow::InfoIcon,
                    "Add Tracks",
                    "Selected MIDI file has " + juce::String(totalTracks) + " tracks.\n"
                    + juce::String(addedTracks) + " tracks added.\n"
                    + juce::String(duplicateTracks) + " tracks were already in the list."
                );
            }
        }
    );
}

void TrackListComponent::removeFromTrackList()
{
    juce::SparseSet<int> selectedRows = listBox.getSelectedRows();
    for (int i = selectedRows.size()-1; i >= 0; i--)
    {
        int rowIndex = selectedRows[i];
        availableTracks.erase(availableTracks.begin() + rowIndex);
    }

    listBox.deselectAllRows();
    listBox.updateContent();
    listBox.repaint();
    auto appDataFolder = juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
        .getChildFile("Piano Synth2");
    auto jsonFile = appDataFolder.getChildFile("myTracks.json");
    saveToFile(jsonFile);
}

void TrackListComponent::saveToFile(const juce::File& fileToSave)
{
    juce::Array<juce::var> filesArray;
    std::map<juce::File, std::vector<TrackEntry>> grouped;


    for (auto& track : availableTracks)
    {
        grouped[track.file].push_back(track);
    }

    for (auto& [file, tracks] : grouped)
    {
        auto* fileObj = new juce::DynamicObject{};
        fileObj->setProperty("filePath", file.getFullPathName());

        juce::Array<juce::var> trackArray;

        for (auto& tr : tracks)
        {
            auto* trackObject = new juce::DynamicObject{};
            trackObject->setProperty("trackIndex", tr.trackIndex);
            trackObject->setProperty("displayName", tr.displayName);

            trackArray.add(juce::var(trackObject));
        }
        fileObj->setProperty("Tracks", trackArray);
        filesArray.add(juce::var(fileObj));

    }

    juce::var jsonVar(filesArray);
    juce::String jsonString = juce::JSON::toString(jsonVar);
    fileToSave.replaceWithText(jsonString);
}

void TrackListComponent::loadFromFile(const juce::File& fileToLoad)
{
    if (!fileToLoad.existsAsFile())
        return;

    juce::String jsonString = fileToLoad.loadFileAsString();

    juce::var jsonVar = juce::JSON::parse(jsonString);

    if (!jsonVar.isArray())
        return;

    juce::Array<juce::var>* jsonArray = jsonVar.getArray();
    availableTracks.clear();

    for (auto& item : *jsonArray)
    {
        auto* fileObj = item.getDynamicObject();
        if (fileObj == nullptr)
            continue;

        juce::String filePath = fileObj->getProperty("filePath").toString();
        juce::File file{ filePath };

        if (!file.existsAsFile())
            continue;

        juce::var tracksVar = fileObj->getProperty("Tracks");
        if (!tracksVar.isArray())
            continue;

        juce::Array<juce::var>* trackArray = tracksVar.getArray();
        
        for (auto& trackItem : *trackArray)
        {
            auto* trackObj = trackItem.getDynamicObject();
            if (trackObj == nullptr)
                continue;
            int trackIndex = (int)trackObj->getProperty("trackIndex");
            juce::String displayName = trackObj->getProperty("displayName").toString();


            TrackEntry tr;
            tr.file = file;
            tr.trackIndex = trackIndex;
            tr.displayName = displayName;
            availableTracks.push_back(tr);
        }
    }

    listBox.updateContent();
    listBox.repaint();
}

std::vector<TrackEntry>& TrackListComponent::getAllAvailableTracks() const
{
    return this->availableTracks;
}

juce::String TrackListComponent::extractDisplayNameFromTrack(const juce::MidiMessageSequence& trackSeq)
{
    juce::String trackName;
    juce::String instrumentName;

    for (int i = 0; i < trackSeq.getNumEvents(); i++)
    {
        const auto& event = trackSeq.getEventPointer(i)->message;

        if (event.isTrackNameEvent())
        {
            trackName = event.getTextFromTextMetaEvent();
        }
        else if (event.isMetaEvent() && event.getMetaEventType() == 0x04)
        {
            instrumentName = event.getTextFromTextMetaEvent();
        }
    }

    if (trackName.isNotEmpty() && instrumentName.isNotEmpty())
        return trackName + " (" + instrumentName + ")";
    else if (trackName.isNotEmpty())
        return trackName;
    else if (instrumentName.isNotEmpty())
        return instrumentName;

    return "Unnamed Track";
}


void MyTabbedComponent::currentTabChanged(int newCurrentTabIndex, const juce::String& newTabName)
{
    if (onTabChanged)
        onTabChanged(newCurrentTabIndex, newTabName);
}
