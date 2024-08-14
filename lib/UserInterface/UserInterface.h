// UserInterface.h
#ifndef USER_INTERFACE_H
#define USER_INTERFACE_H

#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <RotaryEncoder.h>
#include <OneButton.h>
#include <Wire.h>
#include "LiquidManager.h"
#include "Logger.h"

class UserInterface
{
public:
    UserInterface(int rotaryPin1, int rotaryPin2, int buttonPin);
    void init(LiquidManager &liquidManager);
    void update();
    int getCurrentLiquidIndex() const;
    float getCurrentTargetAmount() const;
    bool isDispenseRequested() const;
    void resetDispenseRequest();
    bool flushRequested();
    void resetFlushRequest();
    void displayMainScreen();
    void displayDispenseProgress(const String &pumpState);

private:
    enum class State
    {
        SELECT_LIQUID,
        EDIT_AMOUNT,
        DISPENSING
    };

    static const int SCREEN_WIDTH = 128;
    static const int SCREEN_HEIGHT = 64;
    static const int OLED_RESET = -1;
    static const int SCREEN_ADDRESS = 0x3C;

    Adafruit_SSD1306 display;
    RotaryEncoder encoder;
    OneButton button;
    LiquidManager *liquidManager;
    int currentLiquidIndex;
    bool dispenseRequested;
    bool flushRequest;
    int lastEncoderValue;
    State currentState;

    void handleRotaryEncoder();
    void updateAmount(int direction);
    void displayHeader(const String &title);
    void displayLiquidInfo();
    void displayWeightProgress();
    void displayPumpState(const String &pumpState);
    void displayProgressBar();
    void displayProgressBarWithLabels();
    void drawProgressBar(int x, int y, int width, int height, float percentage);

    static void onButtonClick(void *ptr);
    static void onButtonLongPress(void *ptr);
    static void onButtonDoubleClick(void *ptr);
};

#endif // USER_INTERFACE_H
