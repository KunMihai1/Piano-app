#pragma once
#include <JuceHeader.h>
#include <functional>

/** Lists *.style files from the ArrangerStyles folder; fires callbacks to load or edit. */
class ArrangerStyleListComponent : public juce::Component,
                                   private juce::ListBoxModel
{
public:
    std::function<void (const juce::File&)> onLoadStyle;
    std::function<void (const juce::File&)> onEditStyle;
    std::function<void (const juce::File&)> onDeleteStyle;   // fired after a config file is deleted
    std::function<void()> onCreateNew;
    std::function<void()> onClose;

    ArrangerStyleListComponent();
    void refresh();                 // rescan the folder
    /** Mark the configuration that is currently active for the style (check + "active"
        + subtle highlight). Pass an empty string to clear the marker. */
    void setActiveConfigName (const juce::String& name);
    void resized() override;
    void paint (juce::Graphics& g) override;

    int getNumRows() override;
    void paintListBoxItem (int row, juce::Graphics& g, int w, int h, bool selected) override;
    void listBoxItemDoubleClicked (int row, const juce::MouseEvent&) override;

private:
    juce::Label title { {}, "Saved section configurations" };
    juce::ListBox list { "styles", this };
    juce::TextButton newBtn { "New" }, editBtn { "Edit" }, deleteBtn { "Delete" }, loadBtn { "Load" }, closeBtn { "Close" };
    juce::Array<juce::File> files;
    juce::String activeName;        // configuration currently active for the style (marked in the list)
};
