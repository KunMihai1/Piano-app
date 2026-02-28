/*
  ==============================================================================

    SupabaseClient.h
    Created: 28 Feb 2026 3:30:00am
    Author:  Kisuke

  ==============================================================================
*/

#pragma once

#include "JuceHeader.h"

class SupabaseClient
{
public:
    SupabaseClient();

    juce::String login(const juce::String& email, const juce::String& password);

    juce::String signup(const juce::String& email, const juce::String& password, const juce::String& username);

    juce::String incrementPlaytime(int seconds);

    juce::String addCurrency(int amount);

    void setUserId(const juce::String& newId);

    void setAccessToken(const juce::String& newAccessToken);

    juce::String addOrUpdateDevice(const juce::String& PID, const juce::String& VID);

private:
    std::mutex mutex;
    juce::String userId;
    juce::String accessToken;
};