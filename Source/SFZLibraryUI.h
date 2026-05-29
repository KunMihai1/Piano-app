#pragma once

#include <JuceHeader.h>
#include "SFZlibrary.h"
#include "InstrumentChooser.h"

class SFZLibraryUI : public juce::Component, public juce::ListBoxModel
{
public:
    SFZLibraryUI(SFZLibraryManager& managerRef);
    ~SFZLibraryUI() override;

    std::function<void()> onClose;
    std::function<void()> onLibraryChanged;
    std::function<juce::String()> getCurrentStyleId;  // returns empty string if no style selected
    std::function<juce::String()> getCurrentStyleName; // returns empty string if no style selected

    void paint(juce::Graphics& g) override;
    void resized() override;

    int getNumRows() override;
    void paintListBoxItem(int rowNumber, juce::Graphics& g, int width, int height, bool rowIsSelected) override;

private:
    void addFileButtonClicked();
    void removeFileButtonClicked();
    void updateStatusLabel();
    void buildInstrumentList();

    SFZLibraryManager& manager;
    juce::ListBox libraryList;
    juce::TextButton addFileButton{"Add SFZ File"};
    juce::TextButton removeFileButton{"Remove File"};
    juce::TextButton assignButton{"Assign to Style"};
    juce::TextButton closeButton{"X"};
    juce::Label statusLabel;
    std::unique_ptr<juce::FileChooser> fileChooser;
    juce::StringArray instrumentList;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SFZLibraryUI)
};
