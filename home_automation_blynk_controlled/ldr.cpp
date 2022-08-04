#include "ldr.h"
#include "Arduino.h"
#include "main.h"

void init_ldr(void)
{
   pinMode(GARDEN_LIGHT, OUTPUT);
   
}
void brightness_control(void)
{
  unsigned short inputVal;
  // Read the values from LDR Sensor
  inputVal = analogRead(LDR_SENSOR);
  // Scale it down From (0 to 1023) to (0 to 255) 
  inputVal = inputVal/4;
  // Set the pwm from 255 to 0
  inputVal = 255 - inputVal;
  
  analogWrite (GARDEN_LIGHT, inputVal);
  delay(100);
}
