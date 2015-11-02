#include <digitalWriteFast.h>
#include <SimpleModbusSlave_DUE.h>

/*
//---------------------------------------
// Half step mode
//  Cycle |  A(+)  A(-)  B(+)  B(-)  DEC
//--------+------------------------------
//  A0    |  1     0     0     0     8
//  B1    |  1     1     0     0     12
//  C2    |  0     1     0     0     4
//  D3    |  0     1     1     0     6
//  E4    |  0     0     1     0     2
//  F5    |  0     0     1     1     3
//  G6    |  0     0     0     1     1
//  H7    |  1     0     0     1     9


//---------------------------------------
// Full step mode double torque
//  Cycle |  A(+)  A(-)  B(+)  B(-)  DEC
//--------+------------------------------
//  B1    |  1     1     0     0     12
//  D3    |  0     1     1     0     6
//  F5    |  0     0     1     1     3
//  H7    |  1     0     0     1     9

//---------------------------------------
// Full step mode single torque
//  Cycle |  A(+)  A(-)  B(+)  B(-)  DEC
//--------+------------------------------
//  A0    |  1     0     0     0     8
//  C2    |  0     1     0     0     4
//  E4    |  0     0     1     0     2
//  G6    |  0     0     0     1     1

*/

#define MOTOR_MODE_HALF_STEP                0
#define MOTOR_MODE_FULL_STEP_DOUBLE_TORQUE  1
#define MOTOR_MODE_FULL_STEP_SINGLE_TORQUE  2



#define STEPPER_MOTOR_DIR_CW   1
#define STEPPER_MOTOR_DIR_CCW  0

#define STEPPER_MOTOR_POS_A    8
#define STEPPER_MOTOR_POS_B    12
#define STEPPER_MOTOR_POS_C    4
#define STEPPER_MOTOR_POS_D    6
#define STEPPER_MOTOR_POS_E    2
#define STEPPER_MOTOR_POS_F    3
#define STEPPER_MOTOR_POS_G    1
#define STEPPER_MOTOR_POS_H    9


#define PIN_CHANNEL_A_POS      3
#define PIN_CHANNEL_A_NEG      4
#define PIN_CHANNEL_B_POS      5
#define PIN_CHANNEL_B_NEG      6

#define PIN_SIM_CHANNEL_A_POS  7
#define PIN_SIM_CHANNEL_A_NEG  8
#define PIN_SIM_CHANNEL_B_POS  9
#define PIN_SIM_CHANNEL_B_NEG  10


//-------------------- MODBUS & SERIAL SETTINGS----------------------------

#define SERIAL_BAURDRATE            19200
#define MODBUS_ADDRESS              1
#define MODBUS_TX_ENABLE_PIN        12


//-------------------- MODBUS & SERIAL SETTINGS----------------------------
//////////////// registers of your slave ///////////////////
enum
{
  // just add or remove registers and your good to go...
  // The first register starts at address 0
  VAL0,
  VAL1,
  VAL2,
  VAL3,
  VAL4,
  VAL5,
  VAL6,
  VAL7,
  VAL8,
  VAL9,
  HOLDING_REGS_SIZE // leave this one
  // total number of registers for function 3 and 16 share the same register array
  // i.e. the same address space
};

unsigned int holdingRegs[30]; // function 3 and 16 register array


#define PIN_LED                13

// Variables will change:
long stepCounter = 0;

char steppingPos = 0;        // A+ A- B+ B-
char channelStatus = 0;
char steppingPosPrev = 0;

char chAPosState = 0;        // current state
char chANegState = 0;
char chBPosState = 0;
char chBNegState = 0;


// stepper motor pulse out sim
#define ONE_MSEC_COUNT_NO           4
int one_msecCounter = 0;
int stepperSpeed = 10;
bool stepperDir = STEPPER_MOTOR_DIR_CW;
int phaseCounter = 0;
char steppingPosOut = 0;

char motorWorkingMode = MOTOR_MODE_HALF_STEP;

void setup() {
  // initialize the button pin as a input:
  pinMode(PIN_CHANNEL_A_POS, INPUT);
  pinMode(PIN_CHANNEL_A_NEG, INPUT);
  pinMode(PIN_CHANNEL_B_POS, INPUT);
  pinMode(PIN_CHANNEL_B_NEG, INPUT);
  // initialize the LED as an output:
  pinMode(PIN_LED, OUTPUT);
  // initialize serial communication:
  //  Serial.begin(19200);

  modbus_configure(&Serial, SERIAL_BAURDRATE, MODBUS_ADDRESS, MODBUS_TX_ENABLE_PIN,
                   HOLDING_REGS_SIZE, holdingRegs);

  // modbus_update_comms(baud, id) is not needed but allows for easy update of the
  // port variables and slave id dynamically in any function.
  modbus_update_comms(SERIAL_BAURDRATE, MODBUS_ADDRESS);






  //-------------------- stepper pulse SETTINGS------------------------------------
  pinMode(PIN_SIM_CHANNEL_A_POS, OUTPUT);
  pinMode(PIN_SIM_CHANNEL_A_NEG, OUTPUT);
  pinMode(PIN_SIM_CHANNEL_B_POS, OUTPUT);
  pinMode(PIN_SIM_CHANNEL_B_NEG, OUTPUT);

  cli();//stop interrupts
  //set timer2 interrupt at 8kHz
  TCCR2A = 0;// set entire TCCR2A register to 0
  TCCR2B = 0;// same for TCCR2B
  TCNT2  = 0;//initialize counter value to 0
  // set compare match register for 8khz increments
  OCR2A = 249;// = (16*10^6) / (8000*8) - 1 (must be <256)
  // set compare match register for 816khz increments
  //  OCR2A = 124;//16000000/(8*timer2Freq) - 1; //124;// = (16*10^6) / (16000*8) - 1 (must be <256)
  // OCR2A = 24;//16000000/(8*80000) - 1; //124;// = (16*10^6) / (16000*8) - 1 (must be <256)
  // turn on CTC mode
  TCCR2A |= (1 << WGM21);
  // Set CS21 bit for 8 prescaler
  TCCR2B |= (1 << CS21);
  // enable timer compare interrupt
  TIMSK2 |= (1 << OCIE2A);


  sei();//allow interrupts
  //-------------------- ENCODER SETTINGS------------------------------------
}

inline char phase2channels(char phase)
{
  switch (phase)
  {
    case 0:
      return STEPPER_MOTOR_POS_A;
      break;
    case 1:
      return STEPPER_MOTOR_POS_B;
      break;
    case 2:
      return STEPPER_MOTOR_POS_C;
      break;
    case 3:
      return STEPPER_MOTOR_POS_D;
      break;
    case 4:
      return STEPPER_MOTOR_POS_E;
      break;
    case 5:
      return STEPPER_MOTOR_POS_F;
      break;
    case 6:
      return STEPPER_MOTOR_POS_G;
      break;
    case 7:
      return STEPPER_MOTOR_POS_H;
      break;
    default:
      break;
  }
}

//-------------------------------------------------------------------
ISR(TIMER2_COMPA_vect) {

  one_msecCounter++;
  if (one_msecCounter >= stepperSpeed) {
    one_msecCounter = 0;

    if (stepperDir == STEPPER_MOTOR_DIR_CW)
    {
      phaseCounter++;  // phaseCounter +=2; for full step mode
      if (phaseCounter >= 8)
        phaseCounter = 0;
      digitalWriteFast(PIN_LED, HIGH);
    }
    else
    {
      phaseCounter--;  // phaseCounter -=2; for full step mode
      if (phaseCounter < 0)
        phaseCounter = 7;
      digitalWriteFast(PIN_LED, LOW);
    }
  }

  steppingPosOut = phase2channels(phaseCounter);
  digitalWriteFast(PIN_SIM_CHANNEL_A_POS, (steppingPosOut >> 3) & 0x01);
  digitalWriteFast(PIN_SIM_CHANNEL_A_NEG, (steppingPosOut >> 2) & 0x01);
  digitalWriteFast(PIN_SIM_CHANNEL_B_POS, (steppingPosOut >> 1) & 0x01);
  digitalWriteFast(PIN_SIM_CHANNEL_B_NEG, (steppingPosOut >> 0) & 0x01);

}


inline char channels2position(char channels)
{
  switch (channels)
  {
    case STEPPER_MOTOR_POS_A:
      return 0;
      break;
    case STEPPER_MOTOR_POS_B:
      return 1;
      break;
    case STEPPER_MOTOR_POS_C:
      return 2;
      break;
    case STEPPER_MOTOR_POS_D:
      return 3;
      break;
    case STEPPER_MOTOR_POS_E:
      return 4;
      break;
    case STEPPER_MOTOR_POS_F:
      return 5;
      break;
    case STEPPER_MOTOR_POS_G:
      return 6;
      break;
    case STEPPER_MOTOR_POS_H:
      return 7;
      break;
    default:
      return 0;
      break;
  }
}

//-------------------------------------------------------------------
//----------------------------CH A-----------------------------------
//----------------------------MAIN-----------------------------------
void loop()
{
  modbus_update();
  stepperSpeed = abs((int)holdingRegs[VAL0]);
  if ((int)holdingRegs[VAL0] >= 0)
    stepperDir = STEPPER_MOTOR_DIR_CW;
  else
    stepperDir = STEPPER_MOTOR_DIR_CCW;

  chAPosState = digitalReadFast(PIN_CHANNEL_A_POS);
  chANegState = digitalReadFast(PIN_CHANNEL_A_NEG);
  chBPosState = digitalReadFast(PIN_CHANNEL_B_POS);
  chBNegState = digitalReadFast(PIN_CHANNEL_B_NEG);


  // translate position code to incremental number
  channelStatus = (chAPosState << 3) | (chANegState << 2) | (chBPosState << 1) | chBNegState;
  steppingPos = channels2position(channelStatus);

  // Half step mode
  if (motorWorkingMode == MOTOR_MODE_HALF_STEP)
  {
    else if ((steppingPos > steppingPosPrev) || ((steppingPosPrev == 7) && (steppingPos == 0))) // rotating CW
    {
      stepCounter++;
      //    if(stepCounter >= 32767)  // go MAX
      //      stepCounter = 32767;
    }
    else if ((steppingPos < steppingPosPrev) || ((steppingPosPrev == 0) && (steppingPos == 7))) // rotating CCW
    {
      stepCounter--;
      //    if(stepCounter <= -32767)  // go MIN
      //      stepCounter = -32767;
    }
    else
    {}
  }
  if (motorWorkingMode == MOTOR_MODE_FULL_STEP_DOUBLE_TORQUE)
  {
    if ((steppingPos > steppingPosPrev) || ((steppingPosPrev == 7) && (steppingPos == 1))) // rotating CW
    {
      stepCounter++;
    }
    else if ((steppingPos < steppingPosPrev) || ((steppingPosPrev == 1) && (steppingPos == 7))) // rotating CCW
    {
      stepCounter--;
    }
    else
    {}
  }
  if (motorWorkingMode == MOTOR_MODE_FULL_STEP_SINGLE_TORQUE)
  {
    if ((steppingPos > steppingPosPrev) || ((steppingPosPrev == 6) && (steppingPos == 0))) // rotating CW
    {
      stepCounter++;
    }
    else if ((steppingPos < steppingPosPrev) || ((steppingPosPrev == 0) && (steppingPos == 6))) // rotating CCW
    {
      stepCounter--;
    }
    else
    {}
  }
  else
  {}

  holdingRegs[VAL1] = stepCounter;
  holdingRegs[VAL2] = steppingPos;
  holdingRegs[VAL3] = steppingPosPrev;
  holdingRegs[VAL4] = channelStatus;
  holdingRegs[VAL5] = chAPosState;
  holdingRegs[VAL6] = chANegState;
  holdingRegs[VAL7] = chBPosState;
  holdingRegs[VAL8] = chBNegState;

  steppingPosPrev = steppingPos;
  //  Serial.println(stepCounter);


}
//-------------------------------------------------------------------
//------------------------END-MAIN-----------------------------------


