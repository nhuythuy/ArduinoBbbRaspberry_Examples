// I2C Digital Potentiometer
// by Thuy Huy Nguyen
// Controls AD5252 digital potentiometer via I2C/TWI
// Created 25 SEPT 2015
// See this before using
// http://www.analog.com/en/education/education-library/product-faqs/ad5252.html
// In any case, voltages across terminals W-A, W-B, or A-B of all digital potentiometers
// (except AD7376, AD5260/2, AD5280/82, AD5290, and AD5263) should be limited to |5V|, the polarity constraint.


#include <Wire.h>

void setup()
{
  Serial.begin(9600);
  Serial.println("\nI2C Digital Potentiometer 2 channels\n");

  Wire.begin(); // join i2c bus (address optional for master)
}

byte oldVal1 = 0;
byte oldVal2 = 0;

byte curVal1 = 0;
byte curVal2 = 0;

byte val = 0;

byte updatePosVal(byte pot, byte value)
{
  Wire.beginTransmission(0x2C); // transmit to device #44 (0x2c), device address is specified in datasheet
  Wire.write(byte(pot));       // sends instruction byte for the first potentiometer
  Wire.write(value);              // sends potentiometer value byte  
  byte ret = Wire.endTransmission();     // stop transmitting
  if(ret != 0)
  {
    Serial.print("I2C Digital Potentiometer error code 0x");
    Serial.print(ret,HEX);
    Serial.println("  !");
  }
  else
  {
    Serial.print("I2C Digital Potentiometer value: ");
    Serial.println(val,HEX);
  }
}

void loop()
{
  // Potentiometer 1
  if(oldVal1 != curVal1)
  {
    updatePosVal(0x01, curVal1);
    oldVal1 = curVal1;
  }
  
  // Potentiometer 2
  if(oldVal2 != curVal2)
  {
    updatePosVal(0x03, curVal2);
    oldVal2 = curVal2;
  }

  val++;        // increment value
  curVal1 = val;
  curVal2 = val;
  if(val == 256) // if reached 256th position (max)
    val = 0;    // start over from lowest value

  delay(500);
}


