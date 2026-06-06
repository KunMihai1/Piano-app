#include "displayGUI.h"

Track::Track()
{
    volumeSlider.setSliderStyle(juce::Slider::LinearVertical);
    volumeSlider.setRange(0, 127, 1); //changed from 100 to 127 recently
    volumeSlider.onValueChange = [this]()
    {
        volumeLabel.setText(juce::String(volumeSlider.getValue()), juce::dontSendNotification);
    };

    volumeSlider.onDragEnd = [this]()
    {
        if (getTypeOfTrack() == TrackTypeConversion::toString(TrackType::Percussion) && syncVolumePercussionTracks)
            syncVolumePercussionTracks(volumeSlider.getValue());

        if (onChange)
            onChange();

        if (static_cast<int>(volumeSlider.getValue()) != -1 && isPlaying())
            notify(channel, static_cast<int>(volumeSlider.getValue()), -1);
    };

    addAndMakeVisible(volumeSlider);


    volumeLabel.setText(juce::String(volumeSlider.getValue()), juce::dontSendNotification);
    volumeLabel.setColour(juce::Label::textColourId, juce::Colours::crimson);
    addAndMakeVisible(volumeLabel);

    nameLabel.setText("None", juce::dontSendNotification);
    //nameLabel.setTooltip(nameLabel.getText());
    nameLabel.setColour(juce::Label::textColourId, juce::Colours::white);

    nameLabel.setInterceptsMouseClicks(true, false);
    nameLabel.addMouseListener(this, false);
    nameLabel.setMouseCursor(juce::MouseCursor::PointingHandCursor);

    addAndMakeVisible(nameLabel);

    instrumentChooserButton.setButtonText("Instrument");

    instrumentChooserButton.onClick = [this] {
        if (channel >= 2 && channel <= 16)
        {
            if (channel != 10)
                openInstrumentChooser();
            else
            {
                juce::String toShow = "Since this is a track for percussion, the instrument change won't take effect!\nIf you want to change anything, you must change the original notes!";
                showInstantInfo(toShow);
            }
        }
    };
    instrumentChooserButton.setMouseCursor(juce::MouseCursor::PointingHandCursor);
    addAndMakeVisible(instrumentChooserButton);

    instrumentlist = instrumentListBuild();


}

Track::~Track()
{
    volumeSlider.onDragEnd = nullptr;
    volumeSlider.onValueChange = nullptr; //in case of alt f4 it will crash otherwise since these listeners will propagate the onChange
}

void Track::resized()
{
    auto heightOfSlider = getHeight() / 2 + 20;
    auto startY = (getHeight() - heightOfSlider) / 2;
    volumeSlider.setBounds(0, startY - 10, 15, heightOfSlider);
    volumeLabel.setBounds(20, 0, 30, 40);
    nameLabel.setBounds((getWidth() - 40) / 2, volumeLabel.getHeight() + volumeLabel.getY() + 20, getWidth(), 20);


    int x = volumeSlider.getX() + volumeSlider.getWidth() + 10;
    int maxWidth = getWidth() - x - 5;
    int desiredWidth = 40;
    int actualWidth = juce::jmin(desiredWidth, maxWidth);
    instrumentChooserButton.setBounds(x, nameLabel.getY() - 25, maxWidth - 2, 20);
}

void Track::paint(juce::Graphics& g)
{
    g.setColour(juce::Colours::lightgrey);
    g.drawRect(getLocalBounds(), 1);
}

void Track::mouseEnter(const juce::MouseEvent& ev)
{
    if (ev.eventComponent == &nameLabel && toolTipWindow == nullptr)
    {
        toolTipWindow = std::make_unique<CustomToolTip>(nameLabel.getText());
        toolTipWindow->setSize(150, 24);

        auto labelBounds = nameLabel.getScreenBounds();
        int x = labelBounds.getX() + 4;
        int y = labelBounds.getBottom() + 4;

        toolTipWindow->setTopLeftPosition(x, y);
        toolTipWindow->addToDesktop(0);
        toolTipWindow->toFront(true);
        toolTipWindow->setVisible(true);
    }
}

void Track::mouseExit(const juce::MouseEvent& ev)
{
    if (toolTipWindow != nullptr)
    {
        toolTipWindow->setVisible(false);
        toolTipWindow = nullptr;
    }
}

juce::String Track::getName() const
{
    return this->nameLabel.getText();
}

juce::DynamicObject* Track::getJson() const
{

    auto* obj = new juce::DynamicObject{};

    obj->setProperty("name", nameLabel.getText());
    obj->setProperty("type", typeOfTrack);
    obj->setProperty("volume", volumeSlider.getValue());
    obj->setProperty("instrumentNumber", usedInstrumentNumber);
    obj->setProperty("uuid", uniqueIdentifierTrack.toString());

    return obj;
}

void Track::showInstantInfo(const juce::String& text)
{
    auto label = std::make_unique<juce::Label>();
    label->setText(text, juce::dontSendNotification);
    label->setJustificationType(juce::Justification::centred);
    label->setColour(juce::Label::backgroundColourId, juce::Colours::lightyellow);
    label->setColour(juce::Label::textColourId, juce::Colours::black);
    label->setBorderSize(juce::BorderSize<int>(2));
    label->setSize(300, 100);

    auto* display = findParentComponentOfClass<Display>();
    if (display != nullptr)
    {
        juce::CallOutBox::launchAsynchronously(std::move(label), display->getScreenBounds(), nullptr);
    }
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

void Track::openInstrumentChooser()
{
    std::unique_ptr<InstrumentChooserComponent> chooser = std::make_unique<InstrumentChooserComponent>(instrumentlist);
    chooser->instrumentSelectedFunction = [this](int instrumentIndex, const juce::String& name)
    {
        setInstrumentNumber(instrumentIndex, true);
        nameLabel.setText(name, juce::dontSendNotification);
        if (onChange)
            onChange();

    };

    auto* display = findParentComponentOfClass<Display>();
    auto bounds = display->getScreenBounds();

    chooser->setSize(bounds.getWidth(), bounds.getHeight());

    juce::CallOutBox::launchAsynchronously(std::move(chooser), bounds, nullptr);
}

juce::StringArray Track::instrumentListBuild()
{
    juce::StringArray instrumentListLocal = {
    "Acoustic Grand Piano",
    "Bright Acoustic Piano",
    "Electric Grand Piano",
    "Honky-tonk Piano",
    "Electric Piano 1",
    "Electric Piano 2",
    "Harpsichord",
    "Clavi",
    "Celesta",
    "Glockenspiel",
    "Music Box",
    "Vibraphone",
    "Marimba",
    "Xylophone",
    "Tubular Bells",
    "Dulcimer",
    "Drawbar Organ",
    "Percussive Organ",
    "Rock Organ",
    "Church Organ",
    "Reed Organ",
    "Accordion",
    "Harmonica",
    "Tango Accordion",
    "Acoustic Guitar (nylon)",
    "Acoustic Guitar (steel)",
    "Electric Guitar (jazz)",
    "Electric Guitar (clean)",
    "Electric Guitar (muted)",
    "Overdriven Guitar",
    "Distortion Guitar",
    "Guitar harmonics",
    "Acoustic Bass",
    "Electric Bass (finger)",
    "Electric Bass (pick)",
    "Fretless Bass",
    "Slap Bass 1",
    "Slap Bass 2",
    "Synth Bass 1",
    "Synth Bass 2",
    "Violin",
    "Viola",
    "Cello",
    "Contrabass",
    "Tremolo Strings",
    "Pizzicato Strings",
    "Orchestral Harp",
    "Timpani",
    "String Ensemble 1",
    "String Ensemble 2",
    "SynthStrings 1",
    "SynthStrings 2",
    "Choir Aahs",
    "Voice Oohs",
    "Synth Voice",
    "Orchestra Hit",
    "Trumpet",
    "Trombone",
    "Tuba",
    "Muted Trumpet",
    "French Horn",
    "Brass Section",
    "Synth Brass 1",
    "Synth Brass 2",
    "Soprano Sax",
    "Alto Sax",
    "Tenor Sax",
    "Baritone Sax",
    "Oboe",
    "English Horn",
    "Bassoon",
    "Clarinet",
    "Piccolo",
    "Flute",
    "Recorder",
    "Pan Flute",
    "Blown Bottle",
    "Shakuhachi",
    "Whistle",
    "Ocarina",
    "Lead 1 (square)",
    "Lead 2 (sawtooth)",
    "Lead 3 (calliope)",
    "Lead 4 (chiff)",
    "Lead 5 (charang)",
    "Lead 6 (voice)",
    "Lead 7 (fifths)",
    "Lead 8 (bass + lead)",
    "Pad 1 (new age)",
    "Pad 2 (warm)",
    "Pad 3 (polysynth)",
    "Pad 4 (choir)",
    "Pad 5 (bowed)",
    "Pad 6 (metallic)",
    "Pad 7 (halo)",
    "Pad 8 (sweep)",
    "FX 1 (rain)",
    "FX 2 (soundtrack)",
    "FX 3 (crystal)",
    "FX 4 (atmosphere)",
    "FX 5 (brightness)",
    "FX 6 (goblins)",
    "FX 7 (echoes)",
    "FX 8 (sci-fi)",
    "Sitar",
    "Banjo",
    "Shamisen",
    "Koto",
    "Kalimba",
    "Bag pipe",
    "Fiddle",
    "Shanai",
    "Tinkle Bell",
    "Agogo",
    "Steel Drums",
    "Woodblock",
    "Taiko Drum",
    "Melodic Tom",
    "Synth Drum",
    "Reverse Cymbal",
    "Guitar Fret Noise",
    "Breath Noise",
    "Seashore",
    "Bird Tweet",
    "Telephone Ring",
    "Helicopter",
    "Applause",
    "Gunshot"
    };
    return instrumentListLocal;
}

void Track::setInstrumentNumber(int newInstrumentNumber, bool shouldNotify)
{
    this->usedInstrumentNumber = newInstrumentNumber;
    if (usedInstrumentNumber != -1 && shouldNotify)
        notify(channel, -1, usedInstrumentNumber);
}

int Track::getInstrumentNumber() const
{
    return usedInstrumentNumber;
}

void Track::setVolumeSlider(double value)
{
    this->volumeSlider.setValue(value);
}

void Track::setVolumeLabel(const juce::String& value, bool shouldNotify)
{
    volumeLabel.setText(value, juce::dontSendNotification);
    if (value.getIntValue() != -1 && shouldNotify)
        notify(channel, value.getIntValue(), -1);
}

void Track::setNameLabel(const juce::String& name)
{
    nameLabel.setText(name, juce::dontSendNotification);
}

void Track::mouseDown(const juce::MouseEvent& event)
{
    if (event.eventComponent == &nameLabel)
    {
        if (event.mods.isLeftButtonDown())
        {
            if (onRequestTrackSelection)
            {
                onRequestTrackSelection([this](const juce::String& selectedTrack, const juce::Uuid& uuid, const juce::String& type)
                    {
                        setNameLabel(selectedTrack);
                        setUUID(uuid);
                        setTypeOfTrack(type);
                        if (type.trim() == "percussion")
                            this->channel = 10;
                    }
                );
            }
        }
        else if(event.mods.isRightButtonDown()){
            juce::PopupMenu menu;
            menu.addItem("Rename", [this]()
                {
                    renameOneTrack();
                });
            menu.addItem("Delete", [this]()
                {
                    deleteOneTrack();
                });
            menu.addItem("Copy", [this]()
                {
                    copyOneTrack();
                });

            menu.addItem("Paste", [this]()
                {
                    pasteOneTrack();
                });

            menu.addItem("Notes information", [this]()
                {
                   showNotesInformation();
                });

            menu.showMenuAsync(juce::PopupMenu::Options().withTargetComponent(&nameLabel));
        }
    }
}

void Track::setUUID(const juce::Uuid& newUUID)
{
    this->uniqueIdentifierTrack = newUUID;
}

void Track::setTypeOfTrack(const juce::String& newType)
{
    this->typeOfTrack = newType;
}

juce::String Track::getTypeOfTrack() const
{
    return this->typeOfTrack;
}

juce::Uuid Track::getUsedID() const
{
    return uniqueIdentifierTrack;
}

void Track::setChannel(int newChannel)
{
    this->channel = newChannel;
}

int Track::getChannel() const
{
    return this->channel;
}

double Track::getVolume() const
{
    return this->volumeSlider.getValue();
}

void Track::copyFrom(const Track& other)
{
    setNameLabel(other.getName());
    setInstrumentNumber(other.getInstrumentNumber());
    setTypeOfTrack(other.getTypeOfTrack());
    setVolumeSlider(other.getVolume());
    setVolumeLabel(juce::String(other.getVolume()));
    setUUID(other.getUsedID());
}

void Track::pasteFrom(const Track& source)
{
    copyFrom(source);
}

void Track::renameOneTrack()
{
    auto* window = new juce::AlertWindow{ "Rename track","Enter a name for your track",juce::AlertWindow::NoIcon};

    window->addTextEditor("nameEditor", nameLabel.getText(), "Track Name:");

    window->addButton("OK", 1, juce::KeyPress(juce::KeyPress::returnKey));
    window->addButton("Cancel", 0, juce::KeyPress(juce::KeyPress::escapeKey));

    window->enterModalState(true, juce::ModalCallbackFunction::create([this, window](int result)
        {
            std::unique_ptr<juce::AlertWindow> cleanup{ window };
            if (result != 1)
                return;



            juce::String newNameToPut=window->getTextEditor("nameEditor")->getText().trim();
            setNameLabel(newNameToPut);
            if (onChange)
                onChange();
        }
    ));

    juce::MessageManager::callAsync([window]()
        {
            if (auto* editor = window->getTextEditor("nameEditor"))
                editor->grabKeyboardFocus();
        });
}

void Track::deleteOneTrack()
{
    juce::AlertWindow::showOkCancelBox(
        juce::AlertWindow::WarningIcon,
        "Confirm Remove",
        "Are you sure you want to remove the selected track?",
        "Remove",
        "Cancel",
        this,
        juce::ModalCallbackFunction::create(
            [this](int result)
            {
                if (result == 0)
                    return;

                setNameLabel("None");
                setTypeOfTrack("None");
                setInstrumentNumber(-1);
                setUUID(juce::Uuid{ "noID" });

                if (onChange)
                    onChange();
            }
            ));
}

void Track::copyOneTrack()
{
    if (onCopy)
        onCopy(this);

    juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::InfoIcon, "Track info", "The track has been succesfully copied!");
}

void Track::pasteOneTrack()
{
    if (onPaste)
        onPaste(this);

    if (onChange)
        onChange();
}

void Track::showNotesInformation()
{
    if (onShowInformation)
        onShowInformation(uniqueIdentifierTrack, channel);
}
