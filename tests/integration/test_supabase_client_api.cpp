#include <juce_core/juce_core.h>
#include "SupabaseClient.h"

// ==================================================================
// NETWORK-DEPENDENT TESTS (Integration)
// These tests require network connection for the https calls
// ==================================================================


class SupabaseClientTest : public juce::UnitTest
{
public:
	SupabaseClientTest() : juce::UnitTest("supabaseClient", "Integration-api") {}

	void runTest() override
	{
        testLoginInvalid();
        testLoginConnectionFailure();
        testSignupRandomEmail();
        testIncrementPlaytimeInvalidToken();
        testAddOrUpdateDeviceInvalidToken();
	}

private:
    void testLoginInvalid()
    {
        beginTest("Login with invalid credentials returns 400 or 500");

        SupabaseClient client;
        auto result = client.login("invalid@test.com", "wrongpassword");

        expect(result.statusCode == 400 || result.statusCode == 500,
            "Expected 401 or 500 for invalid login");

        expect(result.body.isNotEmpty(), "Response body should not be empty");
    }

    void testLoginConnectionFailure()
    {
        beginTest("Login handles connection failure gracefully");

        SupabaseClient client;

        
        auto result = client.login("", "");

        expect(result.statusCode == 400 || result.statusCode == 500,
            "Connection failure should return 400 or 500");

        expect(result.body.contains("validation_failed") || result.body.contains("Internal server error"),
            "Expected connection failure or server error message");
    }

    void testSignupRandomEmail()
    {
        beginTest("Signup returns a response for random email");

        SupabaseClient client;

        auto randomEmail =
            "test_" + juce::String(juce::Random::getSystemRandom().nextInt()) + "@test.com";

        auto result = client.signup(randomEmail, "password123", "TestUser");

        expect(result.statusCode == 200 || result.statusCode == 400 || result.statusCode == 500,
            "Expected 200, 400, or 500 status code");
    }

    void testIncrementPlaytimeInvalidToken()
    {
        beginTest("Increment playtime with invalid token returns 401 or 500");

        SupabaseClient client;
        client.setUserId("00000000-0000-0000-0000-000000000000");
        client.setAccessToken("invalidtoken");

        auto result = client.incrementPlaytime(60, "vid-sample", "pid-sample");

        expect(result.statusCode == 401 || result.statusCode == 500,
            "Expected 401 or 500 for invalid token");

        expect(result.body.contains("error") || result.body.contains("Internal server error"),
            "Expected error message in body");
    }

    void testAddOrUpdateDeviceInvalidToken()
    {
        beginTest("AddOrUpdateDevice with invalid token returns 401 or 500");

        SupabaseClient client;
        client.setUserId("00000000-0000-0000-0000-000000000000");
        client.setAccessToken("invalidtoken");

        auto result = client.addOrUpdateDevice("VID-SAMPLE", "PID-SAMPLE", "Test Device", 61);

        expect(result.statusCode == 401 || result.statusCode == 500,
            "Expected 401 or 500 for invalid token");

        expect(result.body.contains("error") || result.body.contains("Internal server error"),
            "Expected error message in body");
    }
};

//static SupabaseClientTest supabaseClientTest;