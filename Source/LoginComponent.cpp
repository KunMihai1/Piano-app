/*
  ==============================================================================

    LoginComponent.cpp
    Created: 25 Feb 2026 9:30:59pm
    Author:  Kisuke

  ==============================================================================
*/

#include "LoginComponent.h"

SupabaseClient::SupabaseClient(const juce::String& projectUrl, const juce::String& anonKey)
    :baseUrl(projectUrl), apiKey(anonKey)
{

}

juce::String SupabaseClient::login(const juce::String& email,
    const juce::String& password)
{
    juce::String body = R"({"email":")" + email + R"(","password":")" + password + R"(","grant_type":"password"})";

    url = url.withPOSTData(body);

    juce::String extraHeaders;
    extraHeaders << "apikey: " << apiKey << "\r\n";
    extraHeaders << "Content-Type: application/json\r\n";

    auto stream = url.createInputStream(
        juce::URL::InputStreamOptions()
        .withConnectionTimeoutMs(5000)
        .withExtraHeaders(extraHeaders)
    );

    if (!stream)
        return "{\"error\":\"connection failed\"}";

    auto response = stream->readEntireStreamAsString();
    DBG("Login response: " + response);
    return response;
}
   


juce::String SupabaseClient::signup(const juce::String& email,
    const juce::String& password,
    const juce::String& username)
{
    
    juce::String body = R"({"email":")" + email +
        R"(","password":")" + password +
        R"(","data":{"name":")" + username + R"("}})";

    auto postUrl = juce::URL(baseUrl + "/auth/v1/signup")
        .withPOSTData(body);

    juce::String extraHeaders;
    extraHeaders << "apikey: " << apiKey << "\r\n";
    extraHeaders << "Content-Type: application/json\r\n";

    auto stream = postUrl.createInputStream(
        juce::URL::InputStreamOptions(juce::URL::ParameterHandling::inPostData)
        .withConnectionTimeoutMs(5000)
        .withExtraHeaders(extraHeaders)
    );

    if (!stream)
        return "{\"error\":\"connection failed\"}";

    return stream->readEntireStreamAsString();
}

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
    g.fillAll(juce::Colours::darkgrey);
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
    passwordTE->setBounds(area.removeFromTop(rowHeight));

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
    auto email = emailTE->getText().trim();
    auto pass = passwordTE->getText().trim();

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

    std::thread([email, pass, safeThis]()
        {
            

            auto response = client.login(email, pass);

            // Log full server response for debugging
            DBG("Supabase login response: " + response);

            juce::MessageManager::callAsync([response, safeThis]()
                {
                    if (safeThis == nullptr)
                        return;

                    auto parsed = juce::JSON::parse(response);

                    if (!parsed.isObject())
                    {
                        // Network failure or invalid JSON
                        juce::AlertWindow::showMessageBoxAsync(
                            juce::AlertWindow::WarningIcon,
                            "Login Failed",
                            "Server returned invalid response:\n" + response
                        );
                        return;
                    }

                    auto* obj = parsed.getDynamicObject();

                    // Handle Supabase error messages
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

                    /*
                    // Check if email is unverified
                    if (obj->hasProperty("confirmation_required") && obj->getProperty("confirmation_required").toString() == "true")
                    {
                        juce::AlertWindow::showMessageBoxAsync(
                            juce::AlertWindow::InfoIcon,
                            "Email Verification Required",
                            "Please verify your email before logging in. Check your inbox."
                        );
                        return;
                    }
                    */

                    // Success
                    if (obj->hasProperty("access_token"))
                    {
                        juce::AlertWindow::showMessageBoxAsync(
                            juce::AlertWindow::InfoIcon,
                            "Login Success",
                            "You are now logged in!"
                        );

                        safeThis->emailTE->clear();
                        safeThis->passwordTE->clear();

                        if (safeThis->onSuccessfullLogin)
                            safeThis->onSuccessfullLogin();
                    }
                    else
                    {
                        // Fallback for unexpected issues
                        juce::AlertWindow::showMessageBoxAsync(
                            juce::AlertWindow::WarningIcon,
                            "Login Failed",
                            "Unknown error occurred:\n" + response
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
    auto pass = passwordTE->getText().trim();
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

    std::thread([email, pass, username, safeThis]()
        {
            

            auto response = client.signup(email, pass, username);

            
            DBG("Supabase signup response: " + response);

            juce::MessageManager::callAsync([response, safeThis]()
                {
                    if (safeThis == nullptr)
                        return;

                    auto parsed = juce::JSON::parse(response);

                    if (!parsed.isObject())
                    {
                        juce::AlertWindow::showMessageBoxAsync(
                            juce::AlertWindow::WarningIcon,
                            "Sign Up Failed",
                            "Invalid server response:\n" + response
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

                    
                    if (!obj->hasProperty("user"))
                    {
                        juce::AlertWindow::showMessageBoxAsync(
                            juce::AlertWindow::WarningIcon,
                            "Sign Up Failed",
                            "User was not created.\nServer response:\n" + response
                        );
                        return;
                    }

                    
                    auto userObj = obj->getProperty("user").getDynamicObject();
                    if (userObj->hasProperty("confirmation_sent_at"))
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
                    safeThis->passwordTE->clear();
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
    passwordTE->clear();

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
    passwordTE->clear();

    UsernameLabel->setVisible(true);
    UsernameTE->setVisible(true);
    doneTB->setVisible(true);
    backTB->setVisible(true);
    forgotPassTB->setVisible(false);
    loginTB->setVisible(false);
    signupTB->setVisible(false);
}