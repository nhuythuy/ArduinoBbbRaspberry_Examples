/*
However, if you use 1.1V as aRef, the equation changes entirely.
If you divide 1.1V over 1024, each step up in the analog reading
is equal to approximately 0.001074V = 1.0742 mV. If 10mV is equal to 1 degree Celcius,
10 / 1.0742 = ~9.31. So, for every change of 9.31 in the analog reading.
There is one degree of temperature change.

To change aRef to 1.1V, you use the command "analogReference(INTERNAL);"
*/

float tempC;
int reading;
int tempPin = 0;

void setup()
{
//  analogReference(INTERNAL);  // Vref= 1.1V
  Serial.begin(9600);
}

void loop()
{
  reading = analogRead(tempPin);
  temp = (5.0 * analogRead(tempPin) * 100.0) / 1024;
  //tempC = reading / 9.31;
  Serial.println(tempC);
  delay(1000);
}
