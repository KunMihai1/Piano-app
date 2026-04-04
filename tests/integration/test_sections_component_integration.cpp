#include <juce_core/juce_core.h>
#include "SectionsComponent.h"

class SectionsComponentIntegrationTest : public juce::UnitTest
{
public:
    SectionsComponentIntegrationTest()
        : juce::UnitTest("StyleSectionComponent - Integration", "Integration") {}

    void runTest() override
    {
        testSingleButtonActivation();
        testButtonToggle();
        testOnlyOneActiveAtATime();
        testCrossGroupDeactivation();
        testCallbacksExecuted();
        testResizedLayout();
        testDeactivateLastClicked();
    }

private:
    std::unique_ptr<StyleSectionComponent> createComponent(
        bool& callbackA, bool& callbackB)
    {
        std::vector<juce::String> names = { "Main" };
        std::vector<std::vector<juce::String>> groupNames = { {"A", "B"} };

        std::unordered_map<juce::String, std::function<void()>> callbacks{
            {"A", [&]() { callbackA = true; }},
            {"B", [&]() { callbackB = true; }}
        };

        return std::make_unique<StyleSectionComponent>(names, groupNames, callbacks);
    }

    void testCrossGroupDeactivation()
    {
        beginTest("Clicking a button in one group deactivates the last button in the other group");

        bool intro1 = false, intro2 = false;
        bool var1 = false, var2 = false;

        // Create introsEndings component
        std::unordered_map<juce::String, std::function<void()>> callbacksIntros{
            {"Intro 1", [&]() { intro1 = true; }},
            {"Intro 2", [&]() { intro2 = true; }}
        };

        auto introsEndings = std::make_unique<StyleSectionComponent>(
            std::vector<juce::String>{"Intros"},
            std::vector<std::vector<juce::String>>{ {"Intro 1", "Intro 2"}},
            callbacksIntros
            );

        // Create variationsFills component
        std::unordered_map<juce::String, std::function<void()>> callbacksVars{
            {"Var 1", [&]() { var1 = true; }},
            {"Var 2", [&]() { var2 = true; }}
        };

        auto variationsFills = std::make_unique<StyleSectionComponent>(
            std::vector<juce::String>{"Variations"},
            std::vector<std::vector<juce::String>>{ {"Var 1", "Var 2"}},
            callbacksVars
            );

        // Wire cross-deactivation
        introsEndings->undoLastClickedForOtherGroupSections = [&]()
        {
            variationsFills->deactivateLastClicked();
        };

        variationsFills->undoLastClickedForOtherGroupSections = [&]()
        {
            introsEndings->deactivateLastClicked();
        };

        auto& introButtons = introsEndings->getSectionGroups()[0]->getButtons();
        auto& varButtons = variationsFills->getSectionGroups()[0]->getButtons();

        // Step 1: Click Intro 1
        introButtons[0]->onClick();
        expect(introsEndings->getLastUsedButton() == introButtons[0].get());
        expect(variationsFills->getLastUsedButton() == nullptr);

        // Step 2: Click Var 2 (from another component)
        varButtons[1]->onClick();

        // Intro 1 should now be deactivated
        expect(introsEndings->getLastUsedButton() == nullptr);
        expect(variationsFills->getLastUsedButton() == varButtons[1].get());

        // Callbacks should all have executed
        expect(intro1 == true);
        expect(var2 == true);
    }

    void testResizedLayout()
    {
        beginTest("resized() lays out section groups correctly");

        bool a = false, b = false;
        auto comp = createComponent(a, b); // creates 1 group with 2 buttons

        // Add a second group manually to test multiple groups layout
        std::vector<juce::String> names = { "Group1", "Group2" };
        std::vector<std::vector<juce::String>> buttonsPerGroup = { {"A", "B"}, {"C", "D"} };
        std::unordered_map<juce::String, std::function<void()>> callbacks = {
            {"A", [&]() { a = true; }}, {"B", [&]() { b = true; }},
            {"C", [&]() {}}, {"D", [&]() {}}
        };
        auto compMulti = std::make_unique<StyleSectionComponent>(names, buttonsPerGroup, callbacks);

        // Trigger layout
        compMulti->setBounds(0, 0, 400, 100);
        compMulti->resized();

        auto& groups = compMulti->getSectionGroups();
        expect(groups.size() == 2);

        int groupWidth = 400 / 2;
        int reducedWidth = groupWidth - 10; // because resized() uses reduced(5)

        expect(groups[0]->getWidth() == reducedWidth);
        expect(groups[1]->getWidth() == reducedWidth);

        // Check that buttons exist inside the groups
        expect(groups[0]->getButtons().size() == 2);
        expect(groups[1]->getButtons().size() == 2);
    }

    void testSingleButtonActivation()
    {
        beginTest("Clicking a button activates it");

        bool a = false, b = false;
        auto comp = createComponent(a, b);
        auto& buttons = comp->getSectionGroups()[0]->getButtons();

        buttons[0]->onClick();

        expect(comp->getLastUsedButton() == buttons[0].get());
        expect(a == true);
        expect(b == false);
    }

    void testButtonToggle()
    {
        beginTest("Clicking the same button again deactivates it");

        bool a = false, b = false;
        auto comp = createComponent(a, b);
        auto& buttons = comp->getSectionGroups()[0]->getButtons();

        buttons[0]->onClick(); // activate
        buttons[0]->onClick(); // deactivate

        expect(comp->getLastUsedButton() == nullptr);
        expect(a == true); // callback still executed
    }

    void testOnlyOneActiveAtATime()
    {
        beginTest("Only one button active at a time");

        bool a = false, b = false;
        auto comp = createComponent(a, b);
        auto& buttons = comp->getSectionGroups()[0]->getButtons();

        buttons[0]->onClick();
        buttons[1]->onClick();

        expect(comp->getLastUsedButton() == buttons[1].get());
        expect(a == true);
        expect(b == true);

        // First button should be deactivated
        // Check its colour state indirectly (optional)
    }

    void testCallbacksExecuted()
    {
        beginTest("Callbacks are executed correctly");

        bool a = false, b = false;
        auto comp = createComponent(a, b);
        auto& buttons = comp->getSectionGroups()[0]->getButtons();

        buttons[0]->onClick();
        buttons[1]->onClick();

        expect(a == true);
        expect(b == true);
    }

    void testDeactivateLastClicked()
    {
        beginTest("deactivateLastClicked works");

        bool a = false, b = false;
        auto comp = createComponent(a, b);
        auto& buttons = comp->getSectionGroups()[0]->getButtons();

        buttons[0]->onClick();
        comp->deactivateLastClicked();

        expect(comp->getLastUsedButton() == nullptr);

        // Click again to ensure toggle works after deactivate
        buttons[1]->onClick();
        expect(comp->getLastUsedButton() == buttons[1].get());
    }
};

static SectionsComponentIntegrationTest sectionsComponentIntegrationTest;