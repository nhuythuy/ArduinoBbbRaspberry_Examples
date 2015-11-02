#include <SPI.h>
#include <Ethernet.h>
#include "MgsModbus.h"


MgsModbus Mb;


#define ENABLE_CHANNEL_0
#define ENABLE_CHANNEL_1
#define ENABLE_CHANNEL_2

//---------------------SERIAL SETTINGS-------------------------------------

#define SERIAL_BAURDRATE            19200


//-------------------- ENCODER SETTINGS------------------------------------
#define TIMER2_FREQ 16000


#ifdef ENABLE_CHANNEL_0
#define ENC0_PIN_A 2
#define ENC0_PIN_B 3
#endif

#ifdef ENABLE_CHANNEL_1
#define ENC1_PIN_A A5
#define ENC1_PIN_B 5
#endif

#ifdef ENABLE_CHANNEL_2
#define ENC2_PIN_A 6
#define ENC2_PIN_B 7
#endif

#define ENCODER_DEFAULT_REV_PULSES   1024
#define ENCODER_DIRECTION_CW         0
#define ENCODER_DIRECTION_CCW        1

#define FAILURE_MODE_NO              0
#define FAILURE_MODE_FREEZE_A_HIGH   1
#define FAILURE_MODE_FREEZE_A_LOW    2
#define FAILURE_MODE_FREEZE_B_HIGH   3
#define FAILURE_MODE_FREEZE_B_LOW    4
#define FAILURE_MODE_FREEZE_AB_HIGH  5
#define FAILURE_MODE_FREEZE_AB_LOW   6

boolean toggleX = 0;

boolean chAState = 0;
boolean chBState = 0;

//------------------------ Channel 0 ----------------------------
#ifdef ENABLE_CHANNEL_0
bool enc0SwDisable = false;
int setFreq0 = 8000; // 8000Hz
long setFreq0Count = 0;
long freq0Counter = 0;
int pos0Counter = 0;
int rev0Pulses = ENCODER_DEFAULT_REV_PULSES;
int rev0Counter = 0;
boolean ch0APreState = 0;

char counter0Phase = 0;
char enc0Dir = ENCODER_DIRECTION_CW;
char failure0Mode = FAILURE_MODE_NO;
#endif
//------------------------ Channel 0 ----------------------------

//------------------------ Channel 1 ----------------------------
#ifdef ENABLE_CHANNEL_1
bool enc1SwDisable = false;
int setFreq1 = 8000; // 8000Hz
long setFreq1Count = 0;
long freq1Counter = 0;
int pos1Counter = 0;
int rev1Pulses = ENCODER_DEFAULT_REV_PULSES;
int rev1Counter = 0;
boolean ch1APreState = 0;

char counter1Phase = 0;
char enc1Dir = ENCODER_DIRECTION_CW;
char failure1Mode = FAILURE_MODE_NO;
#endif
//------------------------ Channel 1 ----------------------------

//------------------------ Channel 2 ----------------------------
#ifdef ENABLE_CHANNEL_2
bool enc2SwDisable = false;
int setFreq2 = 8000; // 16000Hz
long setFreq2Count = 0;
long freq2Counter = 0;
int pos2Counter = 0;
int rev2Pulses = ENCODER_DEFAULT_REV_PULSES;
int rev2Counter = 0;
boolean ch2APreState = 0;

char counter2Phase = 0;
char enc2Dir = ENCODER_DIRECTION_CW;
char failure2Mode = FAILURE_MODE_NO;
#endif
//------------------------ Channel 2 ----------------------------

//-------------------- ENCODER SETTINGS------------------------------------


#define  LEDX 13

// Using the enum instruction allows for an easy method for adding and
// removing registers. Doing it this way saves you #defining the size
// of your slaves register array each time you want to add more registers
// and at a glimpse informs you of your slaves register layout.

//////////////// registers of your slave ///////////////////
enum
{
  // just add or remove registers and your good to go...
  // The first register starts at address 0
  ENC0_FAILURE_MODE,
  ENC0_PULSE_WIDTH,
  ENC0_PULSE_COUNT,
  ENC0_REV_PULSES,
  ENC1_FAILURE_MODE,
  ENC1_PULSE_WIDTH,
  ENC1_PULSE_COUNT,
  ENC1_REV_PULSES,
  ENC2_FAILURE_MODE,
  ENC2_PULSE_WIDTH,
  ENC2_PULSE_COUNT,
  ENC2_REV_PULSES,
  ENC3_FAILURE_MODE,
  ENC3_PULSE_WIDTH,
  ENC3_PULSE_COUNT,
  ENC3_REV_PULSES,
  HOLDING_REGS_SIZE // leave this one
  // total number of registers for function 3 and 16 share the same register array
  // i.e. the same address space
};


bool ledState = false;

void setup()
{
  // Serial setup
  Serial.begin(SERIAL_BAURDRATE);

  // TCP setup
  uint8_t mac[]     = { 0x90, 0xA2, 0xDA, 0x0F, 0xE4, 0xE5 };
  uint8_t ip[]      = { 10, 0, 0, 90 };
//  uint8_t gateway[] = { 192, 168, 1, 1 };
//  uint8_t subnet[]  = { 255, 255, 255, 0 };
  Ethernet.begin(mac, ip);//, gateway, subnet);
  //Avoid pins 4,10,11,12,13 when using ethernet shield

  // Print your local IP address:
  Serial.print("My IP address: ");
  for (byte idx = 0; idx < 4; idx++) {
    Serial.print(ip[idx]);
    Serial.print(".");
  }
  Serial.println();

  // print menu
  Serial.println("0 - print the first 5 words of the MbData space");
  Serial.println("1 - FC 1 - read the first 5 coils from the slave and store them in the lower byte of MbData[1]");
  Serial.println("2 - FC 2 - read the first 5 discrete inputs from the slave and store them in the higer of the MbData[1]");
  Serial.println("3 - FC 3 - read the first 5 registers from the slave and store them in MbData[3..7");
  Serial.println("4 - FC 4 - read the first 5 input registers from the slave and store them in MbData[8..12]");
  Serial.println("5 - FC 5 - write coil 0 of the slave with the bit valeu ot MbData[0.0]");
  Serial.println("6 - FC 6 - write register 0 of the slave with MbData[2]");
  Serial.println("7 - FC 15 - write 5 coils of the slave starting with coil 0 with GetBit(16..20");
  Serial.println("8 - Fc 16 - weite 5 registers of the slave starting on register 0 with MbData[0..4]");

  //-------------------- ENCODER SETTINGS------------------------------------
  pinMode(LEDX, OUTPUT);
#ifdef ENABLE_CHANNEL_0
  setFreq0Count = TIMER2_FREQ / setFreq0;
#endif
#ifdef ENABLE_CHANNEL_1
  setFreq1Count = TIMER2_FREQ / setFreq1;
#endif
#ifdef ENABLE_CHANNEL_2
  setFreq2Count = TIMER2_FREQ / setFreq2;
#endif
#ifdef ENABLE_CHANNEL_3
  setFreq3Count = TIMER2_FREQ / setFreq3;
#endif
  //set pins as outputs
#ifdef ENABLE_CHANNEL_0
  pinMode(ENC0_PIN_A, OUTPUT);  // 0A
  pinMode(ENC0_PIN_B, OUTPUT);  // 0B
#endif
#ifdef ENABLE_CHANNEL_1
  pinMode(ENC1_PIN_A, OUTPUT);  // 1A
  pinMode(ENC1_PIN_B, OUTPUT);  // 1B
#endif
#ifdef ENABLE_CHANNEL_2
  pinMode(ENC2_PIN_A, OUTPUT);  // 2A
  pinMode(ENC2_PIN_B, OUTPUT);  // 2B
#endif


  pinMode(LEDX, OUTPUT);
#ifdef ENABLE_CHANNEL_0
  setFreq0Count = TIMER2_FREQ / setFreq0;
#endif
#ifdef ENABLE_CHANNEL_1
  setFreq1Count = TIMER2_FREQ / setFreq1;
#endif
#ifdef ENABLE_CHANNEL_2
  setFreq2Count = TIMER2_FREQ / setFreq2;
#endif
  //set pins as outputs
#ifdef ENABLE_CHANNEL_0
  pinMode(ENC0_PIN_A, OUTPUT);  // 0A
  pinMode(ENC0_PIN_B, OUTPUT);  // 0B
#endif
#ifdef ENABLE_CHANNEL_1
  pinMode(ENC1_PIN_A, OUTPUT);  // 1A
  pinMode(ENC1_PIN_B, OUTPUT);  // 1B
#endif
#ifdef ENABLE_CHANNEL_2
  pinMode(ENC2_PIN_A, OUTPUT);  // 2A
  pinMode(ENC2_PIN_B, OUTPUT);  // 2B
#endif

  ///////////////////////////////////////////////////////////////////////////
  //-------------------- TIMER INTERRUPT SETTINGS----------------------------
  // turn on the timer clock in the power management controller
  pmc_set_writeprotect(false);		 // disable write protection for pmc registers
  pmc_enable_periph_clk(ID_TC7);	 // enable peripheral clock TC7

  // we want wavesel 01 with RC
  TC_Configure(/* clock */TC2,/* channel */1, TC_CMR_WAVE | TC_CMR_WAVSEL_UP_RC | TC_CMR_TCCLKS_TIMER_CLOCK4);
  TC_SetRC(TC2, 1, 10); //164.0625 ~ 0.25msec 131200);
  TC_Start(TC2, 1);

  // enable timer interrupts on the timer
  TC2->TC_CHANNEL[1].TC_IER = TC_IER_CPCS; // IER = interrupt enable register
  TC2->TC_CHANNEL[1].TC_IDR = ~TC_IER_CPCS; // IDR = interrupt disable register

  // Enable the interrupt in the nested vector interrupt controller
  // TC4_IRQn where 4 is the timer number * timer channels (3) + the channel number (=(1*3)+1) for timer1 channel1
  NVIC_EnableIRQ(TC7_IRQn);
  //-------------------- TIMER INTERRUPT SETTINGS----------------------------

  //-------------------- ENCODER SETTINGS------------------------------------
}


inline void enc0Run()
{
  freq0Counter = freq0Counter + 1;
  if (freq0Counter >= setFreq0Count)
  {
    freq0Counter = 0;

//    if (failure0Mode == 0)
//      if (pos0Counter <= 0) {
//        pos0Counter = 0;      // to be sure, not round up after -37767
//        goto next0;
//      }

    if (enc0Dir == ENCODER_DIRECTION_CW)
    {
      counter0Phase++;
      if (counter0Phase >= 5)
        counter0Phase = 1;
    }
    if (enc0Dir == ENCODER_DIRECTION_CCW)
    {
      counter0Phase--;
      if (counter0Phase <= 0)
        counter0Phase = 4;
    }

    if (counter0Phase == 1) {
      chAState = 1;
      chBState = 0;
    }
    if (counter0Phase == 2) {
      chAState = 1;
      chBState = 1;
    }
    if (counter0Phase == 3) {
      chAState = 0;
      chBState = 1;
    }
    if (counter0Phase == 4) {
      chAState = 0;
      chBState = 0;
    }

    switch (failure0Mode)
    {
      case FAILURE_MODE_NO:
        digitalWrite(ENC0_PIN_A, chAState);
        digitalWrite(ENC0_PIN_B, chBState);
        break;
      case FAILURE_MODE_FREEZE_A_HIGH:
        digitalWrite(ENC0_PIN_A, HIGH);
        digitalWrite(ENC0_PIN_B, chBState);
        break;
      case FAILURE_MODE_FREEZE_A_LOW:
        digitalWrite(ENC0_PIN_A, LOW);
        digitalWrite(ENC0_PIN_B, chBState);
        break;
      case FAILURE_MODE_FREEZE_B_HIGH:
        digitalWrite(ENC0_PIN_A, chAState);
        digitalWrite(ENC0_PIN_B, HIGH);
        break;
      case FAILURE_MODE_FREEZE_B_LOW:
        digitalWrite(ENC0_PIN_A, chAState);
        digitalWrite(ENC0_PIN_B, LOW);
        break;
      case FAILURE_MODE_FREEZE_AB_HIGH:
        digitalWrite(ENC0_PIN_A, HIGH);
        digitalWrite(ENC0_PIN_B, HIGH);
        break;
      case FAILURE_MODE_FREEZE_AB_LOW:
        digitalWrite(ENC0_PIN_A, LOW);
        digitalWrite(ENC0_PIN_B, LOW);
        break;
      default:
        digitalWrite(ENC0_PIN_A, chAState);
        digitalWrite(ENC0_PIN_B, chBState);
    }

next0:;
  }
}

inline void enc1Run()
{
  freq1Counter = freq1Counter + 1;
  if (freq1Counter >= setFreq1Count)
  {
    freq1Counter = 0;

//    if (failure1Mode == 0)
//      if (pos1Counter <= 0) {
//        pos1Counter = 0;
//        goto next1;
//      }

    if (enc1Dir == ENCODER_DIRECTION_CW)
    {
      counter1Phase = counter1Phase + 1;
      if (counter1Phase >= 5)
        counter1Phase = 1;
    }
    if (enc1Dir == ENCODER_DIRECTION_CCW)
    {
      counter1Phase = counter1Phase - 1;
      if (counter1Phase <= 0)
        counter1Phase = 4;
    }

    if (counter1Phase == 1) {
      chAState = 1;
      chBState = 0;
    }
    if (counter1Phase == 2) {
      chAState = 1;
      chBState = 1;
    }
    if (counter1Phase == 3) {
      chAState = 0;
      chBState = 1;
    }
    if (counter1Phase == 4) {
      chAState = 0;
      chBState = 0;
    }

    switch (failure1Mode)
    {
      case FAILURE_MODE_NO:
        digitalWrite(ENC1_PIN_A, chAState);
        digitalWrite(ENC1_PIN_B, chBState);
        break;
      case FAILURE_MODE_FREEZE_A_HIGH:
        digitalWrite(ENC1_PIN_A, HIGH);
        digitalWrite(ENC1_PIN_B, chBState);
        break;
      case FAILURE_MODE_FREEZE_A_LOW:
        digitalWrite(ENC1_PIN_A, LOW);
        digitalWrite(ENC1_PIN_B, chBState);
        break;
      case FAILURE_MODE_FREEZE_B_HIGH:
        digitalWrite(ENC1_PIN_A, chAState);
        digitalWrite(ENC1_PIN_B, HIGH);
        break;
      case FAILURE_MODE_FREEZE_B_LOW:
        digitalWrite(ENC1_PIN_A, chAState);
        digitalWrite(ENC1_PIN_B, LOW);
        break;
      case FAILURE_MODE_FREEZE_AB_HIGH:
        digitalWrite(ENC1_PIN_A, HIGH);
        digitalWrite(ENC1_PIN_B, HIGH);
        break;
      case FAILURE_MODE_FREEZE_AB_LOW:
        digitalWrite(ENC1_PIN_A, LOW);
        digitalWrite(ENC1_PIN_B, LOW);
        break;
      default:
        digitalWrite(ENC1_PIN_A, chAState);
        digitalWrite(ENC1_PIN_B, chBState);
    }

next1:;
  }
}

inline void enc2Run()
{
  freq2Counter = freq2Counter + 1;
  if (freq2Counter >= setFreq2Count)
  {
    freq2Counter = 0;

//    if (failure2Mode == 0)
//      if (pos2Counter <= 0) {
//        pos2Counter = 0;
//        goto next2;
//      }

    if (enc2Dir == ENCODER_DIRECTION_CW)
    {
      counter2Phase = counter2Phase + 1;
      if (counter2Phase >= 5)
        counter2Phase = 1;
    }
    if (enc2Dir == ENCODER_DIRECTION_CCW)
    {
      counter2Phase = counter2Phase - 1;
      if (counter2Phase <= 0)
        counter2Phase = 4;
    }

    if (counter2Phase == 1) {
      chAState = 1;
      chBState = 0;
    }
    if (counter2Phase == 2) {
      chAState = 1;
      chBState = 1;
    }
    if (counter2Phase == 3) {
      chAState = 0;
      chBState = 1;
    }
    if (counter2Phase == 4) {
      chAState = 0;
      chBState = 0;
    }

    switch (failure2Mode)
    {
      case FAILURE_MODE_NO:
        digitalWrite(ENC2_PIN_A, chAState);
        digitalWrite(ENC2_PIN_B, chBState);
        break;
      case FAILURE_MODE_FREEZE_A_HIGH:
        digitalWrite(ENC2_PIN_A, HIGH);
        digitalWrite(ENC2_PIN_B, chBState);
        break;
      case FAILURE_MODE_FREEZE_A_LOW:
        digitalWrite(ENC2_PIN_A, LOW);
        digitalWrite(ENC2_PIN_B, chBState);
        break;
      case FAILURE_MODE_FREEZE_B_HIGH:
        digitalWrite(ENC2_PIN_A, chAState);
        digitalWrite(ENC2_PIN_B, HIGH);
        break;
      case FAILURE_MODE_FREEZE_B_LOW:
        digitalWrite(ENC2_PIN_A, chAState);
        digitalWrite(ENC2_PIN_B, LOW);
        break;
      case FAILURE_MODE_FREEZE_AB_HIGH:
        digitalWrite(ENC2_PIN_A, HIGH);
        digitalWrite(ENC2_PIN_B, HIGH);
        break;
      case FAILURE_MODE_FREEZE_AB_LOW:
        digitalWrite(ENC2_PIN_A, LOW);
        digitalWrite(ENC2_PIN_B, LOW);
        break;
      default:
        digitalWrite(ENC2_PIN_A, chAState);
        digitalWrite(ENC2_PIN_B, chBState);
    }

next2:;
  }
}



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// INTERRUPT HANDLERS
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void TC7_Handler()
{
  // We need to get the status to clear it and allow the interrupt to fire again
  TC_GetStatus(TC2, 1);
//  ledState = !ledState;
//  digitalWrite(LEDX, ledState);



  //------------------------ Channel 0 ----------------------------
#ifdef ENABLE_CHANNEL_0
  enc0Run();
#endif
  //------------------------ END Channel 0 ----------------------------

  //------------------------ Channel 1 ----------------------------
#ifdef ENABLE_CHANNEL_1
  enc1Run();
#endif
  //------------------------ END Channel 1 ----------------------------

  //------------------------ Channel 2 ----------------------------
#ifdef ENABLE_CHANNEL_2
  enc1Run();
#endif
  //------------------------ END Channel 2 ----------------------------

}


//-------------------------------------------------------------------
//----------------------------MAIN-----------------------------------
void loop()
{
  Mb.MbsRun();

  
  //-----------------------------------------------------
#ifdef ENABLE_CHANNEL_0
if(!enc0SwDisable)
{
  failure0Mode = (char)Mb.MbData[ENC0_FAILURE_MODE];
  setFreq0Count = Mb.MbData[ENC0_PULSE_WIDTH];
  //  setFreq0Count = 4 * Mb.MbData[ENC0_PULSE_WIDTH]; // (msec) 0.25 msec (4kHz) for one base pulse
  if (Mb.MbData[ENC0_PULSE_COUNT] > 32767)
    enc0Dir = ENCODER_DIRECTION_CCW;
  else
    enc0Dir = ENCODER_DIRECTION_CW;
  pos0Counter = abs((int)Mb.MbData[ENC0_PULSE_COUNT]);
  rev0Pulses = Mb.MbData[ENC0_REV_PULSES];

//  digitalWrite(13, enc0Dir);
}
#endif
  //-----------------------------------------------------
#ifdef ENABLE_CHANNEL_1
if(!enc1SwDisable)
{
  failure1Mode = (char)Mb.MbData[ENC1_FAILURE_MODE];
  setFreq1Count = Mb.MbData[ENC1_PULSE_WIDTH];
  if (Mb.MbData[ENC1_PULSE_COUNT] > 32767)
    enc1Dir = ENCODER_DIRECTION_CCW;
  else
    enc1Dir = ENCODER_DIRECTION_CW;
  pos1Counter = abs((int)Mb.MbData[ENC1_PULSE_COUNT]);
  rev1Pulses = Mb.MbData[ENC1_REV_PULSES];
}
#endif
  //-----------------------------------------------------
#ifdef ENABLE_CHANNEL_2
if(!enc2SwDisable)
{
  failure2Mode = (char)Mb.MbData[ENC2_FAILURE_MODE];
  setFreq2Count = Mb.MbData[ENC2_PULSE_WIDTH];
  if (Mb.MbData[ENC2_PULSE_COUNT] > 32767)
    enc2Dir = ENCODER_DIRECTION_CCW;
  else
    enc2Dir = ENCODER_DIRECTION_CW;
  pos2Counter = abs((int)Mb.MbData[ENC2_PULSE_COUNT]);
  rev2Pulses = Mb.MbData[ENC2_REV_PULSES];
}
#endif
  //-----------------------------------------------------

}

