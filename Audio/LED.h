#ifndef LED_h
#define LED_h

#include <Arduino.h>
#include <ShiftOutX.h>
#include <ShiftPinNo.h>

class LED 
{
    public:
        LED(long _pin1, long _pin2);
        bool selected;
        long pin1;
        long pin2;
        char val;
        void update(shiftOutX reg);
};
#endif