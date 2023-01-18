/**************************************************************************
  LEGO DNA Sequencer
  TCS34725 Color Sensor Unit Test
  
  Original Code:  2022-12-24

  Tom Rolander, MSEE
  Mentor for Circuit Design, Software, 3D Printing
  Miller Library, Fabrication Lab
  Hopkins Marine Station, Stanford University,
  120 Ocean View Blvd, Pacific Grove, CA 93950
  +1 831.915.9526 | rolander@stanford.edu

 **************************************************************************/

#define PROGRAM "LEGO DNA Sequencer - TCS34725 Color Sensor Unit Test"
#define VERSION "Ver 0.1 2022-12-24"

#define DEBUG_OUTPUT 1

#include <Wire.h>
#include "Adafruit_TCS34725.h"

#define CLEAR   -1
#define RED     3
#define GREEN   2
#define BLUE    1
#define YELLOW  0

#define LED_TCS34725  A3

/* Initialise with specific int time and gain values */
Adafruit_TCS34725 tcs = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_50MS, TCS34725_GAIN_16X);

void setup(void) {
  Serial.begin(115200);
  delay(1000);
  Serial.println();
  Serial.println(PROGRAM);
  Serial.println(VERSION);

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

  Serial.println("Delay 5 seconds, then turn on LED light");
  delay(5000);
  digitalWrite (LED_TCS34725, HIGH); 
  tcs.setInterrupt(false);
}

void loop(void) {
  uint16_t r, g, b, c;
  tcs.getRawData(&r, &g, &b, &c);
  Serial.print("Red: "); Serial.print(r, DEC); Serial.print(" ");
  Serial.print("Green: "); Serial.print(g, DEC); Serial.print(" ");
  Serial.print("Blue: "); Serial.print(b, DEC); Serial.print(" ");
  Serial.print("Clear: "); Serial.print(c, DEC); Serial.print(" ");  
  Serial.println(" ");
  
  delay(1000);
}
