#include "SFZLibraryUI.h"

SFZLibraryUI::SFZLibraryUI(SFZLibraryManager& managerRef) : manager(managerRef)
{
    buildInstrumentList();

    addAndMakeVisible(libraryList);
    libraryList.setModel(this);

    addAndMakeVisible(statusLabel);
    statusLabel.setJustificationType(juce::Justification::centred);
    statusLabel.setColour(juce::Label::textColourId, juce::Colours::lightgreen);

    addAndMakeVisible(addFileButton);
    addFileButton.onClick = [this] { addFileButtonClicked(); };

    addAndMakeVisible(removeFileButton);
    removeFileButton.onClick = [this] { removeFileButtonClicked(); };

    addAndMakeVisible(assignButton);
    assignButton.onClick = [this]
    {
        int selectedRow = libraryList.getSelectedRow();
        if (selectedRow < 0 || selectedRow >= (int)manager.getEntries().size())
        {
            juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::WarningIcon,
                "No file selected", "Please select an SFZ file from the list first.");
            return;
        }

        const auto& entryId = manager.getEntries()[selectedRow].id;
        juce::String styleId = getCurrentStyleId ? getCurrentStyleId() : juce::String();
        if (styleId.isEmpty())
            styleId = "style_default";

        juce::String styleName = getCurrentStyleName ? getCurrentStyleName() : juce::String("DEFAULT Style");

        auto chooser = std::make_unique<InstrumentChooserComponent>(instrumentList);
        chooser->instrumentSelectedFunction = [this, styleId, styleName, entryId](int instrumentIndex, const juce::String& name)
        {
            manager.assignToStyleInstrument(styleId, instrumentIndex, entryId);
            if (onLibraryChanged) onLibraryChanged();
            updateStatusLabel();
            libraryList.repaint();

            juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::InfoIcon,
                "Assigned", "SFZ assigned to style: " + styleName + "\nInstrument: " + name + " (" + juce::String(instrumentIndex) + ")");
        };

        if (auto* display = findParentComponentOfClass<juce::Component>()) // Try to get a good parent bounds, or use desktop
        {
            auto bounds = getScreenBounds();
            chooser->setSize(300, 400);
            juce::CallOutBox::launchAsynchronously(std::move(chooser), bounds, nullptr);
        }
    };

    addAndMakeVisible(closeButton);
    closeButton.onClick = [this] {
        if (onClose) onClose();
        else setVisible(false);
    };
}

SFZLibraryUI::~SFZLibraryUI()
{
    libraryList.setModel(nullptr);
}

void SFZLibraryUI::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::darkgrey);
    g.setColour(juce::Colours::white);
    g.setFont(16.0f);
    g.drawText("SFZ Library", getLocalBounds().removeFromTop(30), juce::Justification::centred, true);

    updateStatusLabel();
}

void SFZLibraryUI::resized()
{
    auto bounds = getLocalBounds();
    closeButton.setBounds(bounds.getRight() - 35, 5, 30, 20);
    bounds.removeFromTop(30); // Header space

    statusLabel.setBounds(bounds.removeFromTop(30).reduced(5, 0));

    auto buttonArea = bounds.removeFromBottom(40).reduced(5);
    int btnW = buttonArea.getWidth() / 3;
    addFileButton.setBounds(buttonArea.removeFromLeft(btnW).reduced(2));
    assignButton.setBounds(buttonArea.removeFromLeft(btnW).reduced(2));
    removeFileButton.setBounds(buttonArea.reduced(2));

    libraryList.setBounds(bounds.reduced(5));
}

int SFZLibraryUI::getNumRows()
{
    return static_cast<int>(manager.getEntries().size());
}

void SFZLibraryUI::paintListBoxItem(int rowNumber, juce::Graphics& g, int width, int height, bool rowIsSelected)
{
    if (rowIsSelected)
        g.fillAll(juce::Colours::coral); // Match InstrumentChooserComponent selection color

    if (rowNumber < manager.getEntries().size())
    {
        const auto& entry = manager.getEntries()[rowNumber];

        juce::String styleId = getCurrentStyleId ? getCurrentStyleId() : juce::String();
        if (styleId.isEmpty())
            styleId = "style_default";

        juce::String assignedInsts = "";
        bool isAssigned = false;
        auto styleIt = manager.getData().styleMappings.find(styleId);
        if (styleIt != manager.getData().styleMappings.end())
        {
            for (const auto& pair : styleIt->second) {
                if (pair.second == entry.id) {
                    isAssigned = true;
                    assignedInsts += juce::String(pair.first) + ", ";
                }
            }
        }

        juce::String textToDraw = entry.name + " (" + entry.sfzPath + ")";

        if (isAssigned)
        {
            assignedInsts = assignedInsts.dropLastCharacters(2);
            textToDraw += "   [Assigned: " + assignedInsts + "]";
        }

        if (rowIsSelected)
            g.setColour(juce::Colours::black); // Better contrast against coral
        else if (isAssigned)
            g.setColour(juce::Colours::lightgreen);
        else
            g.setColour(juce::Colours::peachpuff); // Match InstrumentChooserComponent text color

        g.drawText(textToDraw, 10, 0, width - 20, height, juce::Justification::centredLeft, true);
    }
}

void SFZLibraryUI::addFileButtonClicked()
{
    fileChooser = std::make_unique<juce::FileChooser>("Select SFZ File", juce::File(), "*.sfz");
    auto chooserFlags = juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles;

    fileChooser->launchAsync(chooserFlags, [this](const juce::FileChooser& fc)
    {
        auto file = fc.getResult();
        if (file.existsAsFile())
        {
            manager.addFile(file);
            libraryList.updateContent();
            if (onLibraryChanged) onLibraryChanged();
        }
    });
}

void SFZLibraryUI::removeFileButtonClicked()
{
    int selectedRow = libraryList.getSelectedRow();
    if (selectedRow >= 0 && selectedRow < (int)manager.getEntries().size())
    {
        manager.removeEntry(manager.getEntries()[selectedRow].id);
        libraryList.updateContent();
        if (onLibraryChanged) onLibraryChanged();
        updateStatusLabel();
    }
}

void SFZLibraryUI::updateStatusLabel()
{
    juce::String styleName = getCurrentStyleName ? getCurrentStyleName() : juce::String("DEFAULT Style");
    statusLabel.setText("Current Style: " + styleName, juce::dontSendNotification);
}

void SFZLibraryUI::buildInstrumentList()
{
    instrumentList = {
        "Acoustic Grand Piano", "Bright Acoustic Piano", "Electric Grand Piano", "Honky-tonk Piano",
        "Electric Piano 1", "Electric Piano 2", "Harpsichord", "Clavi",
        "Celesta", "Glockenspiel", "Music Box", "Vibraphone",
        "Marimba", "Xylophone", "Tubular Bells", "Dulcimer",
        "Drawbar Organ", "Percussive Organ", "Rock Organ", "Church Organ",
        "Reed Organ", "Accordion", "Harmonica", "Tango Accordion",
        "Acoustic Guitar (nylon)", "Acoustic Guitar (steel)", "Electric Guitar (jazz)", "Electric Guitar (clean)",
        "Electric Guitar (muted)", "Overdriven Guitar", "Distortion Guitar", "Guitar harmonics",
        "Acoustic Bass", "Electric Bass (finger)", "Electric Bass (pick)", "Fretless Bass",
        "Slap Bass 1", "Slap Bass 2", "Synth Bass 1", "Synth Bass 2",
        "Violin", "Viola", "Cello", "Contrabass",
        "Tremolo Strings", "Pizzicato Strings", "Orchestral Harp", "Timpani",
        "String Ensemble 1", "String Ensemble 2", "SynthStrings 1", "SynthStrings 2",
        "Choir Aahs", "Voice Oohs", "Synth Voice", "Orchestra Hit",
        "Trumpet", "Trombone", "Tuba", "Muted Trumpet",
        "French Horn", "Brass Section", "Synth Brass 1", "Synth Brass 2",
        "Soprano Sax", "Alto Sax", "Tenor Sax", "Baritone Sax",
        "Oboe", "English Horn", "Bassoon", "Clarinet",
        "Piccolo", "Flute", "Recorder", "Pan Flute",
        "Blown Bottle", "Shakuhachi", "Whistle", "Ocarina",
        "Lead 1 (square)", "Lead 2 (sawtooth)", "Lead 3 (calliope)", "Lead 4 (chiff)",
        "Lead 5 (charang)", "Lead 6 (voice)", "Lead 7 (fifths)", "Lead 8 (bass + lead)",
        "Pad 1 (new age)", "Pad 2 (warm)", "Pad 3 (polysynth)", "Pad 4 (choir)",
        "Pad 5 (bowed)", "Pad 6 (metallic)", "Pad 7 (halo)", "Pad 8 (sweep)",
        "FX 1 (rain)", "FX 2 (soundtrack)", "FX 3 (crystal)", "FX 4 (atmosphere)",
        "FX 5 (brightness)", "FX 6 (goblins)", "FX 7 (echoes)", "FX 8 (sci-fi)",
        "Sitar", "Banjo", "Shamisen", "Koto",
        "Kalimba", "Bag pipe", "Fiddle", "Shanai",
        "Tinkle Bell", "Agogo", "Steel Drums", "Woodblock",
        "Taiko Drum", "Melodic Tom", "Synth Drum", "Reverse Cymbal",
        "Guitar Fret Noise", "Breath Noise", "Seashore", "Bird Tweet",
        "Telephone Ring", "Helicopter", "Applause", "Gunshot"
    };
}
