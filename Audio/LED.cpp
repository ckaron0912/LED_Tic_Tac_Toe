#include <LED.h>

//LED constructor
LED::LED(long _pin1, long _pin2)
{
	pin1 = _pin1;
	pin2 = _pin2;
	selected = false;
}

//updates the LEDs state
//changes color based on current values
//needs the reference to the shift register variable
//in order to use the library functions
void LED::update(shiftOutX reg)
{
	if (selected)
	{
		reg.pinOn(pin1);
		reg.pinOn(pin2);
	}
	else if (val == 'X')
	{
		reg.pinOn(pin1);
		reg.pinOff(pin2);
	}
	else if (val == 'O')
	{
		reg.pinOff(pin1);
		reg.pinOn(pin2);
	}
	else
	{
		reg.pinOff(pin1);
		reg.pinOff(pin2);
	}
}
