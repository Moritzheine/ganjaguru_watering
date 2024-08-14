#ifndef SERVO_SWITCH_H
#define SERVO_SWITCH_H

#include <ESP32Servo.h>

class ServoSwitch
{
public:
    ServoSwitch(int pin, const char *name);
    void open();
    void close();
    void set(int angle);

    bool isOpen() const;
    const char *getName() const { return servoName; }
    void relax() { servo.detach(); }
    void attach()
    {
        servo.attach(servoPin, 500, 2400);
    }

private:
    Servo servo; // ESP32Servo instance
    int servoPin;
    const char *servoName;
    bool openState;
    bool isRelaxed;
};

#endif
