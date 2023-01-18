/**************************************************************************
  LEGO DNA Sequencer
  Stepper Motor Unit Test
  
  Original Code:  2022-12-24

  Tom Rolander, MSEE
  Mentor for Circuit Design, Software, 3D Printing
  Miller Library, Fabrication Lab
  Hopkins Marine Station, Stanford University,
  120 Ocean View Blvd, Pacific Grove, CA 93950
  +1 831.915.9526 | rolander@stanford.edu

 **************************************************************************/

#define PROGRAM "LEGO DNA Sequencer - Stepper Motor Unit Test"
#define VERSION "Ver 0.1 2022-12-24"

#define DEBUG_OUTPUT 1


//Includes the Arduino Stepper Library
#include <Stepper.h>

// Defines the number of steps per rotation
const int stepsPerRevolution = 2038;

#define STEPPER_PIN_1 9
#define STEPPER_PIN_2 10
#define STEPPER_PIN_3 11
#define STEPPER_PIN_4 12

// Creates an instance of stepper class
// Pins entered in sequence IN1-IN3-IN2-IN4 for proper step sequence
Stepper myStepper = Stepper(stepsPerRevolution, STEPPER_PIN_1, STEPPER_PIN_3, STEPPER_PIN_2, STEPPER_PIN_4);

static int iSteps = 0;

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println();
  Serial.println(PROGRAM);
  Serial.println(VERSION);

  myStepper.setSpeed(10);

  Serial.println("Hit debugger Send to enter Commands");
  help();  
}

void help()
{
  Serial.println("Command Line Mode:");
  Serial.println("  Fxxxx to go Forward  xxxx steps");
  Serial.println("  Rxxxx to go Reverse xxxx steps");
}

void loop()
{
  if (Serial.available() > 0)
  {
    char cBuffer[7];
    int nChars;
    int iValue = 0;

    nChars = Serial.readBytes(cBuffer, sizeof(cBuffer));
    cBuffer[nChars] = '\0';
    Serial.println(cBuffer);
    cBuffer[0] = toupper(cBuffer[0]);
    if (nChars != 5 ||
        (cBuffer[0] != 'F' && cBuffer[0] != 'R'))
    {
      help();
      return;
    }

    for (int i=1; i<5; i++)
    {
      iValue = (iValue * 10) + (cBuffer[i]-'0');
    }
    if (cBuffer[0] == 'R')
      iValue = 0 - iValue;

    iSteps = iSteps + iValue;
    myStepper.step(iValue);
    Serial.print("Position = ");
    Serial.println(iSteps);
  }
}
