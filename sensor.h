#pragma once

#ifndef SENSOR_H
#define SENSOR_H

#if defined(ARDUINO) && ARDUINO >= 100
#include <Arduino.h>
#else
#include <WProgram.h>
#endif


class sensor
{
private:
    private:
    byte pin;
    byte state;
    byte lastReading;
    byte type;
    unsigned long lastDebounceTime = 0;
    unsigned long debounceDelay = 50;

public:
    sensor(byte pin, byte type);
    void init();
    void update();
    byte getState();
    bool detected();
    bool notDetected();
};

#endif