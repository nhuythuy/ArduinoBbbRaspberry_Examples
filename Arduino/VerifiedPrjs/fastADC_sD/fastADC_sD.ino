// http://forum.arduino.cc/index.php?topic=137635.0
// Be careful with memory limit: max possible of SAMPLES  10000

/*
  SD card datalogger
 This example shows how to log data from three analog sensors
 to an SD card using the SD library.

 The circuit:
 * analog sensors on analog ins 0, 1, and 2
 * SD card attached to SPI bus as follows:
 ** MOSI - pin 11
 ** MISO - pin 12
 ** CLK - pin 13
 ** CS - pin 4
*/
#include <SPI.h>
#include <SD.h>

const int chipSelect = 4;

void setup_sD()
{
  // Open serial communications and wait for port to open:
  Serial.begin(9600);

  Serial.print("Initializing SD card...");

  // see if the card is present and can be initialized:
  if (!SD.begin(chipSelect)) {
    Serial.println("Card failed, or not present");
    // don't do anything more:
    return;
  }
  Serial.println("card initialized.");
}

void setup_ADC()
{
  int t=analogRead(0);
  ADC->ADC_MR |= 0x80; // these lines set free running mode on adc 7 and adc 6 (pin A0 and A1 - see Due Pinout Diagram thread)
  ADC->ADC_CR=2;
  ADC->ADC_CHER=0xC0; // this is (1<<7) | (1<<6) for adc 7 and adc 6                                     
}
void setup() {
  setup_sD();
  setup_ADC();
}

#define SAMPLES  10000

void loop() {
 int t = 0;
 int q0=0,q1=0;
 int temp;
 int a0[SAMPLES];
 int a1[SAMPLES];
 int a2[SAMPLES];
 int a3[SAMPLES];

 Serial.println("Start conversion");
 t=micros();
 for(int i=0;i<SAMPLES;i++){  // 500000
    while((ADC->ADC_ISR & 0xC0)!=0xC0);// wait for two conversions (pin A0[7]  and A1[6])
   a0[i]=ADC->ADC_CDR[7];              // read data on A0 pin
   a1[i]=ADC->ADC_CDR[6];              // read data on A1 pin
//   q0+=a0[i];
//   q1+=a1[i];
 }
 
 t=micros()-t;
 Serial.print("100000 pairs of conversions in ");Serial.print(t);Serial.println(" micros");
 Serial.print("A0 total:");Serial.println(q0);
 Serial.print("A1 total:");Serial.println(q1);
 
for(int i=0;i<SAMPLES;i++)
{
  String dataString = "";
  temp = a0[i];
  dataString = String(temp);
  dataString += ";";
  temp = a1[i];
  dataString += String(temp);
//  Serial.println(dataString);
}


  // open the file. note that only one file can be open at a time,
  // so you have to close this one before opening another.
  File dataFile = SD.open("datalog.txt", FILE_WRITE);

  String dataString = "Data logger 100000 values each ADC channel";
  dataFile.println(dataString);
  dataFile.print("100000 pairs of conversions in ");dataFile.print(t);dataFile.println(" usec");
  // if the file is available, write to it:
  if (dataFile) {
    for(int i=0;i<SAMPLES;i++)
    {
      temp = a0[i];
      dataString = String(temp);
      dataString += ";";
      temp = a1[i];
      dataString += String(temp);
      
      dataFile.println(dataString);
    }
  // if the file isn't open, pop up an error:
  }
  else {
    Serial.println("error opening datalog.txt");
  }
  dataFile.close();

  Serial.println("Writing log finished OK!");

//  // if the file is available, write to it:
//  if (dataFile) {
//    dataFile.println(dataString);
//    dataFile.close();
//    // print to the serial port too:
//    Serial.println(dataString);
//  }
//  // if the file isn't open, pop up an error:
//  else {
//    Serial.println("error opening datalog.txt");
//  }

  while(true) {
//    Serial.println("DONE!");
  }

}

