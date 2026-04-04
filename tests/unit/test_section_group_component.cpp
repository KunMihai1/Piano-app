#include <juce_core/juce_core.h>
#include "SectionGroupComponent.h"

class SectionGroupComponentUnitTest : public juce::UnitTest
{
public:
    SectionGroupComponentUnitTest()
        : juce::UnitTest("SectionGroupComponent - Unit", "Unit") {}

    void runTest() override
    {
        testButtonCreation();
        testActivationMapInitialization();
        testToggleButtonAndTooltip();
        testMouseEnterExitTooltip();
    }

private:
    void testButtonCreation()
    {
        beginTest("Buttons are created correctly");

        std::vector<juce::String> names = { "A", "B", "C" };
        SectionGroupComponent group("TestGroup", names, false);

        auto& buttons = group.getButtons();
        expect(buttons.size() == 3);
        expect(buttons[0]->getButtonText() == "A");
        expect(buttons[1]->getButtonText() == "B");
        expect(buttons[2]->getButtonText() == "C");
    }

    void testActivationMapInitialization()
    {
        beginTest("Activation map initialized correctly");

        std::vector<juce::String> names = { "A", "B" };
        SectionGroupComponent group("TestGroup", names, false);

        auto& map = group.getActivationMap();
        expect(map.size() == 2);
        expect(map["A"] == false);
        expect(map["B"] == false);
    }

    void testToggleButtonAndTooltip()
    {
        beginTest("Toggle button and tooltip setup");

        std::vector<juce::String> names = { "Fill 1", "Fill 2" };
        SectionGroupComponent group("Fills", names, true);

        // Toggle button should exist
        expect(group.getToggleButton() != nullptr);

        // Tooltip should exist
        expect(group.getToggleToolTip() != nullptr);
        expect(!group.getToggleToolTip()->isVisible());
    }

    void testMouseEnterExitTooltip()
    {
        beginTest("Tooltip visibility on hover");

        std::vector<juce::String> names = { "Fill 1" };
        SectionGroupComponent group("Fills", names, true);

        auto* toggle = group.getToggleButton();
        auto* tooltip = group.getToggleToolTip();

        expect(toggle != nullptr);
        expect(tooltip != nullptr);

        // Grab a real mouse input source from Desktop
        auto& source = juce::Desktop::getInstance().getMainMouseSource();

        // Create a MouseEvent with required parameters
        juce::MouseEvent e(
            source,                            // real mouse input source
            juce::Point<float>(0, 0),         // local position
            juce::ModifierKeys::noModifiers,  // no modifiers
            0.0f, // pressure
            0.0f, // orientation
            0.0f, // rotation
            0.0f, // tiltX
            0.0f, // tiltY
            toggle,   // eventComponent
            toggle,   // originator
            juce::Time::getCurrentTime(),     // event time
            juce::Point<float>(0, 0),         // mouse down pos
            juce::Time::getCurrentTime(),     // mouse down time
            1,                                // number of clicks
            false                             // mouse was dragged
        );

        // Simulate mouse enter
        group.mouseEnter(e);
         
        expect(tooltip->isVisible());

        // Simulate mouse exit
        group.mouseExit(e);
        expect(!tooltip->isVisible());
    }
};

static SectionGroupComponentUnitTest sectionGroupComponentUnitTest;