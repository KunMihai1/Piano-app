#include <juce_core/juce_core.h>
#include "addDeviceWindow.h"

class AddDeviceWindowTest : public juce::UnitTest
{
public:
    AddDeviceWindowTest() : juce::UnitTest("AddDeviceWindow", "Component") {}

    void runTest() override
    {
        testCallbackCalledWithValidInput();
        testCallbackNotCalledWithInvalidInput();
    }

private:
    void testCallbackCalledWithValidInput()
    {
        beginTest("Callback is called with valid input");

        AddDeviceWindow window("VID123", "PID456", 1);

        bool callbackCalled = false;
        juce::String addedName;
        int addedKeys = 0;

        // Assign a lambda to capture callback arguments
        window.onAddDevice = [&](const juce::String& name, int keys)
        {
            callbackCalled = true;
            addedName = name;
            addedKeys = keys;
        };

        window.getNameEditor().setText("MyTestDevice");
        window.getKeysEditor().setText("61");

        if (window.getAddButton().onClick)
            window.getAddButton().onClick();

        expect(callbackCalled, "Expected onAddDevice callback to be called");
        expect(addedName == "MyTestDevice", "Expected device name to match input");
        expect(addedKeys == 61, "Expected number of keys to match input");
    }

    void testCallbackNotCalledWithInvalidInput()
    {
        beginTest("Callback not called with invalid input");

        AddDeviceWindow window("VID123", "PID456", 1);

        bool callbackCalled = false;

        // Assign a lambda to capture callback arguments
        window.onAddDevice = [&](const juce::String&, int)
        {
            callbackCalled = true;
        };

        // Empty name
        window.getNameEditor().setText("");
        window.getKeysEditor().setText("61");
        if (window.getAddButton().onClick)
            window.getAddButton().onClick();
        expect(!callbackCalled, "Callback should not be called with empty name");

        // Zero keys
        window.getNameEditor().setText("ValidName");
        window.getKeysEditor().setText("0");
        if (window.getAddButton().onClick)
            window.getAddButton().onClick();
        expect(!callbackCalled, "Callback should not be called with zero keys");
    }
};

static AddDeviceWindowTest addDeviceWindowTest;