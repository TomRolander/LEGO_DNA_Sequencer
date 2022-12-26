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
#define STATE_SEQUENCING  1

static int iState = STATE_START;

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

#define LATCHPIN_74HC595 A2
#define CLOCKPIN_74HC595 A3
#define DATAPIN_74HC595  A1

/* Initialise with specific int time and gain values */
Adafruit_TCS34725 tcs = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_50MS, TCS34725_GAIN_4X);


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

  // Set all the pins of 74HC595 as OUTPUT
  pinMode(LATCHPIN_74HC595, OUTPUT);
  pinMode(DATAPIN_74HC595, OUTPUT);  
  pinMode(CLOCKPIN_74HC595, OUTPUT);
/*
  updateShiftRegister(CLEAR);
  for (int i = 3; i >= 0; i--) // Turn all the LEDs ON one by one.
  {
    updateShiftRegister(i);
    delay(500);
  }
*/  
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

void updateShiftRegister(int iSelect)
{
  int iLEDs = 0;
  if (iSelect != -1 && iSelect <= 3)
    bitSet(iLEDs, iSelect);
   digitalWrite(LATCHPIN_74HC595, LOW);
   shiftOut(DATAPIN_74HC595, CLOCKPIN_74HC595, LSBFIRST, iLEDs);
   digitalWrite(LATCHPIN_74HC595, HIGH);
}

void GetLEGOColor()
{

  uint16_t r, g, b, c;

  delay(50);
  tcs.getRawData(&r, &g, &b, &c);
  Serial.print("Red: "); Serial.print(r, DEC); Serial.print(" ");
  Serial.print("Green: "); Serial.print(g, DEC); Serial.print(" ");
  Serial.print("Blue: "); Serial.print(b, DEC); Serial.print(" ");
  Serial.print("Color: "); Serial.print(c, DEC); Serial.print(" ");

  if(c > 250)
  {
    updateShiftRegister(YELLOW);
    Serial.print("C Yellow");
    //tft.print("C");
  }
  else
  {
    /* if(r < 50 && g < 50 && g < 50){
        updateShiftRegister(CLEAR);
        Serial.print("Nada");
    }
    else */
    if(g > b && g > r){
        updateShiftRegister(GREEN);
        Serial.print("A Green");
        //tft.print("A");
    }
    else if(r > g && r > b){
        updateShiftRegister(RED);
        Serial.print("G Red");
        //tft.print("G");
    }
    else /*if(b > g)*/{
        updateShiftRegister(BLUE);
        Serial.print("T Blue");
        //tft.print("T");
    }
  }
  
  Serial.println(" ");
  
}

void StartSequencer()
{
  lcd.setCursor(0, 0);
  lcd.print("LEGO DNA Sqncr  ");

  lcd.setCursor(0, 1);
  lcd.print(" Push Button    ");

  while (digitalRead(SW) == HIGH)
    ;  
}

void help()
{
  Serial.println("Command Line Mode:");
  Serial.println("  Lxxxxx to go Left  xxxxx steps");
  Serial.println("  Rxxxxx to go Right xxxxx steps");
  Serial.println("  Sxxxxx set RPM to xxxxx"); 
  Serial.println("  C to Read Color Sensor");
  Serial.println("  K Read Color Sensor until Send");
  Serial.println("  1 Color Sensor Light ON");
  Serial.println("  0 Color Sensor Light OFF");
  Serial.println("  E to Start Sequencer");    
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

      if (nChars == 1)
      {
        switch (cBuffer[0])
        {
          case 'E':
            StartSequencer();
            while (digitalRead(13) == LOW)
              ;  
            bCommandLineMode = false;
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
      if (nChars != 6 ||
          (cBuffer[0] != 'L' && cBuffer[0] != 'R' && cBuffer[0] != 'S'))
      {
        help();
        return;
      }

      for (int i=1; i<6; i++)
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
        iState == STATE_SEQUENCING;
      }
  
      // Remember last button press event
      lastButtonPress = millis();
    }
    return;
  }
   
  lcd.setCursor(0, 0);
  lcd.print("Load LEGO Tray  ");

  lcd.setCursor(0, 1);
  lcd.print(" Push Button    ");

  while (digitalRead(13) == HIGH)
    ;
  while (digitalRead(13) == LOW)
    ;

  for (int i=0; i<10; i++)
  {
    tcs.setInterrupt(false);
    digitalWrite (LED_TCS34725, HIGH); 
    
    lcd.setCursor(0, 0);
    lcd.print("Position LEGO #");
    lcd.print(i);
    
    lcd.setCursor(0, 1);
    lcd.print(" Push Button    ");

    while (digitalRead(13) == HIGH)
      ;
      
    GetLEGOColor();
    tcs.setInterrupt(true);
    digitalWrite (LED_TCS34725, LOW); 

    while (digitalRead(13) == LOW)
     ;

    if (i<9)
      myStepper.step(STEPS_PER_LEGO_BLOCK);
    else
      myStepper.step(0 - (STEPS_PER_LEGO_BLOCK * 9));
      
}


}
