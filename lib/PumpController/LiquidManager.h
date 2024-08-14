#pragma once
#include <vector>
#include <Arduino.h>
#include "ServoSwitch.h"

struct DataPoint
{
    unsigned long timestamp;
    float weight;
};

class Liquid
{
public:
    Liquid(const String &name, float targetAmount, ServoSwitch *switch_)
        : name(name), targetAmount(targetAmount), switch_(switch_) {}

    String name;
    float targetAmount;
    ServoSwitch *switch_;
    std::vector<DataPoint> dataPoints;

    void addDataPoint(float weight)
    {
        dataPoints.push_back({millis(), weight});
    }

    void clearDataPoints()
    {
        dataPoints.clear();
    }
};

class LiquidManager
{
public:
    static LiquidManager &getInstance()
    {
        static LiquidManager instance;
        return instance;
    }

    void addLiquid(const String &name, float targetAmount, ServoSwitch *switch_)
    {
        liquids.emplace_back(name, targetAmount, switch_);
    }

    Liquid *getLiquid(int index)
    {
        if (index >= 0 && index < liquids.size())
        {
            return &liquids[index];
        }
        return nullptr;
    }

    void updateLiquidAmount(int index, float newAmount)
    {
        if (index >= 0 && index < liquids.size())
        {
            liquids[index].targetAmount = newAmount;
        }
    }

    int getLiquidCount() const
    {
        return liquids.size();
    }

private:
    LiquidManager() {}
    std::vector<Liquid> liquids;
};