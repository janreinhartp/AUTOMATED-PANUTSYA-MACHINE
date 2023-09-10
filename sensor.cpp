#include "sensor.h"
sensor::sensor(byte pin, byte type)
{
    this->pin = pin;
    this->type = type;
    lastReading = LOW;
    init();
}

void sensor::init()
{
    pinMode(pin, INPUT_PULLUP);
    update();
}

void sensor::update()
{
    // You can handle the debounce of the button directly
    // in the class, so you don't have to think about it
    // elsewhere in your code
    byte newReading = digitalRead(pin);

    if (newReading != lastReading)
    {
        lastDebounceTime = millis();
    }

    if (millis() - lastDebounceTime > debounceDelay)
    {
        // Update the 'state' attribute only if debounce is checked
        state = newReading;
    }

    lastReading = newReading;
}

byte sensor::getState()
{
    //update();
    if (type = 1)
    {
        if(state = 1){
            return 1;
        }else{
            return 0;
        }
        
    }
    else
    {
        if(state = 0){
            return 1;
        }else{
            return 0;
        }
    }
}

bool sensor::detected()
{
    return (getState() == LOW);
}