/*
  ==============================================================================

    SupabaseClient.cpp
    Created: 28 Feb 2026 3:30:00am
    Author:  Kisuke

  ==============================================================================
*/

#include "SupabaseClient.h"


SupabaseClient::SupabaseClient()
{

}

juce::String SupabaseClient::login(const juce::String& email,
    const juce::String& password)
{
    juce::String body = R"({"email":")" + email +
        R"(","password":")" + password + R"("})";
    // Call the Edge Function proxy instead of Supabase directly
    auto postUrl = juce::URL("https://ecmlftmkoqszdwjugqtn.supabase.co/functions/v1/auth-proxy?grant_type=password")
        .withPOSTData(body);
    juce::String extraHeaders;
    extraHeaders << "Content-Type: application/json\r\n";
    int statusCode = 0;
    auto stream = postUrl.createInputStream(
        juce::URL::InputStreamOptions(juce::URL::ParameterHandling::inAddress)
        .withExtraHeaders(extraHeaders)
        .withConnectionTimeoutMs(5000)
        .withStatusCode(&statusCode)
    );
    if (!stream) return "{\"error\":\"connection failed\"}";
    auto response = stream->readEntireStreamAsString();
    return response;
}



juce::String SupabaseClient::signup(const juce::String& email,
    const juce::String& password,
    const juce::String& username)
{
    juce::String body = R"({"email":")" + email +
        R"(","password":")" + password +
        R"(","data":{"name":")" + username + R"("}})";
    auto postUrl = juce::URL("https://ecmlftmkoqszdwjugqtn.supabase.co/functions/v1/auth-proxy?action=signup")
        .withPOSTData(body);
    juce::String extraHeaders;
    extraHeaders << "Content-Type: application/json\r\n";
    auto stream = postUrl.createInputStream(
        juce::URL::InputStreamOptions(juce::URL::ParameterHandling::inAddress)
        .withConnectionTimeoutMs(5000)
        .withExtraHeaders(extraHeaders)
    );
    if (!stream)
        return "{\"error\":\"connection failed\"}";
    return stream->readEntireStreamAsString();
}

juce::String SupabaseClient::incrementPlaytime(int seconds)
{
    juce::String id, token;
    {
        std::lock_guard<std::mutex> lock(mutex); //not needed, now for future refresh token/ re-login/update values while timer is running
        id = userId;
        token = accessToken;
    }

    juce::String body = R"({"user_id":")" + id + R"(","seconds":)" + juce::String(seconds) + "}";
    auto postUrl = juce::URL("https://ecmlftmkoqszdwjugqtn.supabase.co/functions/v1/increment-playtime-api")
        .withPOSTData(body);
    juce::String extraHeaders;
    extraHeaders << "Content-Type: application/json\r\n";
    extraHeaders << "Authorization: Bearer " + token + "\r\n";  
    int statusCode = 0;
    auto stream = postUrl.createInputStream(
        juce::URL::InputStreamOptions(juce::URL::ParameterHandling::inAddress)
        .withExtraHeaders(extraHeaders)
        .withConnectionTimeoutMs(5000)
        .withStatusCode(&statusCode)
    );
    if (!stream)
        return "{\"error\":\"connection failed\"}";
    auto response = stream->readEntireStreamAsString();
    
    return response;
}

juce::String SupabaseClient::addCurrency(int amount)
{
    return "";
}

void SupabaseClient::setUserId(const juce::String& newId)
{
    std::lock_guard<std::mutex> lock(mutex);
    this->userId = newId;
}

void SupabaseClient::setAccessToken(const juce::String& newAccessToken)
{
    std::lock_guard<std::mutex> lock(mutex);
    this->accessToken = newAccessToken;
}

juce::String SupabaseClient::addOrUpdateDevice(const juce::String& VID, const juce::String& PID, const juce::String& deviceName, int nrKeys)
{
    juce::String id, token;
    {
        std::lock_guard<std::mutex> lock(mutex); // protects userId/accessToken
        id = userId;
        token = accessToken;
    }

    
    juce::String body = R"({"user_id":")" + id +
        R"(","vendor_id":")" + VID +
        R"(","product_id":")" + PID +
        R"(","device_name":")" + deviceName +
        R"(","nr_keys":)" + juce::String(nrKeys) + "}";

    
    auto postUrl = juce::URL("https://ecmlftmkoqszdwjugqtn.supabase.co/functions/v1/insert-or-update-device-api")
        .withPOSTData(body);

    
    juce::String extraHeaders;
    extraHeaders << "Content-Type: application/json\r\n";
    extraHeaders << "Authorization: Bearer " + token + "\r\n";

    int statusCode = 0;

    
    auto stream = postUrl.createInputStream(
        juce::URL::InputStreamOptions(juce::URL::ParameterHandling::inAddress)
        .withExtraHeaders(extraHeaders)
        .withConnectionTimeoutMs(5000)
        .withStatusCode(&statusCode)
    );

    if (!stream)
        return "{\"error\":\"connection failed\"}";

    
    auto response = stream->readEntireStreamAsString();

    return response;
}