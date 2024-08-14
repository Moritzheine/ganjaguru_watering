#pragma once
#include "LiquidManager.h"
#include "HX711.h"

class PumpController
{
    enum class State
    {
        IDLE,
        INITIAL_FLUSHING,
        DISPENSING,
        STABILIZING,
        FINAL_FLUSHING,
        DONE
    };

public:
    PumpController(int pumpPin, ServoSwitch *flushSwitch);
    void init(int scaleDataPin, int scaleClockPin);
    void calibrateScale(float knownWeight);
    void dispense(Liquid *liquid);
    void update();
    bool isBusy() const;
    float getCurrentWeight();
    void flush();

    String getState() const
    {
        switch (state)
        {
        case State::IDLE:
            return "IDLE";
        case State::INITIAL_FLUSHING:
            return "INITIAL_FLUSH";
        case State::DISPENSING:
            return "DISPENSING";
        case State::STABILIZING:
            return "STABILIZING";
        case State::FINAL_FLUSHING:
            return "FINAL_FLUSH";
        case State::DONE:
            return "DONE";
        default:
            return "UNKNOWN";
        }
    }

private:
    int MAX_STATE_DURATION = 30000;

    float fraction;
    float remainingAmount;

    void pumpOn();
    void pumpOff();
    void startInitialFlushing();
    void updateInitialFlushing();
    void startDispensing();
    void updateDispensing();
    void updateStabilizing();
    void startFinalFlushing();
    void updateFinalFlushing();
    void adjustFlowRate(float actualDispensed);
    unsigned long calculateDispenseTime(float grams);
    float getRemainingAmount();
    void checkStateTimeout();

    int pumpPin;
    ServoSwitch *flushSwitch;
    HX711 scale;
    Liquid *activeLiquid;
    State state;
    unsigned long lastActionTime;
    unsigned long lastDispenseTime;
    unsigned long dispenseTime;
    float lastDispensedAmount;
    int flushingTime;
    float initialWeight;

    // Pump-specific parameters
    float estimatedFlowRate;
    float minFlowRate;
    float maxFlowRate;
    const unsigned long minUpdateInterval = 100;
    const unsigned long stabilizationTime = 500;
    const float dispenseFraction = 0.6;
};