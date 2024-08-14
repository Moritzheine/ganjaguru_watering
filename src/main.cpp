#include <Arduino.h>
#include "LiquidManager.h"
#include "PumpController.h"
#include "UserInterface.h"

// Pin definitions
const int PUMP_PIN = 14;
const int FLUSH_SWITCH_PIN = 17;
const int SCALE_DATA_PIN = 4;
const int SCALE_CLOCK_PIN = 16;
const int ROTARY_PIN1 = 35;
const int ROTARY_PIN2 = 32;
const int BUTTON_PIN = 34;

LiquidManager &liquidManager = LiquidManager::getInstance();
ServoSwitch flushSwitch(FLUSH_SWITCH_PIN, "Flush");
PumpController pumpController(PUMP_PIN, &flushSwitch);
UserInterface userInterface(ROTARY_PIN1, ROTARY_PIN2, BUTTON_PIN);
// Define switches in an array
ServoSwitch switches[] = {
    ServoSwitch(5, "Bio Grow"),
    ServoSwitch(18, "Bio Bloom"),
    ServoSwitch(19, "Top Max")};

void setup()
{
  Serial.begin(115200);

  flushSwitch.close();
  for (auto &sw : switches)
  {
    sw.close();
  }

  // Add liquids with corresponding switches
  liquidManager.addLiquid("Bio Grow", 1.5, &switches[0]);
  liquidManager.addLiquid("Bio Bloom", 1.5, &switches[1]);
  liquidManager.addLiquid("Top Max", 1.5, &switches[2]);

  userInterface.init(liquidManager);
  pumpController.init(SCALE_DATA_PIN, SCALE_CLOCK_PIN);
  // pumpController.calibrateScale(100.0); // Calibrate with a known 100g weight

  // Other initialization code...
}

void loop()
{
  userInterface.update();
  pumpController.update();

  if (pumpController.isBusy())
  {
    String state = pumpController.getState();
    userInterface.displayDispenseProgress(state);
  }

  if (userInterface.isDispenseRequested())
  {
    int currentLiquidIndex = userInterface.getCurrentLiquidIndex();
    Liquid *selectedLiquid = liquidManager.getLiquid(currentLiquidIndex);

    if (selectedLiquid)
    {
      pumpController.dispense(selectedLiquid);
    }

    userInterface.resetDispenseRequest();
  }
  else if (userInterface.flushRequested())
  {
    pumpController.flush();
    userInterface.resetFlushRequest();
  }

  // Other loop code...
}