#include <juce_core/juce_core.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include "keyListener.h"

class KeyboardListenerIntegrationTest : public juce::UnitTest
{
public:
	KeyboardListenerIntegrationTest() : juce::UnitTest("KeyboardListener-Integration", "Integration"), device{}, handler{ device }, listener{handler} {}

	void runTest() override
	{
        testKeyPressedBehavior();
	}

private:
	MidiDevice device;
	MidiHandler handler;
	KeyboardListener listener;

    void testKeyPressedBehavior()
    {
        beginTest("keyPressed - returns false when keyboard input is disabled");
        {
            juce::KeyPress keyA('A');
            expect(listener.keyPressed(keyA, nullptr) == false);
        }

        beginTest("keyPressed - returns false when keyboard input is enabled");
        {
            listener.setIsKeyboardInput(true);
            juce::KeyPress keyA('A');
            expect(listener.keyPressed(keyA, nullptr) == false);
        }

        beginTest("keyPressed - unmapped key does not crash when enabled");
        {
            listener.setIsKeyboardInput(true);
            juce::KeyPress keyZ('Z');
            expect(listener.keyPressed(keyZ, nullptr) == false);
        }
    }

};