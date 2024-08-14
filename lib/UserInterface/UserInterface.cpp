
// UserInterface.cpp
#include "UserInterface.h"

UserInterface::UserInterface(int rotaryPin1, int rotaryPin2, int buttonPin)
    : display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET),
      encoder(rotaryPin1, rotaryPin2),
      button(buttonPin),
      currentLiquidIndex(0),
      dispenseRequested(false),
      lastEncoderValue(0),
      currentState(State::SELECT_LIQUID),
      flushRequest(false)
{
}

void UserInterface::init(LiquidManager &liquidManager)
{
    this->liquidManager = &liquidManager;

    Wire.begin(25, 26); // SDA on 25, SCL on 26
    if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS))
    {
        Serial.println(F("SSD1306 allocation failed"));
        for (;;)
            ; // Don't proceed, loop forever
    }

    display.clearDisplay();
    display.setRotation(2);
    display.setTextColor(SSD1306_WHITE);

    button.attachClick(onButtonClick, this);
    button.attachLongPressStart(onButtonLongPress, this);
    button.attachDoubleClick(onButtonDoubleClick, this);

    displayMainScreen();
    Logger::log("User interface initialized");
}

void UserInterface::update()
{
    handleRotaryEncoder();
    button.tick();
}

void UserInterface::displayMainScreen()
{
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.println("Select Liquid:");
    display.setTextSize(2);
    display.setCursor(0, 16);
    Liquid *liquid = liquidManager->getLiquid(currentLiquidIndex);
    if (liquid)
    {
        display.println(liquid->name);
    }
    display.setTextSize(1);
    display.setCursor(0, 40);
    display.print("Amount: ");
    if (liquid)
    {
        display.print(liquid->targetAmount, 1);
        display.println("g");
    }
    display.setCursor(0, 56);
    display.print(currentState == State::EDIT_AMOUNT ? "Editing Amount" : "Press to Edit");
    display.display();
}
void UserInterface::displayDispenseProgress(const String &pumpState)
{
    display.clearDisplay();
    displayHeader("Dispensing");
    displayLiquidInfo();
    displayPumpState(pumpState);
    displayProgressBarWithLabels();
    display.display();
}

void UserInterface::displayHeader(const String &title)
{
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.println(title);
    display.drawFastHLine(0, 9, SCREEN_WIDTH, SSD1306_WHITE);
}

void UserInterface::displayLiquidInfo()
{
    Liquid *liquid = liquidManager->getLiquid(currentLiquidIndex);
    if (liquid)
    {
        display.setTextSize(2);
        display.setCursor(0, 14);
        display.println(liquid->name);
    }
}

void UserInterface::displayPumpState(const String &pumpState)
{
    display.setTextSize(1);
    display.setCursor(0, 32);
    display.print("State: ");
    display.println(pumpState);
}

void UserInterface::displayProgressBarWithLabels()
{
    Liquid *liquid = liquidManager->getLiquid(currentLiquidIndex);
    if (liquid)
    {
        float currentWeight = liquid->dataPoints.empty() ? 0 : liquid->dataPoints.back().weight;
        float targetWeight = liquid->targetAmount;
        float percentage = min(currentWeight / targetWeight * 100, 100.0f);

        // Display progress bar
        drawProgressBar(0, 44, SCREEN_WIDTH, 10, percentage);

        // Display current weight (left aligned)
        display.setTextSize(1);
        display.setCursor(0, 56);
        display.print(currentWeight, 2);
        display.print("g");

        // Display target weight (right aligned)
        String targetStr = String(targetWeight, 2) + "g";
        int16_t targetWidth = targetStr.length() * 6; // Approximate width of the text
        display.setCursor(SCREEN_WIDTH - targetWidth, 56);
        display.print(targetStr);
    }
}

void UserInterface::drawProgressBar(int x, int y, int width, int height, float percentage)
{
    display.drawRect(x, y, width, height, SSD1306_WHITE);
    int fillWidth = (width - 2) * percentage / 100.0;
    display.fillRect(x + 1, y + 1, fillWidth, height - 2, SSD1306_WHITE);
}

void UserInterface::handleRotaryEncoder()
{
    encoder.tick();
    int newValue = encoder.getPosition();
    if (newValue != lastEncoderValue)
    {
        int direction = (newValue > lastEncoderValue) ? 1 : -1;
        if (currentState == State::SELECT_LIQUID)
        {
            currentLiquidIndex = (currentLiquidIndex + direction + liquidManager->getLiquidCount()) % liquidManager->getLiquidCount();
            Logger::log("Selected liquid: " + liquidManager->getLiquid(currentLiquidIndex)->name);
        }
        else if (currentState == State::EDIT_AMOUNT)
        {
            updateAmount(direction);
        }
        displayMainScreen();
        lastEncoderValue = newValue;
    }
}

void UserInterface::updateAmount(int direction)
{
    Liquid *liquid = liquidManager->getLiquid(currentLiquidIndex);
    if (liquid)
    {
        liquid->targetAmount += direction * 0.1;
        if (liquid->targetAmount < 0)
        {
            liquid->targetAmount = 0;
        }
        Logger::log("Updated amount for " + liquid->name + ": " +
                    String(liquid->targetAmount) + "g");
    }
}

bool UserInterface::flushRequested()
{
    return flushRequest;
}

void UserInterface::resetFlushRequest()
{
    flushRequest = false;
}

void UserInterface::onButtonClick(void *ptr)
{
    UserInterface *ui = static_cast<UserInterface *>(ptr);
    if (ui->currentState == State::SELECT_LIQUID)
    {
        ui->currentState = State::EDIT_AMOUNT;
        Logger::log("Editing amount for " + ui->liquidManager->getLiquid(ui->currentLiquidIndex)->name);
    }
    else if (ui->currentState == State::EDIT_AMOUNT)
    {
        ui->currentState = State::SELECT_LIQUID;
        Logger::log("Finished editing amount");
    }
    ui->displayMainScreen();
}

void UserInterface::onButtonLongPress(void *ptr)
{
    UserInterface *ui = static_cast<UserInterface *>(ptr);
    ui->dispenseRequested = true;
    Logger::log("Dispense requested for " + ui->liquidManager->getLiquid(ui->currentLiquidIndex)->name);
}

void UserInterface::onButtonDoubleClick(void *ptr)
{
    UserInterface *ui = static_cast<UserInterface *>(ptr);
    ui->flushRequest = true;
    Logger::log("Flush requested");
}

int UserInterface::getCurrentLiquidIndex() const
{
    return currentLiquidIndex;
}

float UserInterface::getCurrentTargetAmount() const
{
    Liquid *liquid = liquidManager->getLiquid(currentLiquidIndex);
    return liquid ? liquid->targetAmount : 0.0f;
}

bool UserInterface::isDispenseRequested() const
{
    return dispenseRequested;
}

void UserInterface::resetDispenseRequest()
{
    dispenseRequested = false;
}