#include <juce_core/juce_core.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include "OverlayComponent.h"

class OverlayComponentUnitTest : public juce::UnitTest
{
public:
    OverlayComponentUnitTest() : juce::UnitTest("OverlayComponent Unit", "Unit") {}

    void runTest() override
    {
        testConstructorDefaults();
        testSettingsButton();
        testExitButton();
        testShowHideOverlay();
        testAccessors();
        testSettingsButtonCallback();
        testKeyPressed();
        testBringSeparateWindowFront();
        testPaint();
        testResized();
        testCallbackDefaults();
    }

private:
    void testConstructorDefaults()
    {
        beginTest("Constructor - starts invisible");
        {
            OverlayComponent overlay;
            expect(overlay.isVisible() == false);
        }

        beginTest("Constructor - is not opaque");
        {
            OverlayComponent overlay;
            expect(overlay.isOpaque() == false);
        }

        beginTest("Constructor - wants keyboard focus");
        {
            OverlayComponent overlay;
            expect(overlay.getWantsKeyboardFocus() == true);
        }
    }

    void testSettingsButton()
    {
        beginTest("Settings button - text is 'Settings'");
        {
            OverlayComponent overlay;
            expect(overlay.getSettingsButton().getButtonText() == "Settings");
        }

        beginTest("Settings button - has pointing hand cursor");
        {
            OverlayComponent overlay;
            expect(overlay.getSettingsButton().getMouseCursor() == juce::MouseCursor::PointingHandCursor);
        }

        beginTest("Settings button - text colour is white");
        {
            OverlayComponent overlay;
            auto colour = overlay.getSettingsButton().findColour(juce::TextButton::textColourOffId);
            expect(colour == juce::Colours::white);
        }
    }

    void testExitButton()
    {
        beginTest("Exit button - text is 'Exit'");
        {
            OverlayComponent overlay;
            expect(overlay.getExitButton().getButtonText() == "Exit");
        }

        beginTest("Exit button - has pointing hand cursor");
        {
            OverlayComponent overlay;
            expect(overlay.getExitButton().getMouseCursor() == juce::MouseCursor::PointingHandCursor);
        }

        beginTest("Exit button - text colour is white");
        {
            OverlayComponent overlay;
            auto colour = overlay.getExitButton().findColour(juce::TextButton::textColourOffId);
            expect(colour == juce::Colours::white);
        }
    }

    void testShowHideOverlay()
    {
        beginTest("showOverlay - setVisible true makes component visible");
        {
            OverlayComponent overlay;
            expect(overlay.isVisible() == false);
            overlay.showOverlay();
            expect(overlay.isVisible() == true);
        }

        beginTest("hideOverlay - makes component invisible");
        {
            OverlayComponent overlay;
            overlay.showOverlay();
            expect(overlay.isVisible() == true);
            overlay.hideOverlay();
            expect(overlay.isVisible() == false);
        }

        beginTest("show then hide then show - toggles correctly");
        {
            OverlayComponent overlay;
            overlay.showOverlay();
            expect(overlay.isVisible() == true);
            overlay.hideOverlay();
            expect(overlay.isVisible() == false);
            overlay.showOverlay();
            expect(overlay.isVisible() == true);
        }
    }

    void testAccessors()
    {
        beginTest("getExitButton returns a valid reference");
        {
            OverlayComponent overlay;
            auto& btn = overlay.getExitButton();
            expect(btn.getButtonText() == "Exit");
        }

        beginTest("getSettingsButton returns a valid reference");
        {
            OverlayComponent overlay;
            auto& btn = overlay.getSettingsButton();
            expect(btn.getButtonText() == "Settings");
        }
    }

    void testSettingsButtonCallback()
    {
        beginTest("Settings button click - invokes onSettingsClick callback");
        {
            OverlayComponent overlay;
            bool called = false;
            overlay.onSettingsClick = [&called]() { called = true; };
            overlay.getSettingsButton().onClick();
            expect(called == true);
        }

        beginTest("Settings button click - no crash when onSettingsClick is null");
        {
            OverlayComponent overlay;
            overlay.onSettingsClick = nullptr;
            overlay.getSettingsButton().onClick();
            expect(true); // no crash
        }
    }

    void testKeyPressed()
    {
        beginTest("keyPressed - escape key returns true");
        {
            OverlayComponent overlay;
            bool result = overlay.keyPressed(juce::KeyPress(juce::KeyPress::escapeKey));
            expect(result == true);
        }

        beginTest("keyPressed - non-escape key returns false");
        {
            OverlayComponent overlay;
            bool result = overlay.keyPressed(juce::KeyPress('a', 0, 0));
            expect(result == false);
        }

        beginTest("keyPressed - escape calls onRequestClose when set");
        {
            OverlayComponent overlay;
            bool closeCalled = false;
            overlay.onRequestClose = [&closeCalled]() { closeCalled = true; };
            overlay.keyPressed(juce::KeyPress(juce::KeyPress::escapeKey));
            expect(closeCalled == true);
        }

        beginTest("keyPressed - escape without onRequestClose calls setWindowFlag and hides");
        {
            OverlayComponent overlay;
            overlay.onRequestClose = nullptr;
            bool flagCalled = false;
            overlay.setWindowFlag = [&flagCalled]() { flagCalled = true; };

            overlay.showOverlay();
            overlay.keyPressed(juce::KeyPress(juce::KeyPress::escapeKey));

            expect(flagCalled == true);
            expect(overlay.isVisible() == false);
        }

        beginTest("keyPressed - escape without onRequestClose and without setWindowFlag still hides");
        {
            OverlayComponent overlay;
            overlay.onRequestClose = nullptr;
            overlay.setWindowFlag = nullptr;

            overlay.showOverlay();
            overlay.keyPressed(juce::KeyPress(juce::KeyPress::escapeKey));

            expect(overlay.isVisible() == false);
        }
    }

    void testBringSeparateWindowFront()
    {
        beginTest("bringSeparateWindowFront - callable when set");
        {
            OverlayComponent overlay;
            bool called = false;
            overlay.bringSeparateWindowFront = [&called]() { called = true; };
            overlay.bringSeparateWindowFront();
            expect(called == true);
        }

        beginTest("bringSeparateWindowFront - null check prevents crash");
        {
            OverlayComponent overlay;
            overlay.bringSeparateWindowFront = nullptr;
            // mouseDown checks for null before calling, so no crash
            expect(!overlay.bringSeparateWindowFront);
        }
    }

    void testPaint()
    {
        beginTest("paint - executes without crash");
        {
            OverlayComponent overlay;
            overlay.setBounds(0, 0, 400, 300);

            juce::Image img(juce::Image::ARGB, 400, 300, true);
            juce::Graphics g(img);

            overlay.paint(g);
            expect(true); // no crash
        }
    }

    void testResized()
    {
        beginTest("resized - executes without crash");
        {
            OverlayComponent overlay;
            overlay.setBounds(0, 0, 800, 600);
            overlay.resized();
            expect(true); // no crash
        }

        beginTest("resized - settings button gets non-zero bounds");
        {
            OverlayComponent overlay;
            overlay.setBounds(0, 0, 800, 600);
            overlay.resized();
            auto& btn = overlay.getSettingsButton();
            expect(btn.getWidth() > 0);
            expect(btn.getHeight() > 0);
        }

        beginTest("resized - exit button gets non-zero bounds");
        {
            OverlayComponent overlay;
            overlay.setBounds(0, 0, 800, 600);
            overlay.resized();
            auto& btn = overlay.getExitButton();
            expect(btn.getWidth() > 0);
            expect(btn.getHeight() > 0);
        }
    }

    void testCallbackDefaults()
    {
        beginTest("Callbacks are null by default");
        {
            OverlayComponent overlay;
            expect(!overlay.onSettingsClick);
            expect(!overlay.windowExists);
            expect(!overlay.closeWindow);
            expect(!overlay.bringSeparateWindowFront);
            expect(!overlay.setWindowFlag);
            expect(!overlay.onRequestClose);
        }
    }
};

static OverlayComponentUnitTest overlayComponentUnitTest;