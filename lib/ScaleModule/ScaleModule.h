#ifndef SCALE_MODULE_H
#define SCALE_MODULE_H

class ScaleModule
{
public:
    ScaleModule(int dataPin, int clockPin);
    void tare();
    float getWeight(); // Returns weight in grams
    void calibrate();

private:
    int dataPin;
    int clockPin;
};

#endif
