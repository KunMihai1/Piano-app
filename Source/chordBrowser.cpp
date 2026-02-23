/*
  ==============================================================================

    chordBrowser.cpp
    Created: 22 Feb 2026 9:52:13pm
    Author:  Kisuke

  ==============================================================================
*/

#include "chordBrowser.h"

ChordPreview::ChordPreview()
{
    const juce::String labels[] = { "Root", "1st Inversion", "2nd Inversion" };

    for (int i = 0; i < 3; ++i)
    {
        auto* container = new juce::Component();
        addAndMakeVisible(container);
        containers.add(container);

        auto* lbl = new juce::Label();
        lbl->setText(labels[i], juce::dontSendNotification);
        lbl->setJustificationType(juce::Justification::centred);
        lbl->setFont(juce::Font(13.0f, juce::Font::bold));
        container->addAndMakeVisible(lbl);
        labelsArray.add(lbl);

        auto* imageComp = new juce::ImageComponent();
        imageComp->setMouseCursor(juce::MouseCursor::PointingHandCursor);
        imageComp->setOpaque(true);
        imageComp->setInterceptsMouseClicks(false, false);
        container->addAndMakeVisible(imageComp);
        imageComponents.add(imageComp);
    }
}

void ChordPreview::resized()
{
    auto area = getLocalBounds().reduced(6);
    int singleH = area.getHeight() / 3;

    for (int i = 0; i < containers.size(); ++i)
    {
        auto row = area.removeFromTop(singleH);
        containers[i]->setBounds(row);

        
        auto containerLocal = containers[i]->getLocalBounds();

        
        auto labelArea = containerLocal.removeFromTop(22).reduced(4, 2);
        labelsArray[i]->setBounds(labelArea);

        
        imageComponents[i]->setBounds(containerLocal.reduced(4));
    }
}

void ChordPreview::showChordImages(const Chord& chord)
{
    const juce::Image images[] = { chord.imgRoot, chord.imgInv1, chord.imgInv2 };

    for (int i = 0; i < imageComponents.size(); ++i)
    {
        if (!images[i].isNull())
            imageComponents[i]->setImage(images[i]);
        else
            imageComponents[i]->setImage(juce::Image()); 
    }
}

void ChordPreview::clearImages()
{
    for (auto* img : imageComponents)
        img->setImage(juce::Image());
}


ChordRowComponent::ChordRowComponent(const Chord& chord, std::function<void(const Chord& c)> onClick, std::function<void(const Chord& c)> onHoverEnter, std::function<void()> onHoverExit)
    :chordData{chord}, onClick{onClick}, onHoverEnter{onHoverEnter}, onHoverExit{onHoverExit}
{
    setInterceptsMouseClicks(true, true);
    setMouseCursor(juce::MouseCursor::PointingHandCursor);
}

ChordRowComponent::~ChordRowComponent()
{
}

void ChordRowComponent::updateChordData(const Chord& c)
{
    chordData = c;
    repaint();
}

void ChordRowComponent::paint(juce::Graphics& g)
{
    auto r = getLocalBounds();
    if (isMouseOver())
        g.fillAll(juce::Colour(0xffe8f0ff));
    g.setColour(juce::Colours::lightgrey);
    g.setFont(20.0f);
    g.drawText(chordData.name, r.reduced(6, 4), juce::Justification::centredLeft);
}

void ChordRowComponent::mouseEnter(const juce::MouseEvent& ev)
{
    if (onHoverEnter) onHoverEnter(chordData);
    repaint();
}

void ChordRowComponent::mouseExit(const juce::MouseEvent& ev)
{
    if (onHoverExit) onHoverExit();
    repaint();
}

void ChordRowComponent::mouseDown(const juce::MouseEvent& ev)
{
    if (onClick) onClick(chordData);
}

ChordBrowserComponent::ChordBrowserComponent(const std::vector<Chord>& allChords): allChords{allChords}
{
    addAndMakeVisible(search);
    search.setTextToShowWhenEmpty("Search chords...", juce::Colours::grey);
    search.addListener(this);

    addAndMakeVisible(listBox);


    listBox.setModel(this);


    addAndMakeVisible(preview);

    rebuildFilteredList();
    setSize(1200, 750);
}

ChordBrowserComponent::~ChordBrowserComponent()
{
    listBox.setModel(nullptr);
    listBox.updateContent();


    search.removeListener(this);
}

int ChordBrowserComponent::getNumRows()
{
    return (int)filteredIndices.size();
}

juce::Component* ChordBrowserComponent::refreshComponentForRow(
    int rowNumber, bool isRowSelected, juce::Component* existingComponentToUpdate)
{
    if (rowNumber < 0 || rowNumber >= (int)filteredIndices.size())
    {
        if (existingComponentToUpdate != nullptr)
            delete existingComponentToUpdate;

        return nullptr;
    }
    const Chord& c = allChords[filteredIndices[(size_t)rowNumber]];

    
    auto* row = dynamic_cast<ChordRowComponent*>(existingComponentToUpdate);

    if (row == nullptr)
    {
        
        if (existingComponentToUpdate != nullptr)
            delete existingComponentToUpdate;

        

        row = new ChordRowComponent(c,
            [this](const Chord& chosen) { if (onChordChosen) onChordChosen(chosen); },
            [this](const Chord& hovered) { showPreviewForChord(hovered); },
            [this]() { clearPreview(); });
    }
    else
    {
        
        row->updateChordData(c);
    }

    return row;
}

void ChordBrowserComponent::paint(juce::Graphics& g)
{
}

void ChordBrowserComponent::resized()
{
    auto r = getLocalBounds().reduced(8);
    auto top = r.removeFromTop(28);
    search.setBounds(top.removeFromLeft(r.getWidth()));

    
    auto left = r.removeFromLeft(getWidth() / 3);
    listBox.setBounds(left.reduced(2));

    preview.setBounds(r.reduced(4));
}

void ChordBrowserComponent::paintListBoxItem(int rowNumber, juce::Graphics&, int width, int height, bool rowIsSelected)
{
}


void ChordBrowserComponent::textEditorTextChanged(juce::TextEditor& editor)
{
    filterText = editor.getText();
    rebuildFilteredList();
}

void ChordBrowserComponent::textEditorReturnKeyPressed(juce::TextEditor& editor)
{
}

void ChordBrowserComponent::textEditorEscapeKeyPressed(juce::TextEditor& editor)
{
}

void ChordBrowserComponent::textEditorFocusLost(juce::TextEditor& editor)
{
}

void ChordBrowserComponent::rebuildFilteredList()
{
    filteredIndices.clear();
    for (int i = 0; i < (int)allChords.size(); ++i)
    {
        if (filterText.isEmpty()
            || allChords[(size_t)i].name.toLowerCase().contains(filterText.toLowerCase()))
        {
            filteredIndices.push_back(i);
        }
    }
    listBox.updateContent();
}

void ChordBrowserComponent::showPreviewForChord(const Chord& chord)
{
    preview.showChordImages(chord);

}

void ChordBrowserComponent::clearPreview()
{
    preview.clearImages();
}
