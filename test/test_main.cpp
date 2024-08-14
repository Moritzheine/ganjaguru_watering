// #include <Arduino.h>
// #include <unity.h>
// #include "PumpController.h"
// #include "ServoSwitch.h"
// #include "ScaleModule.h"

// // Mock objects and functions
// class MockScaleModule : public ScaleModule
// {
// public:
//     MockScaleModule(int dataPin, int clockPin) : ScaleModule(dataPin, clockPin) {}
//     float getWeight()
//     {
//         // Simulate a constant rate of weight increase (1g/s)
//         return (millis() / 1000.0);
//     }
// };

// // Global objects
// ServoSwitch switches[] = {ServoSwitch(5), ServoSwitch(6), ServoSwitch(7), ServoSwitch(8)};
// MockScaleModule scaleModule(2, 3);
// PumpController pumpController(switches, 4, 4);

// void test_pump_functionality()
// {
//     // log Start
//     Serial.begin(9600);

//     // Reset test environment
//     unsigned long startTime = millis();

//     // log starttime
//     Serial.println(startTime);

//     // Set initial conditions
//     float initialAmount = 5.0; // Target grams
//     float fraction = 0.5;
//     unsigned long stabilizationTime = 3000; // in ms

//     auto remainingAmountFunc = [&]() -> float
//     {
//         return initialAmount - scaleModule.getWeight();
//     };

//     // Start the test
//     pumpController.addSubstance(0, initialAmount, fraction, stabilizationTime, remainingAmountFunc);

//     // Allow some time for the pump to run
//     while (millis() - startTime < 6000)
//     {
//         pumpController.update();
//         delay(100);
//     }

//     // Check if the pump controller correctly handled the fractional addition
//     float expectedAmount = initialAmount * (1 - pow(fraction, 2)); // After two cycles
//     TEST_ASSERT_FLOAT_WITHIN(0.1, expectedAmount, scaleModule.getWeight());
// }

// void setup()
// {
//     UNITY_BEGIN(); // IMPORTANT LINE!
//     RUN_TEST(test_pump_functionality);
//     UNITY_END(); // stop unit testing
// }

// void loop()
// {
//     // Not used in this testing setup.
// }

#include <Arduino.h>
#include <unity.h>

String STR_TO_TEST;

void setUp(void)
{
    // set stuff up here
    STR_TO_TEST = "Hello, world!";
}

void tearDown(void)
{
    // clean stuff up here
    STR_TO_TEST = "";
}

void test_string_concat(void)
{
    String hello = "Hello, ";
    String world = "world!";
    TEST_ASSERT_EQUAL_STRING(STR_TO_TEST.c_str(), (hello + world).c_str());
}

void test_string_substring(void)
{
    TEST_ASSERT_EQUAL_STRING("Hello", STR_TO_TEST.substring(0, 5).c_str());
}

void test_string_index_of(void)
{
    TEST_ASSERT_EQUAL(7, STR_TO_TEST.indexOf('w'));
}

void test_string_equal_ignore_case(void)
{
    TEST_ASSERT_TRUE(STR_TO_TEST.equalsIgnoreCase("HELLO, WORLD!"));
}

void test_string_to_upper_case(void)
{
    STR_TO_TEST.toUpperCase();
    TEST_ASSERT_EQUAL_STRING("HELLO, WORLD!", STR_TO_TEST.c_str());
}

void test_string_replace(void)
{
    STR_TO_TEST.replace('!', '?');
    TEST_ASSERT_EQUAL_STRING("Hello, world?", STR_TO_TEST.c_str());
}

void setup()
{
    delay(2000); // service delay
    UNITY_BEGIN();

    RUN_TEST(test_string_concat);
    RUN_TEST(test_string_substring);
    RUN_TEST(test_string_index_of);
    RUN_TEST(test_string_equal_ignore_case);
    RUN_TEST(test_string_to_upper_case);
    RUN_TEST(test_string_replace);

    UNITY_END(); // stop unit testing
}

void loop()
{
}