/*
  ==============================================================================

    LoginComponent.h
    Created: 25 Feb 2026 9:30:59pm
    Author:  Kisuke

  ==============================================================================
*/

#pragma once

#include "JuceHeader.h"

class PasswordTextEditor : public juce::TextEditor
{
public:
    PasswordTextEditor();

    void resized() override;

private:
    bool visible = false;
    juce::Rectangle<int> iconArea;
    std::unique_ptr<juce::DrawableButton> eyeButton;
    std::unique_ptr<juce::Drawable> eyeVisibleDrawable;
    std::unique_ptr<juce::Drawable> eyeInvisibleDrawable;
};

class SupabaseClient
{
public:
    SupabaseClient();

    SupabaseClient(const juce::String& projectUrl, const juce::String& anonKey);

    juce::String login(const juce::String& email, const juce::String& password);

    juce::String signup(const juce::String& email, const juce::String& password, const juce::String& username);

private:
    juce::String baseUrl;
    juce::String apiKey;
};

class LoginComponent : public juce::Component
{
public:
    
    std::function<void()> onSuccessfullLogin;

    LoginComponent();

    void paint(juce::Graphics& g) override;


    void resized() override;

    ~LoginComponent();

private:

    void handleLogin();
    void handleSignup();
    void handleForgotPassword();
    void handleDoneSignup();
    void handleBack();

    void toLoginVisbility();
    void toSignupVisibility();

    std::unique_ptr<juce::Label> emailLabel, passwordLabel, UsernameLabel;
    std::unique_ptr<juce::TextEditor> emailTE, UsernameTE;
    std::unique_ptr<PasswordTextEditor> passwordTE;
    std::unique_ptr<juce::TextButton> loginTB, signupTB, forgotPassTB,doneTB,backTB;
};