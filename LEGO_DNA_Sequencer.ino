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



void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println();
  Serial.println(PROGRAM);
  Serial.println(VERSION);

}

void loop() {
  // put your main code here, to run repeatedly:

}
