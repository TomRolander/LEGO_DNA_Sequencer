/**************************************************************************
  LEGO DNA Sequencer
  Buzzer Unit Test
  Original Code:  2022-12-24
  Tom Rolander, MSEE
  Mentor for Circuit Design, Software, 3D Printing
  Miller Library, Fabrication Lab
  Hopkins Marine Station, Stanford University,
  120 Ocean View Blvd, Pacific Grove, CA 93950
  +1 831.915.9526 | rolander@stanford.edu
 **************************************************************************/

#define PROGRAM F("LEGO DNA Sequencer - Buzzer Unit Test")
#define VERSION F("Ver 0.1 2023-01-04")
#define DEBUG_OUTPUT 1
#define DEBUG_MODE   0


int buzzer = A0;//the pin of the active buzzer

void setup()
{
  pinMode(buzzer, OUTPUT); //initialize the buzzer pin as an output
}

void Beeps(int iNmbBeeps, int iMSLengthON, int iMSLengthOFF)
{
  for (int i=0; i<iNmbBeeps; i++)
  {
    digitalWrite(buzzer, HIGH);
    delay(iMSLengthON);//wait for 1ms
    digitalWrite(buzzer, LOW);
    delay(iMSLengthOFF);//wait for 1ms    
  }
}

void loop()
{
  Beeps(10, 1, 1000);
  delay(5000);

  Beeps(1, 20, 0);
  delay(5000);

  Beeps(2, 20, 250);
  delay(5000);

  Beeps(3, 20, 200);
  delay(5000);
}
