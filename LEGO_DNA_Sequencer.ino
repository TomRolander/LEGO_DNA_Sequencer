#include <EEPROM.h>

/**************************************************************************
  LEGO DNA Sequencer
  Main Program

  Original Code:  2022-12-24

  Tom Rolander, MSEE
  Mentor for Circuit Design, Software, 3D Printing
  Miller Library, Fabrication Lab
  Hopkins Marine Station, Stanford University,
  120 Ocean View Blvd, Pacific Grove, CA 93950
  +1 831.915.9526 | rolander@stanford.edu

 **************************************************************************/

#define PROGRAM F("LEGO DNA Sequencer - Main Program")
#define VERSION F("Ver 0.2 2022-12-29")

#define DEBUG_OUTPUT 1
#define DEBUG_MODE   1

#define FLORA 1

#define STATE_START       0
#define STATE_LOAD_TRAY   1
#define STATE_SEQUENCING  2

static int iState = STATE_START;

static bool bAutomated = true;

#define STEPS_PER_LEGO_BLOCK  550

#define LED_TCS34725  A0

static bool bCommandLineMode = false;

// Rotary Encoder Inputs
#define CLK A1
#define DT A2
#define SW 13

static int iRotaryEncoder_Counter = 0;
static int iRotaryEncoder_CurrentStateCLK;
static int iRotaryEncoder_LastStateCLK;
static bool bRotaryEncoder_CurrentDirectionClockwise;
static unsigned long lastButtonPress = 0;

#include <EEPROM.h>
// EEPROM.write(address, value);
// EEPROM.commit();
// EEPROM.read(address);

// EEPROM state variables
#define EEPROM_SIZE 512
#define EEPROM_SIGNATURE                    0 // 000-003  Signature 'KISS'
#define EEPROM_NUMBER_OF_LOOPS              4 // 004-005  iNumberOfLoops
#define EEPROM_TIME_DELAY_MINUTES           6 // 006-007  iTimeDelayMinutes
#define EEPROM_VIDEO_RECORD_SECONDS         8 // 008-009  iVideoRecordSeconds
#define EEPROM_LIGHT_DELAY_BEFORE_SECONDS  10 // 010-011  iLightDelayBeforeSeconds
#define EEPROM_LIGHT_DELAY_AFTER_SECONDS   12 // 012-013  iLightDelayAfterSeconds

//#define LEGO_DNA_MAX  23
#define LEGO_DNA_MAX  23

int iLEGO_DNA_Number = 6;
char sLEGO_DNA_Sequence[LEGO_DNA_MAX][11] =
{ "ATTGGTCATT",
  "TGCTCCTACA",
  "CACAATCTAC",
  "GCTCCCGGGT",
  "CAAATCTTAG",
  "CGTCTACCAA"
};
char sLEGO_DNA_Name[LEGO_DNA_MAX][17] =
{ "  Mako Shark    ",
  "  Abalone       ",
  "  Loligo Squid  ",
  "  Copepod       ",
  "  Sea Star      ",
  "  Tuna          "
};


#include <LiquidCrystal.h>
int Contrast = 75;
LiquidCrystal lcd(8, 7, 5, 4, 3, 2);

//Includes the Arduino Stepper Library
#include <Stepper.h>

// Defines the number of steps per rotation
const int stepsPerRevolution = 2038;

#define STEPPER_PIN_1 9
#define STEPPER_PIN_2 10
#define STEPPER_PIN_3 11
#define STEPPER_PIN_4 12

#define STEPS_PER_LEGO_BLOCK  550

// Creates an instance of stepper class
// Pins entered in sequence IN1-IN3-IN2-IN4 for proper step sequence
Stepper myStepper = Stepper(stepsPerRevolution, STEPPER_PIN_1, STEPPER_PIN_3, STEPPER_PIN_2, STEPPER_PIN_4);

#include <Wire.h>
#include "Adafruit_TCS34725.h"

#define CLEAR   -1
#define RED     'G'
#define GREEN   'A'
#define BLUE    'T'
#define YELLOW  'C'

/* Initialise with specific int time and gain values */
#define INTEGRATION_TIME_DELAY 50

#if  FLORA
Adafruit_TCS34725 tcs = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_50MS, TCS34725_GAIN_4X);
#else
Adafruit_TCS34725 tcs = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_50MS, TCS34725_GAIN_16X);
#endif

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println();
  Serial.println(PROGRAM);
  Serial.println(VERSION);

  // Set encoder pins as inputs
  pinMode(CLK, INPUT);
  pinMode(DT, INPUT);
  pinMode(SW, INPUT_PULLUP);

  // Read the initial state of CLK
  iRotaryEncoder_LastStateCLK = digitalRead(CLK);

  analogWrite(6, Contrast);
  lcd.begin(16, 2);

  pinMode (LED_TCS34725, OUTPUT);
  digitalWrite (LED_TCS34725, LOW);

  if (tcs.begin()) {
    Serial.println(F("Found sensor"));
    // LED off to prevent overheating
    tcs.setInterrupt(true);

  } else {
    Serial.println(F("No TCS34725 found ... check your connections"));
    while (1);
  }

  myStepper.setSpeed(10);

  char cBuffer[5];
  int nChars;
  long lStartTime = millis();

#if DEBUG_MODE
  bCommandLineMode = true;
  lcd.setCursor(0, 0);
  lcd.print("LEGO DNA Sqncr  ");
  lcd.setCursor(0, 1);
  lcd.print("Debugger Console");
#else
  StartSequencer();
#endif
}

void UpdateLCD(char cColor)
{
  lcd.print(cColor);
}

char GetLEGOColor()
{
  char cRetcode = '\0';

  uint16_t r, g, b, c;

  delay(INTEGRATION_TIME_DELAY);
  tcs.getRawData(&r, &g, &b, &c);
  /*
    r = r/21;
    g = g/21;
    b = b/21;
    c = c/21;
  */
  Serial.print(F("Red: ")); Serial.print(r, DEC); Serial.print(F(" "));
  Serial.print(F("Green: ")); Serial.print(g, DEC); Serial.print(F(" "));
  Serial.print(F("Blue: ")); Serial.print(b, DEC); Serial.print(F(" "));
  Serial.print(F("Color: ")); Serial.print(c, DEC); Serial.print(F(" "));

#if FLORA
  if (c >= 350)
#else
  if (c > 975 /*200*/)
#endif
  {
    UpdateLCD(YELLOW);
    cRetcode = YELLOW;
    Serial.print(F("C Yellow"));
  }
  else if (r >= 190) {
    UpdateLCD(RED);
    cRetcode = RED;
    Serial.print(F("G Red"));
  }
  else if (b >= 270) {
    UpdateLCD(BLUE);
    cRetcode = BLUE;
    Serial.print(F("T Blue"));
  }
  else if (g > b && g > r) {
    UpdateLCD(GREEN);
    cRetcode = GREEN;
    Serial.print(F("A Green"));
  }
  else if (r > g && r > b) {
    UpdateLCD(RED);
    cRetcode = RED;
    Serial.print(F("G Red"));
  }
  else { /*if(b > g)*/
    UpdateLCD(BLUE);
    cRetcode = BLUE;
    Serial.print(F("T Blue"));
  }

  Serial.println(F(""));

  return (cRetcode);
}

void StartSequencer()
{
  lcd.setCursor(0, 0);
  lcd.print("LEGO DNA Sqncr  ");

  lcd.setCursor(0, 1);
  lcd.print(" Push Button    ");
}

void help()
{
  Serial.println(F("Command Line Mode:"));
  Serial.println(F("  Lxxxx to go Left  xxxx steps"));
  Serial.println(F("  Rxxxx to go Right xxxx steps"));
  Serial.println(F("  Sxxxx set RPM to xxxx"));
  Serial.println(F("  C to Read Color Sensor"));
  Serial.println(F("  K Read Color Sensor until Send"));
  Serial.println(F("  1 Color Sensor Light ON"));
  Serial.println(F("  0 Color Sensor Light OFF"));
  Serial.println(F("  M Manual Sequencer"));
  Serial.println(F("  A Automated Sequencer"));
  Serial.println(F("  E to Start Sequencer"));
}

bool GetButtonPressed()
{
  // Read the button state
  int btnState = digitalRead(SW);

  //If we detect LOW signal, button is pressed
  if (btnState == LOW) {
    //if 50ms have passed since last LOW pulse, it means that the
    //button has been pressed, released and pressed again
    if (millis() - lastButtonPress > 50) {
      //      lcd.setCursor(0, 1);
      //      lcd.print("Button pressed! ");
    }

    // Remember last button press event
    lastButtonPress = millis();
    return (true);
  }
  return (false);
}

bool GetYesOrNo(bool bInitialResponse, char *sQuestion)
{
  bool bResponse = bInitialResponse;

  lcd.setCursor(0, 0);
  lcd.print("                ");
  lcd.setCursor(0, 0);
  lcd.print(sQuestion);
  lcd.setCursor(0, 1);
  if (bInitialResponse == false)
    lcd.print("[NO]  YES       ");
  else
    lcd.print(" NO  [YES]      ");

  while (GetButtonPressed() == false)
  {
    delay(1);
    if (CheckRotaryEncoder())
    {
      if (bRotaryEncoder_CurrentDirectionClockwise == false)
      {
        if (bResponse != false)
        {
          lcd.setCursor(0, 1);
          lcd.print("[NO]  YES       ");
        }
        bResponse = false;
      }
      else
      {
        if (bResponse != true)
        {
          lcd.setCursor(0, 1);
          lcd.print(" NO  [YES]      ");
        }
        bResponse = true;
      }
    }
  }
  while (digitalRead(SW) == LOW)
    ;

  return (bResponse);
}

bool CheckRotaryEncoder()
{
  bool bResponse = true;

  // Read the current state of CLK
  iRotaryEncoder_CurrentStateCLK = digitalRead(CLK);

  // If last and current state of CLK are different, then pulse occurred
  // React to only 1 state change to avoid double count
  if (iRotaryEncoder_CurrentStateCLK != iRotaryEncoder_LastStateCLK  && iRotaryEncoder_CurrentStateCLK == 1) {

    // If the DT state is different than the CLK state then
    // the encoder is rotating CCW so decrement
    if (digitalRead(DT) != iRotaryEncoder_CurrentStateCLK) {
      iRotaryEncoder_Counter --;
      bRotaryEncoder_CurrentDirectionClockwise = false;
    } else {
      // Encoder is rotating CW so increment
      iRotaryEncoder_Counter ++;
      bRotaryEncoder_CurrentDirectionClockwise = true;
    }
  }
  else
  {
    bResponse = false;
  }

  // Remember last CLK state
  iRotaryEncoder_LastStateCLK = iRotaryEncoder_CurrentStateCLK;
  return (bResponse);
}

void loop()
{
  if (bCommandLineMode)
  {
    if (Serial.available() > 0)
    {
      char cBuffer[7];
      int nChars;
      int iValue = 0;

      nChars = Serial.readBytes(cBuffer, sizeof(cBuffer));
      Serial.print(F("nChars = "));
      Serial.println(nChars);
      cBuffer[nChars] = '\0';
      Serial.print(F("cBuffer = "));
      Serial.println(cBuffer);

      cBuffer[0] = toupper(cBuffer[0]);

      if (nChars == 1)
      {
        switch (cBuffer[0])
        {
          case 'E':
            StartSequencer();
            iState = STATE_START;
            bCommandLineMode = false;
            return;

          case 'M':
            bAutomated = false;
            break;

          case 'A':
            bAutomated = true;
            break;

          case 'C':
            GetLEGOColor();
            break;

          case 'K':
            while (Serial.available() <= 0)
            {
              GetLEGOColor();
              delay(500);
            }
            nChars = Serial.readBytes(cBuffer, sizeof(cBuffer));
            help();
            break;

          case '1':
            tcs.setInterrupt(false);
            digitalWrite (LED_TCS34725, HIGH);
            break;

          case '0':
            tcs.setInterrupt(true);
            digitalWrite (LED_TCS34725, LOW);
            break;

          default:
            Serial.println(F("Unrecognized command!"));
            help();
            break;
        }
        return;
      }
      if (nChars != 5 ||
          (cBuffer[0] != 'L' && cBuffer[0] != 'R' && cBuffer[0] != 'S'))
      {
        help();
        return;
      }

      for (int i = 1; i < 5; i++)
      {
        iValue = (iValue * 10) + (cBuffer[i] - '0');
      }
      Serial.print(F("Value = "));
      if (cBuffer[0] == 'R')
        iValue = 0 - iValue;
      Serial.println(iValue);

      if (cBuffer[0] == 'S')
        myStepper.setSpeed(iValue);
      else
      {
        myStepper.step(iValue);
        GetLEGOColor();
      }
    }
    return;
  }

  if (iState == STATE_START)
  {
    if (GetButtonPressed())
    {
      while (digitalRead(SW) == LOW)
        ;

      iState = STATE_LOAD_TRAY;
      lcd.setCursor(0, 0);
      lcd.print("Load LEGO Tray  ");

      lcd.setCursor(0, 1);
      lcd.print(" Push Button    ");
    }
    else
    {
      return;
    }
  }

  if (iState == STATE_LOAD_TRAY)
  {
    //    Serial.println(F("STATE_LOAD_TRAY"));
    while (GetButtonPressed() == false)
      ;
    iState = STATE_SEQUENCING;
  }
  //  Serial.println(F("GetButtonPressed()"));

  lcd.setCursor(0, 0);
  lcd.print("Sequencing DNA  ");

  lcd.setCursor(0, 1);
  lcd.print("                ");

  lcd.setCursor(0, 1);


  // STATE_SEQUENCING
  tcs.setInterrupt(false);
  digitalWrite (LED_TCS34725, HIGH);

  myStepper.setSpeed(10);
  //myStepper.step((STEPS_PER_LEGO_BLOCK) + ((3*STEPS_PER_LEGO_BLOCK)/4));
#if FLORA
  myStepper.step(962);
#else
  myStepper.step(1237);
#endif
  char sDNASequence[11];
  sDNASequence[10] = '\0';

  for (int i = 0; i < 10; i++)
  {
    if (bAutomated == false)
    {
      lcd.setCursor(0, 0);
      lcd.print("Position LEGO # ");
      lcd.print(i);
      lcd.setCursor(0, 1);
      lcd.print(" Push Button    ");

      while (GetButtonPressed() == false)
        ;
    }

    delay(500);
    sDNASequence[i] = GetLEGOColor();

    if (i < 9)
      myStepper.step(STEPS_PER_LEGO_BLOCK);
    else
    {
      tcs.setInterrupt(true);
      digitalWrite (LED_TCS34725, LOW);

      lcd.setCursor(0, 0);
      lcd.print("Unloading tray  ");

      myStepper.setSpeed(15);
      //    myStepper.step(0 - ((STEPS_PER_LEGO_BLOCK * 9) + ((STEPS_PER_LEGO_BLOCK) + ((3*STEPS_PER_LEGO_BLOCK)/4))));
#if FLORA
      myStepper.step(-5912);
#else
      myStepper.step(-6187);
#endif
      myStepper.setSpeed(10);
      iState = STATE_LOAD_TRAY;
      int index;
      for (index = 0; index < iLEGO_DNA_Number; index++)
      {
        if (strcmp(sDNASequence, sLEGO_DNA_Sequence[index]) == 0)
          break;
      }

      if (index < iLEGO_DNA_Number)
      {
        lcd.setCursor(0, 0);
        lcd.print("Successful match");
        lcd.setCursor(0, 1);
        lcd.print(sLEGO_DNA_Name[index]);
      }
      else
      {
        lcd.setCursor(0, 0);
        lcd.print("DNA not a match ");
        lcd.setCursor(0, 1);
        lcd.print(" U N K N O W N  ");
      }

      while (GetButtonPressed() == false)
        ;
      while (digitalRead(SW) == LOW)
        ;

      if (index >= iLEGO_DNA_Number)
      {
        if (GetYesOrNo(false, "Add to Database?"))
        {
          Serial.println(F("Add to Database!"));
          lcd.setCursor(0, 0);
          lcd.print("Enter Name      ");
          lcd.setCursor(0, 1);
          lcd.print("                ");

          char sNewName[17] = "";
          char cNextChar;

          for (int i = 0; i < 16; i++)
          {
            cNextChar = GetNextChar(i);
            if (cNextChar == '\0')
            {
              break;
            }
            sNewName[i] = cNextChar;
            sNewName[i + 1] = '\0';
          }
          Serial.println(sNewName);

          if (GetYesOrNo(false, sNewName))
          {
            Serial.print(F("Adding to Database ["));
            Serial.print(sDNASequence);
            Serial.print(F("] = ["));
            Serial.print(sNewName);
            Serial.println(F("]"));

            // Now add to DATABASE
          }
        }
      }
      lcd.setCursor(0, 0);
      lcd.print("Load LEGO Tray  ");
      lcd.setCursor(0, 1);
      lcd.print(" Push Button    ");
    }
  }

}

char GetNextChar(int index)
{
  lcd.setCursor(index, 1);
  lcd.print('_');

  char cCurrentChar = '@';
  int iCurrentCounter = iRotaryEncoder_Counter;
  while (GetButtonPressed() == false)
  {
    if (CheckRotaryEncoder())
    {
      if (iCurrentCounter != iRotaryEncoder_Counter)
      {
        if (iRotaryEncoder_Counter > iCurrentCounter)
        {
          if (cCurrentChar >= 'Z')
          {
            cCurrentChar = '?';
          }
          cCurrentChar = cCurrentChar + (iRotaryEncoder_Counter - iCurrentCounter);
          if (cCurrentChar > 'Z')
            cCurrentChar = '@';
        }
        else
        {
          if (cCurrentChar <= '@')
          {
            cCurrentChar = '[';
          }
          cCurrentChar = cCurrentChar - (iCurrentCounter - iRotaryEncoder_Counter);
          if (cCurrentChar < '@')
            cCurrentChar = 'Z';
        }
        lcd.setCursor(index, 1);
        lcd.print(cCurrentChar);
        iCurrentCounter = iRotaryEncoder_Counter;
      }
    }
  }
  while (digitalRead(SW) == LOW)
    ;
  if (cCurrentChar == '@')
    cCurrentChar = '\0';
  return (cCurrentChar);
}
