#include "ServoSwitch.h"

ServoSwitch::ServoSwitch(int pin, const char *name) : servoPin(pin), servoName(name), openState(false)
{

    servo.setPeriodHertz(50);          // Standard 50hz servo
    servo.attach(servoPin, 500, 2400); // Attach the servo on a pin (GPIO) with 500us-2400us pulse width range
    close();                           // Ensure the switch starts closed
}

void ServoSwitch::set(int angle)
{
    // this->attach();
    servo.write(angle);
    // this->relax();
}

void ServoSwitch::open()
{
    // this->attach();
    // Serial.println("Opening switch");
    servo.write(0); // Assuming 90° is the open position
    openState = true;
    delay(200);
    // this->relax();
}

void ServoSwitch::close()
{
    // this->attach();
    // Serial.println("Closing switch");
    servo.write(70); // Assuming 0° is the closed position
    delay(200);
    servo.write(55); // Assuming 0° is the closed position
    openState = false;
    // this->relax();
}

bool ServoSwitch::isOpen() const
{
    return openState;
}
