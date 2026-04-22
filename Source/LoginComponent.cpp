/*
  ==============================================================================

    LoginComponent.cpp
    Created: 25 Feb 2026 9:30:59pm
    Author:  Kisuke

  ==============================================================================
*/

#include "LoginComponent.h"


LoginComponent::LoginComponent(std::shared_ptr<SupabaseClient> newClient): client{newClient}
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

    passwordField = std::make_unique<PasswordField>();
    addAndMakeVisible(passwordField.get());


    UsernameLabel = std::make_unique<juce::Label>();
    UsernameLabel->setText("Username", juce::dontSendNotification);
    
    addAndMakeVisible(UsernameLabel.get());

    UsernameLabel->setVisible(false);

    UsernameTE = std::make_unique<juce::TextEditor>();
    UsernameTE->setMultiLine(false);
    
    addAndMakeVisible(UsernameTE.get());

    UsernameTE->setVisible(false);


    loginTB = std::make_unique<juce::TextButton>("Login");
    loginTB->onClick = [this]() { handleLogin(); };
    loginTB->setMouseCursor(juce::MouseCursor::PointingHandCursor);
    addAndMakeVisible(loginTB.get());
    

    signupTB = std::make_unique<juce::TextButton>("Sign Up");
    signupTB->onClick = [this]() { handleSignup(); };
    signupTB->setMouseCursor(juce::MouseCursor::PointingHandCursor);
    addAndMakeVisible(signupTB.get());


    forgotPassTB = std::make_unique<juce::TextButton>("Forgot Password?");
    forgotPassTB->onClick = [this]() { handleForgotPassword(); };
    forgotPassTB->setMouseCursor(juce::MouseCursor::PointingHandCursor);
    addAndMakeVisible(forgotPassTB.get());

    doneTB = std::make_unique<juce::TextButton>("Done");
    doneTB->onClick = [this]()
    {
        handleDoneSignup();
    };

    doneTB->setMouseCursor(juce::MouseCursor::PointingHandCursor);
    addAndMakeVisible(doneTB.get());

    doneTB->setVisible(false);

    backTB = std::make_unique<juce::TextButton>("Back");
    backTB->onClick = [this]()
    {
        handleBack();
     };

    addAndMakeVisible(backTB.get());
    backTB->setVisible(false);


}

void LoginComponent::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::grey);

    g.setColour(juce::Colours::black);

    int borderThickness = 3;

    g.drawRect(getLocalBounds(), borderThickness);
}


void LoginComponent::resized()
{
    auto bounds = getLocalBounds();
    auto area = bounds.reduced(20);
    const int rowHeight = 30;

    
    if (backTB != nullptr)
        backTB->setBounds(bounds.getX() + 10,
            bounds.getY() + 10,
            80,
            30);

    
    area.removeFromTop(50);

    
    emailLabel->setBounds(area.removeFromTop(rowHeight).removeFromLeft(120));
    emailTE->setBounds(area.removeFromTop(rowHeight));

    passwordLabel->setBounds(area.removeFromTop(rowHeight).removeFromLeft(120));
    passwordField->setBounds(area.removeFromTop(rowHeight));

    UsernameLabel->setBounds(area.removeFromTop(rowHeight).removeFromLeft(150));
    UsernameTE->setBounds(area.removeFromTop(rowHeight));

    area.removeFromTop(20); 

    
    auto buttonArea = area.removeFromTop(rowHeight);
    int buttonSpacing = 10;
    int buttonWidth = (buttonArea.getWidth() - 2 * buttonSpacing) / 3;
    int x = buttonArea.getX();

    loginTB->setBounds(x, buttonArea.getY(), buttonWidth, rowHeight);
    x += buttonWidth + buttonSpacing;

    signupTB->setBounds(x, buttonArea.getY(), buttonWidth, rowHeight);
    x += buttonWidth + buttonSpacing;

    forgotPassTB->setBounds(x, buttonArea.getY(), buttonWidth, rowHeight);

    
    if (doneTB != nullptr)
    {
        int doneWidth = 120;
        int doneHeight = 35;

        int doneX = bounds.getCentreX() - doneWidth / 2;
        int doneY = bounds.getBottom() - 70; 

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

            if (safeThis == nullptr)
                return;

            auto body = response.body;

            auto parsed = juce::JSON::parse(body);

            juce::MessageManager::callAsync([body,parsed,response, safeThis, clientPtr]()
                {
                    

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

            if (safeThis == nullptr)
                return;

            auto body = response.body;

            auto parsed = juce::JSON::parse(body);

            juce::MessageManager::callAsync([body,parsed,response, safeThis]()
                {
                    

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
}

PasswordField::PasswordField()
{
    passwordTE.setMultiLine(false);
    passwordTE.setPasswordCharacter('*');
    passwordTE.setCaretVisible(true);
    passwordTE.setTextToShowWhenEmpty("Enter password...", juce::Colours::grey);
    addAndMakeVisible(passwordTE);

    
    eyeButton = std::make_unique<juce::DrawableButton>("eye", juce::DrawableButton::ImageFitted);

    eyeVisibleDrawable = juce::Drawable::createFromImageData(BinaryData::PasswordVisible_png, BinaryData::PasswordVisible_pngSize);
    eyeInvisibleDrawable = juce::Drawable::createFromImageData(BinaryData::PasswordInvisible_png, BinaryData::PasswordInvisible_pngSize);

    eyeButton->setImages(eyeInvisibleDrawable.get());

    eyeButton->onClick = [this]()
        {
            visible = !visible;
            passwordTE.setPasswordCharacter(visible ? 0 : '*'); 
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
