#include <juce_core/juce_core.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include "OverlayComponent.h"

// ==================================================================
// INTEGRATION TESTS
// These tests verify OverlayComponent behavior in realistic
// scenarios where callbacks interact with external state,
// simulating how MainComponent wires up the overlay.
// ==================================================================

class OverlayComponentIntegrationTest : public juce::UnitTest
{
public:
    OverlayComponentIntegrationTest()
        : juce::UnitTest("OverlayComponent Integration", "Integration") {}

    void runTest() override
    {
        // ---- Simulated MainComponent integration: ESC open/close cycle ----

        beginTest("Integration - ESC open/close cycle simulates MainComponent behavior");
        {
            // Simulate the overlayWindow unique_ptr pattern from MainComponent
            std::unique_ptr<OverlayComponent> overlayWindow = nullptr;
            bool overlayShouldBeVisible = false;

            // First ESC press: create and show overlay
            overlayWindow = std::make_unique<OverlayComponent>();
            overlayWindow->setBounds(0, 0, 1920, 1080);
            overlayWindow->showOverlay();
            overlayShouldBeVisible = true;

            overlayWindow->onRequestClose = [&overlayWindow, &overlayShouldBeVisible]()
            {
                if (overlayWindow)
                {
                    overlayShouldBeVisible = false;
                    overlayWindow.reset();
                }
            };

            expect(overlayWindow != nullptr);
            expect(overlayWindow->isVisible() == true);
            expect(overlayShouldBeVisible == true);

            // Second ESC press: close overlay via onRequestClose
            if (overlayWindow && overlayWindow->onRequestClose)
                overlayWindow->onRequestClose();

            expect(overlayWindow == nullptr);
            expect(overlayShouldBeVisible == false);
        }

        // ---- Settings button opens settings window ----

        beginTest("Integration - Settings button triggers MIDI window creation");
        {
            OverlayComponent overlay;
            bool midiWindowCreated = false;
            bool midiWindowShouldBeVisible = false;

            overlay.onSettingsClick = [&midiWindowCreated, &midiWindowShouldBeVisible]()
            {
                midiWindowShouldBeVisible = true;
                midiWindowCreated = true;
            };

            // Simulate clicking the settings button
            overlay.getSettingsButton().onClick();

            expect(midiWindowCreated == true);
            expect(midiWindowShouldBeVisible == true);
        }

        // ---- setWindowFlag callback integration ----

        beginTest("Integration - setWindowFlag updates external state on ESC without onRequestClose");
        {
            OverlayComponent overlay;
            bool overlayShouldBeVisible = true;

            overlay.onRequestClose = nullptr; // no close handler
            overlay.setWindowFlag = [&overlayShouldBeVisible]()
            {
                overlayShouldBeVisible = false;
            };

            overlay.showOverlay();
            expect(overlay.isVisible() == true);

            // Press ESC — falls through to setWindowFlag branch
            overlay.keyPressed(juce::KeyPress(juce::KeyPress::escapeKey));

            expect(overlayShouldBeVisible == false);
            expect(overlay.isVisible() == false);
        }

        // ---- bringSeparateWindowFront callback integration ----

        beginTest("Integration - bringSeparateWindowFront callback brings MIDI window to front");
        {
            OverlayComponent overlay;
            bool midiWindowBroughtToFront = false;

            overlay.bringSeparateWindowFront = [&midiWindowBroughtToFront]()
            {
                midiWindowBroughtToFront = true;
            };

            // Directly invoke the callback (mouseDown delegates to this)
            overlay.bringSeparateWindowFront();
            expect(midiWindowBroughtToFront == true);
        }

        // ---- Full lifecycle: create, configure, show, interact, close ----

        beginTest("Integration - Full overlay lifecycle");
        {
            std::unique_ptr<OverlayComponent> overlayWindow = nullptr;
            bool overlayShouldBeVisible = false;
            bool settingsOpened = false;
            bool windowFrontCalled = false;

            // Step 1: Create overlay (simulating first ESC press)
            overlayWindow = std::make_unique<OverlayComponent>();
            overlayWindow->setBounds(0, 0, 1920, 1080);
            overlayShouldBeVisible = true;

            // Step 2: Wire up all callbacks (like MainComponent does)
            overlayWindow->onRequestClose = [&overlayWindow, &overlayShouldBeVisible]()
            {
                if (overlayWindow)
                {
                    overlayShouldBeVisible = false;
                    overlayWindow.reset();
                }
            };

            overlayWindow->setWindowFlag = [&overlayShouldBeVisible]()
            {
                overlayShouldBeVisible = false;
            };

            overlayWindow->bringSeparateWindowFront = [&windowFrontCalled]()
            {
                windowFrontCalled = true;
            };

            overlayWindow->onSettingsClick = [&settingsOpened]()
            {
                settingsOpened = true;
            };

            overlayWindow->showOverlay();
            expect(overlayWindow->isVisible() == true);

            // Step 4: Click settings
            overlayWindow->getSettingsButton().onClick();
            expect(settingsOpened == true);

            // Step 5: invoke bringSeparateWindowFront
            overlayWindow->bringSeparateWindowFront();
            expect(windowFrontCalled == true);

            // Step 6: Close via ESC (onRequestClose)
            overlayWindow->onRequestClose();
            expect(overlayWindow == nullptr);
            expect(overlayShouldBeVisible == false);
        }

        // ---- globalFocusChanged simulation: hide/show on app deactivate/activate ----

        beginTest("Integration - overlay visibility controlled by app focus state");
        {
            std::unique_ptr<OverlayComponent> overlayWindow = std::make_unique<OverlayComponent>();
            bool overlayShouldBeVisible = true;

            overlayWindow->showOverlay();
            expect(overlayWindow->isVisible() == true);

            // Simulate app losing focus
            overlayWindow->hideOverlay();
            expect(overlayWindow->isVisible() == false);

            // Simulate app regaining focus — restore based on flag
            overlayWindow->showOverlay();
            expect(overlayWindow->isVisible() == true);

            // Now close overlay
            overlayShouldBeVisible = false;

            // Simulate app losing and regaining focus again
            overlayWindow->hideOverlay();
            overlayWindow->setVisible(overlayShouldBeVisible);
            expect(overlayWindow->isVisible() == false);
        }

        // ---- Multiple open/close cycles ----

        beginTest("Integration - multiple create/destroy cycles do not leak");
        {
            for (int i = 0; i < 10; ++i)
            {
                std::unique_ptr<OverlayComponent> overlayWindow = std::make_unique<OverlayComponent>();
                bool overlayShouldBeVisible = true;

                overlayWindow->onRequestClose = [&overlayWindow, &overlayShouldBeVisible]()
                {
                    overlayShouldBeVisible = false;
                    overlayWindow.reset();
                };

                overlayWindow->showOverlay();
                expect(overlayWindow != nullptr);
                expect(overlayWindow->isVisible() == true);

                overlayWindow->onRequestClose();
                expect(overlayWindow == nullptr);
                expect(overlayShouldBeVisible == false);
            }
            expect(true); // survived 10 cycles
        }

        // ---- Settings button idempotency ----

        beginTest("Integration - clicking settings multiple times calls callback each time");
        {
            OverlayComponent overlay;
            int callCount = 0;

            overlay.onSettingsClick = [&callCount]() { callCount++; };

            overlay.getSettingsButton().onClick();
            overlay.getSettingsButton().onClick();
            overlay.getSettingsButton().onClick();

            expect(callCount == 3);
        }

        // ---- Resized after showOverlay sets valid button bounds ----

        beginTest("Integration - show then resize provides usable button layout");
        {
            OverlayComponent overlay;
            overlay.setBounds(0, 0, 1920, 1080);
            overlay.setVisible(true);
            overlay.resized();

            auto& settings = overlay.getSettingsButton();
            auto& exit = overlay.getExitButton();

            // Buttons should have been laid out
            expect(settings.getWidth() > 0);
            expect(settings.getHeight() > 0);
            expect(exit.getWidth() > 0);
            expect(exit.getHeight() > 0);

            // Settings button should be above exit button
            expect(settings.getY() < exit.getY());
        }

        // ---- ESC keyPressed dispatches correctly through onRequestClose ----

        beginTest("Integration - ESC on overlay triggers keyPressed -> onRequestClose -> destroys overlay");
        {
            std::unique_ptr<OverlayComponent> overlayWindow = std::make_unique<OverlayComponent>();
            bool overlayShouldBeVisible = true;

            overlayWindow->onRequestClose = [&overlayWindow, &overlayShouldBeVisible]()
            {
                overlayShouldBeVisible = false;
                overlayWindow.reset();
            };

            overlayWindow->setVisible(true);

            // Simulate ESC key via keyPressed
            bool handled = overlayWindow->keyPressed(juce::KeyPress(juce::KeyPress::escapeKey));
            expect(handled == true);
            expect(overlayWindow == nullptr);
            expect(overlayShouldBeVisible == false);
        }
    }
};

static OverlayComponentIntegrationTest overlayComponentIntegrationTest;
