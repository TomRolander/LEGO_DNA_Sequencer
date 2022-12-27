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

#define PROGRAM "LEGO DNA Sequencer - Main Program"
#define VERSION "Ver 0.1 2022-12-24"

#define DEBUG_OUTPUT 1

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

int counter = 0;
int currentStateCLK;
int lastStateCLK;
String currentDir ="";
unsigned long lastButtonPress = 0;

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
#define RED     3
#define GREEN   2
#define BLUE    1
#define YELLOW  0

/* Initialise with specific int time and gain values */
#define INTEGRATION_TIME_DELAY 50

Adafruit_TCS34725 tcs = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_50MS, TCS34725_GAIN_16X);


void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println();
  Serial.println(PROGRAM);
  Serial.println(VERSION);

  // Set encoder pins as inputs
  pinMode(CLK,INPUT);
  pinMode(DT,INPUT);
  pinMode(SW, INPUT_PULLUP);

  // Read the initial state of CLK
  lastStateCLK = digitalRead(CLK);

  analogWrite(6, Contrast);
  lcd.begin(16, 2);

  pinMode (LED_TCS34725, OUTPUT);
  digitalWrite (LED_TCS34725, LOW);

  if (tcs.begin()) {
    Serial.println("Found sensor");
    // LED off to prevent overheating
    tcs.setInterrupt(true);
    
  } else {
    Serial.println("No TCS34725 found ... check your connections");
    while (1);
  }

  myStepper.setSpeed(10);

  char cBuffer[5];
  int nChars;
  long lStartTime = millis();

  Serial.println("Hit debuger Send to enter Command Line Mode");

  while (Serial.available() <= 0)
  {
    if (millis() > lStartTime + 5000)
      break;
  }
  if (Serial.available() > 0)
  {
    bCommandLineMode = true;
    help();
    nChars = Serial.readBytes(cBuffer, sizeof(cBuffer));
  }
  else
  {
    StartSequencer();    
  }
}

void UpdateLCD(int iColor)
{
  
}

void GetLEGOColor()
{

  uint16_t r, g, b, c;

  delay(INTEGRATION_TIME_DELAY);
  tcs.getRawData(&r, &g, &b, &c);
  Serial.print("Red: "); Serial.print(r, DEC); Serial.print(" ");
  Serial.print("Green: "); Serial.print(g, DEC); Serial.print(" ");
  Serial.print("Blue: "); Serial.print(b, DEC); Serial.print(" ");
  Serial.print("Color: "); Serial.print(c, DEC); Serial.print(" ");

  if(c > 600 /*200*/)
  {
    UpdateLCD(YELLOW);
    Serial.print("C Yellow");
  }
/*  
  else
   if(r >= 40){
      UpdateLCD(RED);
      Serial.print("G Red");
  }
  else 
   if(b >= 50){
      UpdateLCD(BLUE);
      Serial.print("T Blue");
  }
*/  
  else 
  if(g > b && g > r){
      UpdateLCD(GREEN);
      Serial.print("A Green");
  }
  else
  if(r > g && r > b){
      UpdateLCD(RED);
      Serial.print("G Red");
  }
  else /*if(b > g)*/{
      UpdateLCD(BLUE);
      Serial.print("T Blue");
  }
  
  Serial.println(" ");
  
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
  Serial.println("Command Line Mode:");
  Serial.println("  Lxxxx to go Left  xxxx steps");
  Serial.println("  Rxxxx to go Right xxxx steps");
  Serial.println("  Sxxxx set RPM to xxxx"); 
  Serial.println("  C to Read Color Sensor");
  Serial.println("  K Read Color Sensor until Send");
  Serial.println("  1 Color Sensor Light ON");
  Serial.println("  0 Color Sensor Light OFF");
  Serial.println("  M Manual Sequencer");    
  Serial.println("  A Automated Sequencer");    
  Serial.println("  E to Start Sequencer");    
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
      Serial.println("Button pressed!");
      lcd.setCursor(0, 1);
      lcd.print("Button pressed! ");
      iState = STATE_SEQUENCING;
    }
  
    // Remember last button press event
    lastButtonPress = millis();
    return (true);
  }
  return (false);
  
}

void loop() {

  if (bCommandLineMode)
  {
    if (Serial.available() > 0)
    {
      char cBuffer[7];
      int nChars;
      int iValue = 0;

      nChars = Serial.readBytes(cBuffer, sizeof(cBuffer));
      Serial.print("nChars = ");
      Serial.println(nChars);
      cBuffer[nChars] = '\0';
      Serial.print("cBuffer = ");
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
            Serial.println("Unrecognized command!");
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

      for (int i=1; i<5; i++)
      {
        iValue = (iValue * 10) + (cBuffer[i]-'0');
      }
      Serial.print("Value = ");
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
    while (GetButtonPressed() == false)
      ;
    iState = STATE_SEQUENCING;
  }

  // STATE_SEQUENCING
  tcs.setInterrupt(false);
  digitalWrite (LED_TCS34725, HIGH);

  myStepper.setSpeed(10);
  myStepper.step(STEPS_PER_LEGO_BLOCK * 2);
     
  for (int i=0; i<10; i++)
  {
    if (bAutomated == false)
    {    
      lcd.setCursor(0, 0);
      lcd.print("Position LEGO #");
      lcd.print(i);
      
      lcd.setCursor(0, 1);
      lcd.print(" Push Button    ");
  
      while (GetButtonPressed() == false)
        ;
    }

    delay(500);
    GetLEGOColor();

    if (i<9)
      myStepper.step(STEPS_PER_LEGO_BLOCK);
    else
    {
      tcs.setInterrupt(true);
      digitalWrite (LED_TCS34725, LOW); 
      myStepper.setSpeed(15);
      myStepper.step(0 - ((STEPS_PER_LEGO_BLOCK * 9) + (STEPS_PER_LEGO_BLOCK * 2)));
      myStepper.setSpeed(10);
      iState = STATE_LOAD_TRAY;
    }      
}


}
