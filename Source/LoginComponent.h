/*
  ==============================================================================

    LoginComponent.h
    Created: 25 Feb 2026 9:30:59pm
    Author:  Kisuke

  ==============================================================================
*/

#pragma once

#include "JuceHeader.h"

class LoginComponent : public juce::Component
{
public:
    LoginComponent();

    void paint(juce::Graphics& g) override;

    void resized() override;

    ~LoginComponent();

private:

    void handleLogin();
    void handleSignup();
    void handleOTP();
    void handleForgotPassword();

    std::unique_ptr<juce::Label> emailLabel, passwordLabel, otpLabel;
    std::unique_ptr<juce::TextEditor> emailTE, passwordTE, otpTE;
    std::unique_ptr<juce::TextButton> loginTB, signupTB, verifyTB, forgotPassTB;
};