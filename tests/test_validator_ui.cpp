
#include <juce_core/juce_core.h>
#include "ValidatorUI.h"

class ValidatorTest : public juce::UnitTest
{
public:
    ValidatorTest() : juce::UnitTest("Validator") {}

    void runTest() override
    {
        // ---- isValidMidiIntValue ----

        beginTest("isValidMidiIntValue - valid range 0-127");
        {
            expect(Validator::isValidMidiIntValue(0) == true);
            expect(Validator::isValidMidiIntValue(64) == true);
            expect(Validator::isValidMidiIntValue(127) == true);
        }

        beginTest("isValidMidiIntValue - out of range");
        {
            expect(Validator::isValidMidiIntValue(-1) == false);
            expect(Validator::isValidMidiIntValue(128) == false);
            expect(Validator::isValidMidiIntValue(-100) == false);
            expect(Validator::isValidMidiIntValue(999) == false);
        }

        // ---- isValidMidiIntegerVelocities ----

        beginTest("isValidMidiIntegerVelocities - valid velocities");
        {
            expect(Validator::isValidMidiIntegerVelocities(0.0) == true);
            expect(Validator::isValidMidiIntegerVelocities(64.0) == true);
            expect(Validator::isValidMidiIntegerVelocities(127.0) == true);
        }

        beginTest("isValidMidiIntegerVelocities - invalid velocities");
        {
            expect(Validator::isValidMidiIntegerVelocities(-1.0) == false);
            expect(Validator::isValidMidiIntegerVelocities(128.0) == false);
        }

        // ---- isValidMidiIntegerString ----

        beginTest("isValidMidiIntegerString - valid strings");
        {
            expect(Validator::isValidMidiIntegerString("0") == true);
            expect(Validator::isValidMidiIntegerString("127") == true);
            expect(Validator::isValidMidiIntegerString("-5") == true);
            expect(Validator::isValidMidiIntegerString("+10") == true);
            expect(Validator::isValidMidiIntegerString("42") == true);
        }

        beginTest("isValidMidiIntegerString - invalid: letters");
        {
            expect(Validator::isValidMidiIntegerString("abc") == false);
            expect(Validator::isValidMidiIntegerString("12a") == false);
        }

        beginTest("isValidMidiIntegerString - invalid: minus/plus not at start");
        {
            expect(Validator::isValidMidiIntegerString("5-3") == false);
            expect(Validator::isValidMidiIntegerString("5+3") == false);
        }

        beginTest("isValidMidiIntegerString - invalid: double signs");
        {
            expect(Validator::isValidMidiIntegerString("++5") == false);
            expect(Validator::isValidMidiIntegerString("--5") == false);
        }

        beginTest("isValidMidiIntegerString - invalid: sign only");
        {
            expect(Validator::isValidMidiIntegerString("+") == false);
            expect(Validator::isValidMidiIntegerString("-") == false);
        }

        // ---- isValidMidiDoubleString ----

        beginTest("isValidMidiDoubleString - valid strings");
        {
            expect(Validator::isValidMidiDoubleString("0.5") == true);
            expect(Validator::isValidMidiDoubleString("123") == true);
            expect(Validator::isValidMidiDoubleString("-1.5") == true);
            expect(Validator::isValidMidiDoubleString("+2.0") == true);
            expect(Validator::isValidMidiDoubleString("0") == true);
        }

        beginTest("isValidMidiDoubleString - invalid: letters");
        {
            expect(Validator::isValidMidiDoubleString("1.5a") == false);
            expect(Validator::isValidMidiDoubleString("abc") == false);
        }

        beginTest("isValidMidiDoubleString - invalid: minus/plus not at start");
        {
            expect(Validator::isValidMidiDoubleString("5-3") == false);
            expect(Validator::isValidMidiDoubleString("5+3") == false);
        }

        beginTest("isValidMidiDoubleString - invalid: double dots or signs");
        {
            expect(Validator::isValidMidiDoubleString("1..5") == false);
            expect(Validator::isValidMidiDoubleString("++5") == false);
            expect(Validator::isValidMidiDoubleString("--5") == false);
        }

        beginTest("isValidMidiDoubleString - invalid: sign with dot only");
        {
            expect(Validator::isValidMidiDoubleString("+") == false);
            expect(Validator::isValidMidiDoubleString("-") == false);
            expect(Validator::isValidMidiDoubleString("+.") == false);
            expect(Validator::isValidMidiDoubleString("-.") == false);
        }

        // ---- isValidMidiDoubleValueTimeStamps ----

        beginTest("isValidMidiDoubleValueTimeStamps - valid within bounds");
        {
            
            expect(Validator::isValidMidiDoubleValueTimeStamps(0.5, 5.0, 4.0, 6.0) == true);
        }

        beginTest("isValidMidiDoubleValueTimeStamps - newTime goes negative");
        {
            
            expect(Validator::isValidMidiDoubleValueTimeStamps(-5.0, 1.0, 0.0, 2.0) == false);
        }

        beginTest("isValidMidiDoubleValueTimeStamps - newTime before previous");
        {
            
            expect(Validator::isValidMidiDoubleValueTimeStamps(-2.0, 5.0, 4.0, 6.0) == false);
        }

        beginTest("isValidMidiDoubleValueTimeStamps - newTime after next");
        {
            
            expect(Validator::isValidMidiDoubleValueTimeStamps(3.0, 5.0, 4.0, 6.0) == false);
        }
    }
};

static ValidatorTest validatorTest;
