#include <juce_core/juce_core.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include "OverlayComponent.h"

class OverlayComponentIntegrationTest : public juce::UnitTest
{
public:
    OverlayComponentIntegrationTest()
        : juce::UnitTest("OverlayComponent Integration", "Integration") {}

    void runTest() override
    {
        testEscOpenCloseCycle();
        testSettingsButtonCreatesMidiWindow();
        testSetWindowFlagWithoutOnRequestClose();
        testBringSeparateWindowFront();
        testFullLifecycle();
        testFocusVisibilityControl();
        testMultipleCreateDestroyCycles();
        testSettingsButtonIdempotency();
        testResizeLayout();
        testEscKeyDispatch();
    }

private:
    void testEscOpenCloseCycle()
    {
        beginTest("Integration - ESC open/close cycle simulates MainComponent behavior");

        std::unique_ptr<OverlayComponent> overlayWindow = nullptr;
        bool overlayShouldBeVisible = false;

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

        if (overlayWindow && overlayWindow->onRequestClose)
            overlayWindow->onRequestClose();

        expect(overlayWindow == nullptr);
        expect(overlayShouldBeVisible == false);
    }

    void testSettingsButtonCreatesMidiWindow()
    {
        beginTest("Integration - Settings button triggers MIDI window creation");

        OverlayComponent overlay;
        bool midiWindowCreated = false;
        bool midiWindowShouldBeVisible = false;

        overlay.onSettingsClick = [&midiWindowCreated, &midiWindowShouldBeVisible]()
        {
            midiWindowShouldBeVisible = true;
            midiWindowCreated = true;
        };

        overlay.getSettingsButton().onClick();

        expect(midiWindowCreated == true);
        expect(midiWindowShouldBeVisible == true);
    }

    void testSetWindowFlagWithoutOnRequestClose()
    {
        beginTest("Integration - setWindowFlag updates external state on ESC without onRequestClose");

        OverlayComponent overlay;
        bool overlayShouldBeVisible = true;

        overlay.onRequestClose = nullptr;
        overlay.setWindowFlag = [&overlayShouldBeVisible]()
        {
            overlayShouldBeVisible = false;
        };

        overlay.showOverlay();
        expect(overlay.isVisible() == true);

        overlay.keyPressed(juce::KeyPress(juce::KeyPress::escapeKey));

        expect(overlayShouldBeVisible == false);
        expect(overlay.isVisible() == false);
    }

    void testBringSeparateWindowFront()
    {
        beginTest("Integration - bringSeparateWindowFront callback brings MIDI window to front");

        OverlayComponent overlay;
        bool midiWindowBroughtToFront = false;

        overlay.bringSeparateWindowFront = [&midiWindowBroughtToFront]()
        {
            midiWindowBroughtToFront = true;
        };

        overlay.bringSeparateWindowFront();
        expect(midiWindowBroughtToFront == true);
    }

    void testFullLifecycle()
    {
        beginTest("Integration - Full overlay lifecycle");

        std::unique_ptr<OverlayComponent> overlayWindow = nullptr;
        bool overlayShouldBeVisible = false;
        bool settingsOpened = false;
        bool windowFrontCalled = false;

        overlayWindow = std::make_unique<OverlayComponent>();
        overlayWindow->setBounds(0, 0, 1920, 1080);
        overlayShouldBeVisible = true;

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

        overlayWindow->getSettingsButton().onClick();
        expect(settingsOpened == true);

        overlayWindow->bringSeparateWindowFront();
        expect(windowFrontCalled == true);

        overlayWindow->onRequestClose();
        expect(overlayWindow == nullptr);
        expect(overlayShouldBeVisible == false);
    }

    void testFocusVisibilityControl()
    {
        beginTest("Integration - overlay visibility controlled by app focus state");

        std::unique_ptr<OverlayComponent> overlayWindow = std::make_unique<OverlayComponent>();
        bool overlayShouldBeVisible = true;

        overlayWindow->showOverlay();
        expect(overlayWindow->isVisible() == true);

        overlayWindow->hideOverlay();
        expect(overlayWindow->isVisible() == false);

        overlayWindow->showOverlay();
        expect(overlayWindow->isVisible() == true);

        overlayShouldBeVisible = false;

        overlayWindow->hideOverlay();
        overlayWindow->setVisible(overlayShouldBeVisible);
        expect(overlayWindow->isVisible() == false);
    }

    void testMultipleCreateDestroyCycles()
    {
        beginTest("Integration - multiple create/destroy cycles do not leak");

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
        expect(true);
    }

    void testSettingsButtonIdempotency()
    {
        beginTest("Integration - clicking settings multiple times calls callback each time");

        OverlayComponent overlay;
        int callCount = 0;

        overlay.onSettingsClick = [&callCount]() { callCount++; };

        overlay.getSettingsButton().onClick();
        overlay.getSettingsButton().onClick();
        overlay.getSettingsButton().onClick();

        expect(callCount == 3);
    }

    void testResizeLayout()
    {
        beginTest("Integration - show then resize provides usable button layout");

        OverlayComponent overlay;
        overlay.setBounds(0, 0, 1920, 1080);
        overlay.setVisible(true);
        overlay.resized();

        auto& settings = overlay.getSettingsButton();
        auto& exit = overlay.getExitButton();

        expect(settings.getWidth() > 0);
        expect(settings.getHeight() > 0);
        expect(exit.getWidth() > 0);
        expect(exit.getHeight() > 0);

        expect(settings.getY() < exit.getY());
    }

    void testEscKeyDispatch()
    {
        beginTest("Integration - ESC on overlay triggers keyPressed -> onRequestClose -> destroys overlay");

        std::unique_ptr<OverlayComponent> overlayWindow = std::make_unique<OverlayComponent>();
        bool overlayShouldBeVisible = true;

        overlayWindow->onRequestClose = [&overlayWindow, &overlayShouldBeVisible]()
        {
            overlayShouldBeVisible = false;
            overlayWindow.reset();
        };

        overlayWindow->setVisible(true);

        bool handled = overlayWindow->keyPressed(juce::KeyPress(juce::KeyPress::escapeKey));
        expect(handled == true);
        expect(overlayWindow == nullptr);
        expect(overlayShouldBeVisible == false);
    }
};

static OverlayComponentIntegrationTest overlayComponentIntegrationTest;
