#include "PumpController.h"
#include "Logger.h"

PumpController::PumpController(int pumpPin, ServoSwitch *flushSwitch)
    : pumpPin(pumpPin), flushSwitch(flushSwitch), state(State::IDLE),
      estimatedFlowRate(1.0), minFlowRate(0.8), maxFlowRate(10.0),
      stabilizationTime(500), flushingTime(8000), lastDispensedAmount(0.0),
      fraction(0.6) // Added fraction for partial dispensing
{
}

void PumpController::init(int scaleDataPin, int scaleClockPin)
{
    pinMode(pumpPin, OUTPUT);
    pumpOff();
    dispenseTime = 0;
    scale.begin(scaleDataPin, scaleClockPin);
    scale.set_scale(2111.45); // Use a calibration factor from calibrateScale()
    scale.tare();
    Logger::log("Pump controller initialized", Logger::INFO);
}

void PumpController::calibrateScale(float knownWeight)
{
    Logger::log("Place known weight on the scale...", Logger::INFO);
    delay(5000);
    long reading = scale.get_units(10);
    float calibrationFactor = reading / knownWeight;
    scale.set_scale(calibrationFactor);
    Logger::log("Scale calibrated. Calibration factor: " + String(calibrationFactor), Logger::INFO);
}

void PumpController::flush()
{
    if (state != State::IDLE)
    {
        Logger::log("Pump is busy. Cannot flush.", Logger::WARNING);
        return;
    }
    Logger::log("Flushing pump...", Logger::INFO);
    flushSwitch->open();
    pumpOn();
    delay(flushingTime);
    pumpOff();
    flushSwitch->close();
    Logger::log("Pump flushed", Logger::INFO);
}

void PumpController::dispense(Liquid *liquid)
{
    if (state == State::IDLE)
    {
        activeLiquid = liquid;
        activeLiquid->clearDataPoints();
        activeLiquid->addDataPoint(0);
        lastDispensedAmount = 0.0;
        remainingAmount = activeLiquid->targetAmount;
        startInitialFlushing();
    }
    else
    {
        Logger::log("Pump is busy. Cannot start new dispense operation.", Logger::WARNING);
    }
}

void PumpController::update()
{
    if (state != State::IDLE)
    {
        checkStateTimeout();
    }

    switch (state)
    {
    case State::INITIAL_FLUSHING:
        updateInitialFlushing();
        break;
    case State::DISPENSING:
        updateDispensing();
        break;
    case State::STABILIZING:
        updateStabilizing();
        break;
    case State::FINAL_FLUSHING:
        updateFinalFlushing();
        break;
    case State::DONE:
        state = State::IDLE;
        break;
    default:
        break;
    }
}

void PumpController::startInitialFlushing()
{
    Logger::log("Starting initial flushing...", Logger::INFO);
    flushSwitch->open();
    lastActionTime = millis();
    state = State::INITIAL_FLUSHING;
    pumpOn();
}

void PumpController::updateInitialFlushing()
{
    static float lastWeight = 0;
    static unsigned long lastWeightChange = 0;
    float currentWeight = getCurrentWeight();

    if (abs(currentWeight - lastWeight) > 0.1)
    {
        lastWeight = currentWeight;
        lastWeightChange = millis();
    }

    if (millis() - lastWeightChange > 2000)
    {
        Logger::log("Initial flushing complete. Starting dispensing...", Logger::INFO);
        pumpOff();
        delay(500);
        flushSwitch->close();

        scale.tare();
        initialWeight = getCurrentWeight();
        activeLiquid->switch_->open();
        startDispensing();
    }
}

void PumpController::startDispensing()
{
    Logger::log("Starting dispensing...", Logger::INFO);
    state = State::DISPENSING;
    lastActionTime = millis();

    float amountToDispense = remainingAmount * fraction;
    Logger::log("Dispensing " + String(amountToDispense) + "g out of " + String(remainingAmount) + "g remaining", Logger::INFO);

    pumpOn();
}

void PumpController::updateDispensing()
{
    float targetThisIteration = remainingAmount * fraction;
    long pumpTime = millis() - lastDispenseTime;

    if (pumpTime >= calculateDispenseTime(targetThisIteration))
    {
        pumpOff();
        Logger::log("Calculated dispense time " + String(calculateDispenseTime(targetThisIteration)) + "ms", Logger::INFO);
        Logger::log("Pump was on for " + String(pumpTime) + "ms", Logger::INFO);
        Logger::log("Dispensing stopped; Stabilizing...", Logger::INFO);
        state = State::STABILIZING;
        lastActionTime = millis();
    }
}

void PumpController::updateStabilizing()
{
    if (millis() - lastActionTime > stabilizationTime)
    {
        float currentWeight = getCurrentWeight();
        float dispensedAmount = currentWeight - initialWeight;
        float dispensedThisIteration = dispensedAmount - lastDispensedAmount;
        activeLiquid->addDataPoint(dispensedAmount);

        // Adjust flow rate based on the last iteration
        // adjustFlowRate(dispensedThisIteration);

        lastDispensedAmount = dispensedAmount;
        remainingAmount = activeLiquid->targetAmount - dispensedAmount;

        Logger::log("After stabilization - Dispensed: " + String(dispensedAmount) + "g, Remaining: " + String(remainingAmount) + "g", Logger::INFO);

        if (remainingAmount > 0.01)
        {
            Logger::log("Stabilization complete. Resuming dispensing for remaining " + String(remainingAmount) + "g", Logger::INFO);
            startDispensing();
        }
        else
        {
            activeLiquid->switch_->close();
            startFinalFlushing();
        }
    }
}

void PumpController::startFinalFlushing()
{
    Logger::log("Starting final flushing...", Logger::INFO);
    flushSwitch->open();
    state = State::FINAL_FLUSHING;
    lastActionTime = millis();
    pumpOn();
}

void PumpController::updateFinalFlushing()
{
    if (millis() - lastActionTime > flushingTime)
    {
        Logger::log("Flushing ended", Logger::INFO);
        pumpOff();
        flushSwitch->close();
        delay(500);
        float finalWeight = getCurrentWeight();
        float totalDispensed = finalWeight - initialWeight;
        activeLiquid->addDataPoint(finalWeight);
        if (totalDispensed < activeLiquid->targetAmount && abs(totalDispensed - activeLiquid->targetAmount) > 0.1)
        {
            Logger::log("Dispense operation incomplete. Total dispensed: " + String(totalDispensed) + "g", Logger::WARNING);
            state = State::STABILIZING;
            activeLiquid->switch_->open();
            return;
        }
        state = State::DONE;
        Logger::log("Dispense operation complete. Total dispensed: " + String(totalDispensed) + "g", Logger::INFO);
    }
}

void PumpController::pumpOn()
{
    lastDispenseTime = millis();
    digitalWrite(pumpPin, HIGH);
    Logger::log("Pump turned on", Logger::INFO);
}

void PumpController::pumpOff()
{
    dispenseTime += millis() - lastDispenseTime;
    digitalWrite(pumpPin, LOW);
    Logger::log("Pump turned off", Logger::INFO);
}

bool PumpController::isBusy() const
{
    return state != State::IDLE;
}

float PumpController::getCurrentWeight()
{
    float weight = scale.get_units(10);
    return weight;
}

void PumpController::adjustFlowRate(float actualDispensed)
{
    unsigned long elapsedTime = millis() - lastDispenseTime;
    float newFlowRate = (actualDispensed / elapsedTime) * 1000;
    Logger::log("Current flow rate: " + String(newFlowRate) + " g/s", Logger::INFO);

    // Use a moving average to smooth out flow rate adjustments
    estimatedFlowRate = (estimatedFlowRate * 0.7) + (newFlowRate * 0.3);
    estimatedFlowRate = constrain(estimatedFlowRate, minFlowRate, maxFlowRate);

    Logger::log("Adjusted estimated flow rate to: " + String(estimatedFlowRate) + " g/s", Logger::INFO);

    // Adjust fraction based on how close we are to the target
    if (remainingAmount < activeLiquid->targetAmount * 0.2)
    {
        fraction = 0.25; // Smaller fraction for fine-tuning
    }
    else if (remainingAmount < activeLiquid->targetAmount * 0.5)
    {
        fraction = 0.4; // Medium fraction for mid-range
    }
    else
    {
        fraction = 0.5; // Larger fraction for initial dispenses
    }
    Logger::log("Adjusted fraction to: " + String(fraction), Logger::INFO);
}

unsigned long PumpController::calculateDispenseTime(float grams)
{
    return static_cast<unsigned long>((grams / estimatedFlowRate) * 1000);
}

void PumpController::checkStateTimeout()
{
    if (millis() - lastActionTime > MAX_STATE_DURATION)
    {
        Logger::log("State timeout occurred. Resetting to IDLE state.", Logger::ERROR);
        pumpOff();
        flushSwitch->close();
        if (activeLiquid && activeLiquid->switch_)
        {
            activeLiquid->switch_->close();
        }
        state = State::IDLE;
    }
}