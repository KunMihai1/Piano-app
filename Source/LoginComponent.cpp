/*
  ==============================================================================

    LoginComponent.cpp
    Created: 25 Feb 2026 9:30:59pm
    Author:  Kisuke

  ==============================================================================
*/

#include "LoginComponent.h"
#include "AppColours.h"

using namespace AppColours;


LoginComponent::LoginComponent(std::shared_ptr<SupabaseClient> newClient): client{newClient}
{
    auto setupLabel = [](juce::Label& lbl, const juce::String& text) {
        lbl.setText(text, juce::dontSendNotification);
        lbl.setColour(juce::Label::textColourId, subtitleText);
        lbl.setFont(juce::FontOptions(16.0f, juce::Font::bold));
        lbl.setJustificationType(juce::Justification::centredRight);
    };

    auto setupTE = [](juce::TextEditor& te, const juce::String& emptyText) {
        te.setMultiLine(false);
        te.setColour(juce::TextEditor::backgroundColourId, background.withAlpha(0.6f));
        te.setColour(juce::TextEditor::textColourId, titleText);
        te.setColour(juce::TextEditor::outlineColourId, separator);
        te.setColour(juce::TextEditor::focusedOutlineColourId, accent1);
        te.setFont(juce::FontOptions(16.0f));
        te.setTextToShowWhenEmpty(emptyText, subtitleText);
    };

    emailLabel = std::make_unique<juce::Label>();
    setupLabel(*emailLabel, "Email:");
    addAndMakeVisible(emailLabel.get());

    emailTE = std::make_unique<juce::TextEditor>();
    setupTE(*emailTE, "Enter email...");
    addAndMakeVisible(emailTE.get());

    passwordLabel = std::make_unique<juce::Label>();
    setupLabel(*passwordLabel, "Password:");
    addAndMakeVisible(passwordLabel.get());

    passwordField = std::make_unique<PasswordField>();
    addAndMakeVisible(passwordField.get());

    UsernameLabel = std::make_unique<juce::Label>();
    setupLabel(*UsernameLabel, "Username:");
    addAndMakeVisible(UsernameLabel.get());
    UsernameLabel->setVisible(false);

    UsernameTE = std::make_unique<juce::TextEditor>();
    setupTE(*UsernameTE, "Enter username...");
    addAndMakeVisible(UsernameTE.get());
    UsernameTE->setVisible(false);

    auto styleButton = [](juce::TextButton* btn, juce::Colour bgColour) {
        btn->setColour(juce::TextButton::buttonColourId, bgColour);
        btn->setColour(juce::TextButton::textColourOnId, titleText);
        btn->setColour(juce::TextButton::textColourOffId, titleText);
        btn->setMouseCursor(juce::MouseCursor::PointingHandCursor);
    };

    loginTB = std::make_unique<juce::TextButton>("Login");
    loginTB->onClick = [this]() { handleLogin(); };
    styleButton(loginTB.get(), accent1); // Warm orange for main action
    addAndMakeVisible(loginTB.get());
    
    signupTB = std::make_unique<juce::TextButton>("Sign Up");
    signupTB->onClick = [this]() { handleSignup(); };
    styleButton(signupTB.get(), panelBg);
    addAndMakeVisible(signupTB.get());

    forgotPassTB = std::make_unique<juce::TextButton>("Forgot Pass?");
    forgotPassTB->onClick = [this]() { handleForgotPassword(); };
    styleButton(forgotPassTB.get(), panelBg);
    addAndMakeVisible(forgotPassTB.get());

    doneTB = std::make_unique<juce::TextButton>("Done");
    doneTB->onClick = [this]() { handleDoneSignup(); };
    styleButton(doneTB.get(), accent1);
    addAndMakeVisible(doneTB.get());
    doneTB->setVisible(false);

    backTB = std::make_unique<juce::TextButton>("Back");
    backTB->onClick = [this]() { handleBack(); };
    styleButton(backTB.get(), panelBg);
    addAndMakeVisible(backTB.get());
    backTB->setVisible(false);
}

void LoginComponent::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    
    // Glassmorphism card background
    g.setColour(cardBg.withAlpha(0.85f));
    g.fillRoundedRectangle(bounds, 15.0f);

    // Subtle border
    g.setColour(separator.withAlpha(0.5f));
    g.drawRoundedRectangle(bounds, 15.0f, 1.5f);

    // Header Title
    g.setFont(juce::FontOptions(32.0f, juce::Font::bold));
    g.setColour(titleText);
    
    // Offset the header down slightly so it doesn't overlap the Back button
    auto headerArea = bounds.removeFromTop(90).withTrimmedTop(25);
    g.drawText(UsernameLabel->isVisible() ? "Create Account" : "Welcome Back", 
               headerArea, juce::Justification::centred, false);
}


void LoginComponent::resized()
{
    auto bounds = getLocalBounds();
    auto area = bounds.reduced(30); 
    const int rowHeight = 35; 
    const int spacing = 20;

    if (backTB != nullptr)
        backTB->setBounds(bounds.getX() + 15, bounds.getY() + 15, 80, 30);

    area.removeFromTop(60); // Skip header

    auto layoutField = [&](juce::Label* label, juce::Component* field) {
        auto row = area.removeFromTop(rowHeight);
        label->setBounds(row.removeFromLeft(90).reduced(0, 0)); 
        row.removeFromLeft(10); // Spacing between label and input
        field->setBounds(row);
        area.removeFromTop(spacing);
    };

    if (UsernameLabel->isVisible()) {
        layoutField(UsernameLabel.get(), UsernameTE.get());
    } else {
        UsernameLabel->setBounds(0,0,0,0);
        UsernameTE->setBounds(0,0,0,0);
    }

    layoutField(emailLabel.get(), emailTE.get());
    layoutField(passwordLabel.get(), passwordField.get());

    area.removeFromTop(10); 

    auto buttonArea = area.removeFromTop(rowHeight + 5);
    int buttonSpacing = 12;
    
    if (loginTB->isVisible()) {
        int buttonWidth = (buttonArea.getWidth() - 2 * buttonSpacing) / 3;
        int x = buttonArea.getX();

        loginTB->setBounds(x, buttonArea.getY(), buttonWidth, buttonArea.getHeight());
        x += buttonWidth + buttonSpacing;

        signupTB->setBounds(x, buttonArea.getY(), buttonWidth, buttonArea.getHeight());
        x += buttonWidth + buttonSpacing;

        forgotPassTB->setBounds(x, buttonArea.getY(), buttonWidth, buttonArea.getHeight());
    }

    if (doneTB != nullptr && doneTB->isVisible())
    {
        int doneWidth = 140;
        int doneHeight = 45;
        int doneX = bounds.getCentreX() - doneWidth / 2;
        int doneY = bounds.getBottom() - 65; 
        doneTB->setBounds(doneX, doneY, doneWidth, doneHeight);
    }
}

LoginComponent::~LoginComponent()
{
}

void LoginComponent::handleLogin()
{
    if (onSuccessfullLogin)
        onSuccessfullLogin();
    return;

    auto email = emailTE->getText().trim();
    auto pass = passwordField->getTextEditor().getText().trim();

    if (email.isEmpty() || pass.isEmpty())
    {
        juce::AlertWindow::showMessageBoxAsync(
            juce::AlertWindow::WarningIcon,
            "Login",
            "Please enter both email and password."
        );
        return;
    }

    juce::Component::SafePointer<LoginComponent> safeThis(this);
    auto clientPtr = client;

    

    std::thread([email, pass, safeThis, clientPtr]()
        {

            auto response = clientPtr->login(email, pass);

           

            juce::MessageManager::callAsync([response, safeThis, clientPtr]()
                {
                    if (safeThis == nullptr)
                        return;

                    auto body = response.body;

                    auto parsed = juce::JSON::parse(body);

                    if (!parsed.isObject())
                    {
                        
                        juce::AlertWindow::showMessageBoxAsync(
                            juce::AlertWindow::WarningIcon,
                            "Login Failed",
                            "Network failure or invalid JSON"
                        );
                        return;
                    }

                    auto* obj = parsed.getDynamicObject();

                    
                    if (obj->hasProperty("error") || obj->hasProperty("error_description") || obj->hasProperty("msg"))
                    {
                        juce::String errorMessage;

                        if (obj->hasProperty("error_description"))
                            errorMessage = obj->getProperty("error_description").toString();
                        else if (obj->hasProperty("error"))
                            errorMessage = obj->getProperty("error").toString();
                        else if (obj->hasProperty("msg"))
                            errorMessage = obj->getProperty("msg").toString();

                        juce::AlertWindow::showMessageBoxAsync(
                            juce::AlertWindow::WarningIcon,
                            "Login Failed",
                            errorMessage
                        );
                        return;
                    }

                    
                    
                    
                    if (obj->hasProperty("confirmation_required") && obj->getProperty("confirmation_required").toString() == "true")
                    {
                        juce::AlertWindow::showMessageBoxAsync(
                            juce::AlertWindow::InfoIcon,
                            "Email Verification Required",
                            "Please verify your email before logging in. Check your inbox."
                        );
                        return;
                    }
                    

                    if (obj->hasProperty("user"))
                    {
                        auto userObj = obj->getProperty("user").getDynamicObject();
                        if (userObj != nullptr)
                        {
                            clientPtr->setUserId(userObj->getProperty("id").toString());
                        }
                    }

                    if (obj->hasProperty("access_token"))
                    {

                        clientPtr->setAccessToken(obj->getProperty("access_token").toString());

                        juce::AlertWindow::showMessageBoxAsync(
                            juce::AlertWindow::InfoIcon,
                            "Login Success",
                            "You are now logged in!"
                        );

                        

                        safeThis->emailTE->clear();
                        safeThis->passwordField->getTextEditor().clear();

                        if (safeThis->onSuccessfullLogin)
                            safeThis->onSuccessfullLogin();
                    }
                    else
                    {
                        
                        juce::AlertWindow::showMessageBoxAsync(
                            juce::AlertWindow::WarningIcon,
                            "Login Failed",
                            "Unknown error occurred:\n" + body
                        );
                    }
                });

        }).detach();
}


void LoginComponent::handleSignup()
{
    if (!UsernameLabel->isVisible())
    {
        toSignupVisibility();
    }
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

void LoginComponent::handleDoneSignup()
{
    auto email = emailTE->getText().trim();
    auto pass = passwordField->getTextEditor().getText().trim();
    auto username = UsernameTE->getText().trim();

    if (email.isEmpty() || pass.isEmpty() || username.isEmpty())
    {
        juce::AlertWindow::showMessageBoxAsync(
            juce::AlertWindow::WarningIcon,
            "Sign Up",
            "Please fill in all fields."
        );
        return;
    }

    juce::Component::SafePointer<LoginComponent> safeThis(this);
    auto clientPtr = client;

    std::thread([email, pass, username, safeThis, clientPtr]()
        {

            auto response = clientPtr->signup(email, pass, username);

            

            juce::MessageManager::callAsync([response, safeThis]()
                {
                    if (safeThis == nullptr)
                        return;

                    auto body = response.body;

                    auto parsed = juce::JSON::parse(body);

                    if (!parsed.isObject())
                    {
                        juce::AlertWindow::showMessageBoxAsync(
                            juce::AlertWindow::WarningIcon,
                            "Sign Up Failed",
                            "Invalid server response:\n" + body
                        );
                        return;
                    }

                    auto* obj = parsed.getDynamicObject();

                    
                    if (obj->hasProperty("error") || obj->hasProperty("error_description"))
                    {
                        juce::String errorMessage = "Signup failed.";
                        if (obj->hasProperty("error_description"))
                            errorMessage = obj->getProperty("error_description").toString();
                        else if (obj->hasProperty("error"))
                            errorMessage = obj->getProperty("error").toString();
                        juce::AlertWindow::showMessageBoxAsync(
                            juce::AlertWindow::WarningIcon,
                            "Sign Up Failed",
                            errorMessage
                        );
                        return;
                    }

                    
                    bool isSession = obj->hasProperty("access_token");
                    bool isUser = obj->hasProperty("id");
                    if (!isSession && !isUser)
                    {
                        juce::AlertWindow::showMessageBoxAsync(
                            juce::AlertWindow::WarningIcon,
                            "Sign Up Failed",
                            "User was not created.\nServer response:\n" + body
                        );
                        return;
                    }

                    
                    if (isUser && obj->hasProperty("confirmation_sent_at"))
                    {
                        juce::AlertWindow::showMessageBoxAsync(
                            juce::AlertWindow::InfoIcon,
                            "Sign Up Success",
                            "Account created successfully! Please check your email to verify."
                        );
                    }
                    else
                    {
                        juce::AlertWindow::showMessageBoxAsync(
                            juce::AlertWindow::InfoIcon,
                            "Sign Up Success",
                            "Account created successfully!"
                        );
                    }

                    
                    safeThis->UsernameLabel->setVisible(false);
                    safeThis->UsernameTE->setVisible(false);
                    safeThis->backTB->setVisible(false);
                    safeThis->doneTB->setVisible(false);
                    safeThis->loginTB->setVisible(true);
                    safeThis->forgotPassTB->setVisible(true);
                    safeThis->signupTB->setVisible(true);

                    safeThis->emailTE->clear();
                    safeThis->passwordField->getTextEditor().clear();
                    safeThis->UsernameTE->clear();
                });

        }).detach();
}

void LoginComponent::handleBack()
{
    toLoginVisbility();

}

void LoginComponent::toLoginVisbility()
{
    UsernameTE->clear();
    emailTE->clear();
    passwordField->getTextEditor().clear();

    UsernameLabel->setVisible(false);
    UsernameTE->setVisible(false);
    doneTB->setVisible(false);
    backTB->setVisible(false);
    forgotPassTB->setVisible(true);
    loginTB->setVisible(true);
    signupTB->setVisible(true);
    
    resized();
    repaint();
}

void LoginComponent::toSignupVisibility()
{
    UsernameTE->clear();
    emailTE->clear();
    passwordField->getTextEditor().clear();

    UsernameLabel->setVisible(true);
    UsernameTE->setVisible(true);
    doneTB->setVisible(true);
    backTB->setVisible(true);
    forgotPassTB->setVisible(false);
    loginTB->setVisible(false);
    signupTB->setVisible(false);
    
    resized();
    repaint();
}

PasswordField::PasswordField()
{
    passwordTE.setMultiLine(false);
    passwordTE.setPasswordCharacter((juce::juce_wchar)0x2022);
    passwordTE.setCaretVisible(true);
    passwordTE.setTextToShowWhenEmpty("Enter password...", subtitleText);
    
    passwordTE.setColour(juce::TextEditor::backgroundColourId, background.withAlpha(0.6f));
    passwordTE.setColour(juce::TextEditor::textColourId, titleText);
    passwordTE.setColour(juce::TextEditor::outlineColourId, separator);
    passwordTE.setColour(juce::TextEditor::focusedOutlineColourId, accent1);
    passwordTE.setFont(juce::FontOptions(16.0f));
    addAndMakeVisible(passwordTE);

    
    eyeButton = std::make_unique<juce::DrawableButton>("eye", juce::DrawableButton::ImageFitted);

    eyeVisibleDrawable = juce::Drawable::createFromImageData(BinaryData::PasswordVisible_png, BinaryData::PasswordVisible_pngSize);
    eyeInvisibleDrawable = juce::Drawable::createFromImageData(BinaryData::PasswordInvisible_png, BinaryData::PasswordInvisible_pngSize);

    eyeButton->setImages(eyeInvisibleDrawable.get());

    eyeButton->onClick = [this]()
        {
            visible = !visible;
            passwordTE.setPasswordCharacter(visible ? 0 : (juce::juce_wchar)0x2022); 
            eyeButton->setImages(visible ? eyeVisibleDrawable.get() : eyeInvisibleDrawable.get());
        };

    eyeButton->setMouseCursor(juce::MouseCursor::PointingHandCursor);

    addAndMakeVisible(eyeButton.get());
}

void PasswordField::resized()
{
    auto area = getLocalBounds();
    int buttonSize = area.getHeight();

    passwordTE.setBounds(area.removeFromLeft(area.getWidth() - buttonSize - 20));
    eyeButton->setBounds(area.reduced(2));
}

juce::TextEditor& PasswordField::getTextEditor()
{
    return passwordTE;
}
