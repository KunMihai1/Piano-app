/*
  ==============================================================================

    LoginComponent.cpp
    Created: 25 Feb 2026 9:30:59pm
    Author:  Kisuke

  ==============================================================================
*/

#include "LoginComponent.h"

LoginComponent::LoginComponent()
{
    emailLabel = std::make_unique<juce::Label>();
    emailLabel->setText("Email:", juce::dontSendNotification);
    addAndMakeVisible(emailLabel.get());

    emailTE = std::make_unique<juce::TextEditor>();
    emailTE->setMultiLine(false);
    addAndMakeVisible(emailTE.get());



    passwordLabel = std::make_unique<juce::Label>();
    passwordLabel->setText("Password:", juce::dontSendNotification);
    addAndMakeVisible(passwordLabel.get());

    passwordTE = std::make_unique<juce::TextEditor>();
    passwordTE->setMultiLine(false);
    passwordTE->setPasswordCharacter('*');
    addAndMakeVisible(passwordTE.get());


    otpLabel = std::make_unique<juce::Label>();
    otpLabel->setText("Verification Code:", juce::dontSendNotification);
    
    addAndMakeVisible(otpLabel.get());
    otpLabel->setVisible(false);

    otpTE = std::make_unique<juce::TextEditor>();
    otpTE->setMultiLine(false);
    otpTE->setInputRestrictions(6, "0123456789");
    
    addAndMakeVisible(otpTE.get());
    otpTE->setVisible(false);


    loginTB = std::make_unique<juce::TextButton>("Login");
    loginTB->onClick = [this]() { handleLogin(); };
    loginTB->setMouseCursor(juce::MouseCursor::PointingHandCursor);
    addAndMakeVisible(loginTB.get());
    

    signupTB = std::make_unique<juce::TextButton>("Sign Up");
    signupTB->onClick = [this]() { handleSignup(); };
    signupTB->setMouseCursor(juce::MouseCursor::PointingHandCursor);
    addAndMakeVisible(signupTB.get());



    verifyTB = std::make_unique<juce::TextButton>("Verify Code");
    verifyTB->onClick = [this]() { handleOTP(); };
    verifyTB->setMouseCursor(juce::MouseCursor::PointingHandCursor);
    addAndMakeVisible(verifyTB.get());
    verifyTB->setVisible(false);

    forgotPassTB = std::make_unique<juce::TextButton>("Forgot Password?");
    forgotPassTB->onClick = [this]() { handleForgotPassword(); };
    forgotPassTB->setMouseCursor(juce::MouseCursor::PointingHandCursor);
    addAndMakeVisible(forgotPassTB.get());

}

void LoginComponent::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::darkgrey);
}

void LoginComponent::resized()
{
    auto area = getLocalBounds().reduced(20);
    auto rowHeight = 30;

    // Labels + text editors (stacked)
    emailLabel->setBounds(area.removeFromTop(rowHeight).removeFromLeft(120));
    emailTE->setBounds(area.removeFromTop(rowHeight));

    passwordLabel->setBounds(area.removeFromTop(rowHeight).removeFromLeft(120));
    passwordTE->setBounds(area.removeFromTop(rowHeight));

    otpLabel->setBounds(area.removeFromTop(rowHeight).removeFromLeft(150));
    otpTE->setBounds(area.removeFromTop(rowHeight));

    // Buttons - same row
    auto buttonArea = area.removeFromTop(rowHeight);
    int buttonSpacing = 10;
    int buttonWidth = (buttonArea.getWidth() - 3 * buttonSpacing) / 4; // 4 buttons
    int x = buttonArea.getX();

    loginTB->setBounds(x, buttonArea.getY()+10, buttonWidth, rowHeight);
    x += buttonWidth + buttonSpacing;

    signupTB->setBounds(x, buttonArea.getY()+10, buttonWidth, rowHeight);
    x += buttonWidth + buttonSpacing;

    verifyTB->setBounds(x, buttonArea.getY()+10, buttonWidth, rowHeight);
    x += buttonWidth + buttonSpacing;
    

    forgotPassTB->setBounds(x, buttonArea.getY()+10, buttonWidth, rowHeight);
}

LoginComponent::~LoginComponent()
{
}

void LoginComponent::handleLogin()
{
    auto email = emailTE->getText();
    auto pass = passwordTE->getText();
}

void LoginComponent::handleSignup()
{
    auto email = emailTE->getText();
    auto pass = passwordTE->getText();
}

void LoginComponent::handleOTP()
{
    auto code = otpTE->getText();
}

void LoginComponent::handleForgotPassword()
{
    auto email = emailTE->getText();

    if (email.isEmpty())
    {
        juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::WarningIcon,
            "Forgot Password",
            "Please enter your email first.");
        return;
    }


    email = emailTE->getText();

    if (email.isEmpty())
    {
        juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::WarningIcon,
            "Forgot Password",
            "Please enter your email first.");
        return;
    }
}
