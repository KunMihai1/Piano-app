#include "ArrangerStyleListComponent.h"
#include "IOHelper.h"

ArrangerStyleListComponent::ArrangerStyleListComponent()
{
    addAndMakeVisible (title);
    title.setColour (juce::Label::textColourId, juce::Colours::white);
    addAndMakeVisible (list);
    addAndMakeVisible (newBtn);
    addAndMakeVisible (editBtn);
    addAndMakeVisible (deleteBtn);
    addAndMakeVisible (loadBtn);
    addAndMakeVisible (closeBtn);

    closeBtn.onClick = [this] { if (onClose) onClose(); };
    newBtn.onClick  = [this] { if (onCreateNew) onCreateNew(); };
    editBtn.onClick = [this]
    {
        const int r = list.getSelectedRow();
        if (r >= 0 && r < files.size() && onEditStyle) onEditStyle (files[r]);
    };
    deleteBtn.onClick = [this]
    {
        const int r = list.getSelectedRow();
        if (r < 0 || r >= files.size()) return;
        const auto file = files[r];
        juce::AlertWindow::showOkCancelBox (juce::AlertWindow::WarningIcon,
            "Delete configuration",
            "Delete \"" + file.getFileNameWithoutExtension() + "\"?\nThis cannot be undone.",
            "Delete", "Cancel", this,
            juce::ModalCallbackFunction::create ([this, file] (int result)
            {
                if (result != 1) return;          // 1 = Delete, 0 = Cancel
                file.deleteFile();
                if (onDeleteStyle) onDeleteStyle (file);   // let host clear it if it was active
                refresh();
            }));
    };
    loadBtn.onClick = [this]
    {
        const int r = list.getSelectedRow();
        if (r >= 0 && r < files.size() && onLoadStyle) onLoadStyle (files[r]);
    };
    refresh();
}

void ArrangerStyleListComponent::refresh()
{
    files.clear();
    auto folder = IOHelper::getArrangerStylesFolder();
    for (auto& f : folder.findChildFiles (juce::File::findFiles, false, "*.style"))
        files.add (f);
    list.updateContent();
    repaint();
}

int ArrangerStyleListComponent::getNumRows() { return files.size(); }

void ArrangerStyleListComponent::setActiveConfigName (const juce::String& name)
{
    if (activeName == name) return;
    activeName = name;
    repaint();
}

void ArrangerStyleListComponent::paintListBoxItem (int row, juce::Graphics& g, int w, int h, bool selected)
{
    if (row < 0 || row >= files.size()) return;

    const auto rowName  = files[row].getFileNameWithoutExtension();
    const bool isActive = activeName.isNotEmpty() && rowName == activeName;

    if (selected)
        g.fillAll (juce::Colour (0xff2a6cc4));           // clear blue highlight for the selected row
    else if (isActive)
        g.fillAll (juce::Colour (0xff203a2b));           // subtle green wash for the active configuration

    const juce::Colour accent = selected ? juce::Colours::white : juce::Colour (0xff5fd08a);
    const juce::Colour text   = selected ? juce::Colours::white : juce::Colours::lightgrey;

    int nameX = 6;
    if (isActive)                                        // leading check mark, name indented to match
    {
        g.setColour (accent);
        g.drawText (juce::String::fromUTF8 ("\xE2\x9C\x93"), 6, 0, 14, h, juce::Justification::centredLeft);
        nameX = 22;
    }

    const int activeLabelW = isActive ? 48 : 0;
    g.setColour (text);
    g.drawText (rowName, nameX, 0, w - nameX - 6 - activeLabelW, h, juce::Justification::centredLeft);

    if (isActive)                                        // trailing "active" tag
    {
        g.setColour (accent);
        g.setFont (juce::jmin (12.0f, (float) h - 4.0f));
        g.drawText ("active", w - 6 - activeLabelW, 0, activeLabelW, h, juce::Justification::centredRight);
    }
}

void ArrangerStyleListComponent::listBoxItemDoubleClicked (int row, const juce::MouseEvent&)
{
    if (row >= 0 && row < files.size() && onLoadStyle) onLoadStyle (files[row]);
}

void ArrangerStyleListComponent::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colour (0xff181818));
}

void ArrangerStyleListComponent::resized()
{
    auto area = getLocalBounds().reduced (6);
    title.setBounds (area.removeFromTop (20));
    area.removeFromTop (4);

    auto bottom = area.removeFromBottom (28);
    const int bw = bottom.getWidth() / 5;
    newBtn.setBounds    (bottom.removeFromLeft (bw).reduced (2));
    editBtn.setBounds   (bottom.removeFromLeft (bw).reduced (2));
    deleteBtn.setBounds (bottom.removeFromLeft (bw).reduced (2));
    loadBtn.setBounds   (bottom.removeFromLeft (bw).reduced (2));
    closeBtn.setBounds  (bottom.reduced (2));

    area.removeFromBottom (4);
    list.setBounds (area);
}
