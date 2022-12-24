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

#define LED_TCS34725  A0

#include <LiquidCrystal.h>
int Contrast = 75;
LiquidCrystal lcd(8, 7, 5, 4, 3, 2);

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

  pinMode(13, INPUT_PULLUP);

  analogWrite(6, Contrast);
  lcd.begin(16, 2);

  pinMode (LED_TCS34725, OUTPUT);
  digitalWrite (LED_TCS34725, LOW);

  if (tcs.begin()) {
    Serial.println("Found sensor");
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
  
  lcd.setCursor(0, 0);
  lcd.print("LEGO DNA Sqncr  ");

  lcd.setCursor(0, 1);
  lcd.print(" Push Button    ");

  while (digitalRead(13) == HIGH)
    ;
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
    if(r < 50 && g < 50 && g < 50){
        updateShiftRegister(CLEAR);
        Serial.print("Nada");
    }
    else if(g > b && g > r){
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
void loop() {
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
    digitalWrite (LED_TCS34725, HIGH); 
    
    lcd.setCursor(0, 0);
    lcd.print("Position LEGO #");
    lcd.print(i);
    
    lcd.setCursor(0, 1);
    lcd.print(" Push Button    ");

    while (digitalRead(13) == HIGH)
      ;
      
    GetLEGOColor();
    digitalWrite (LED_TCS34725, LOW); 

    while (digitalRead(13) == LOW)
     ;
}


}
