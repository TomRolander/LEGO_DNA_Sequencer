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
#define VERSION F("Ver 0.3 2022-12-31")

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

#include <MD5.h>

#define NMB_BAD_WORDS 10
const unsigned char ucBadWords[NMB_BAD_WORDS][16] =
{
{0x84,0xb9,0x06,0x03,0x0d,0x48,0x52,0x7c,0xe0,0x5c,0x36,0x82,0x0f,0x94,0xc9,0xa3},
{0x01,0x2e,0x3b,0x42,0x29,0xed,0x77,0x78,0x85,0x07,0x44,0xbb,0x32,0x7f,0xa5,0x54},
{0x71,0xb4,0x74,0xfa,0x0a,0x68,0x85,0x32,0x55,0x2a,0x0a,0x95,0x46,0x4a,0x9a,0x00},
{0x4f,0x79,0x1f,0x1f,0xc2,0xf1,0xbc,0x1a,0x86,0xab,0xc3,0x8d,0x46,0x78,0x9b,0x34},
{0x00,0x94,0xcf,0x00,0x63,0x7a,0xee,0x49,0xb2,0x3c,0x4f,0x44,0xba,0xaf,0xe0,0xde},
{0x4a,0x46,0x6a,0x30,0x2e,0x55,0xdd,0x2e,0x4e,0x8d,0xf6,0xde,0xa7,0xaa,0xc3,0x2f},
{0xbe,0x97,0x62,0x73,0xca,0x31,0x79,0xad,0x77,0x72,0x63,0xfb,0x26,0xac,0x08,0xd4},
{0x27,0x50,0xfe,0xdb,0xda,0x13,0x41,0xe9,0x84,0x2b,0xdd,0xb7,0x44,0xc8,0x62,0x03},
{0xa0,0x15,0x4f,0x58,0xd6,0xa4,0x46,0x1c,0x9d,0x35,0x3d,0x04,0xd6,0x3d,0xb0,0x8b},
{0xf7,0x3e,0x87,0x12,0x37,0x3e,0xba,0xab,0xa7,0xf8,0xb6,0x91,0xa1,0x92,0x24,0xeb}
};

/*
  F("84b906030d48527ce05c36820f94c9a3"),
  F("012e3b4229ed7778850744bb327fa554"),
  F("71b474fa0a688532552a0a95464a9a00"),
  F("4f791f1fc2f1bc1a86abc38d46789b34"),
  F("0094cf00637aee49b23c4f44baafe0de"),
  F("4a466a302e55dd2e4e8df6dea7aac32f"),
  F("be976273ca3179ad777263fb26ac08d4"),
  F("2750fedbda1341e9842bddb744c86203"),
  F("a0154f58d6a4461c9d353d04d63db08b"),
  F("f73e8712373ebaaba7f8b691a19224eb")
 */

#include <EEPROM.h>

#define EEPROM_LEGO_SIGNATURE F("LEGO");

// EEPROM state variables
#define EEPROM_SIZE 512

#define EEPROM_SIGNATURE                    0 // 000-003  Signature 'LEGO'
#define EEPROM_NUMBER_OF_LEGO_DNA           4 // 004-004  iNumberOfEEPROM
#define EEPROM_LEGO_DNA                     5 // 005-004  LEGO_DNAs

//#define EEPROM_LEGO_DNA_MAX                18
#define EEPROM_LEGO_DNA_MAX                10
#define EEPROM_LEGO_DNA_ENTRY_SIZE         28

//#define LEGO_DNA_MAX  24
#define LEGO_DNA_MAX  16

static int iNumberOfEEPROM = 0;

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
{ "  MAKO SHARK    ",
  "  ABALONE       ",
  "  SQUID         ",
  "  COPEPOD       ",
  "  SEA STAR      ",
  "  TUNA          "
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

for (int i=0; i<128; i++)
{
  int iByte = EEPROM.read(i);
  if (i % 16 == 0)
    Serial.println("");
  if (iByte < 16)
    Serial.print("0");
  Serial.print(iByte, HEX);
  Serial.print(" ");
}
Serial.println("");

  if (EEPROM.read(EEPROM_SIGNATURE + 0) == 'L' &&
      EEPROM.read(EEPROM_SIGNATURE + 1) == 'E' &&
      EEPROM.read(EEPROM_SIGNATURE + 2) == 'G' &&
      EEPROM.read(EEPROM_SIGNATURE + 3) == 'O')
  {
    iNumberOfEEPROM = EEPROM[EEPROM_NUMBER_OF_LEGO_DNA];

    int index;
    int iOffset = EEPROM_LEGO_DNA;
    for (index = 0; index < iNumberOfEEPROM; index++)
    {
      for (int i = 0; i < 11; i++)
      {
        sLEGO_DNA_Sequence[iLEGO_DNA_Number][i] = EEPROM[iOffset];
        iOffset = iOffset + 1;
      }
      for (int i = 0; i < 17; i++)
      {
        sLEGO_DNA_Name[iLEGO_DNA_Number][i] = EEPROM[iOffset];
        iOffset = iOffset + 1;
      }
Serial.println(F("EEPROM Data"));
Serial.print(F("Index "));
Serial.print(iLEGO_DNA_Number);
Serial.print(F(" ["));
Serial.print(&sLEGO_DNA_Sequence[iLEGO_DNA_Number][0]);
Serial.print(F("] ["));
Serial.print(&sLEGO_DNA_Name[iLEGO_DNA_Number][0]);
Serial.println("]");
      
      iLEGO_DNA_Number = iLEGO_DNA_Number + 1;
    }

  }
  else
  {
#if DEBUG_OUTPUT
    Serial.println("EEPROM LEGO not found");
#endif
    SetupEEPROM();
  }
#if DEBUG_OUTPUT
  Serial.println("EEPROM LEGO found");
  Serial.print("iNumberOfEEPROM = ");
  Serial.println(iNumberOfEEPROM);
#endif
  //  EEPROM.end();

}

void SetupEEPROM()
{
  EEPROM[EEPROM_SIGNATURE + 0] = 'L';
  EEPROM[EEPROM_SIGNATURE + 1] = 'E';
  EEPROM[EEPROM_SIGNATURE + 2] = 'G';
  EEPROM[EEPROM_SIGNATURE + 3] = 'O';
  EEPROM[EEPROM_NUMBER_OF_LEGO_DNA] = iNumberOfEEPROM;

#if DEBUG_OUTPUT
  Serial.println("EEPROM LEGO initialized");
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
  Serial.println(F("  Z zero out added DNAs"));
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
          case 'Z':
            iNumberOfEEPROM = 0;
            iLEGO_DNA_Number = 6;
            for (int i=0; i<512; i++)
              EEPROM[i] = '\0';
            SetupEEPROM();
            break;
            
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
    }
    else
    {
      return;
    }
  }

  if (iState == STATE_LOAD_TRAY)
  {
    lcd.setCursor(0, 0);
    lcd.print("Load LEGO Tray  ");

    lcd.setCursor(0, 1);
    lcd.print(" Push Button    ");

    while (GetButtonPressed() == false)
    {
      if (CheckRotaryEncoder())
      {
        bCommandLineMode = true;
        return;
      }
    }
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
//#if FLORA
//  myStepper.step(962);
//#else
  myStepper.step(1260);
//#endif
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
//#if FLORA
//      myStepper.step(-5912);
//#else
      myStepper.step(-6210);
//#endif
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

          int index;
          for (index = 0; index < 16; index++)
          {
            cNextChar = GetNextChar(index);
            if (cNextChar == '\0')
            {
              break;
            }
            sNewName[index] = cNextChar;
            sNewName[index + 1] = '\0';
          }
          for (; index < 16; index++)
          {
            sNewName[index] = ' ';
          }
          sNewName[16] = '\0';
          Serial.println(sNewName);

          if (GetYesOrNo(false, sNewName))
          {
            if (iNumberOfEEPROM >= (EEPROM_LEGO_DNA_MAX - 1))
            {
              lcd.setCursor(0, 0);
              lcd.print("EEPROM DB full! ");
              lcd.setCursor(0, 1);
              lcd.print(" Push Button    ");

              while (GetButtonPressed() == false)
                ;
              while (digitalRead(SW) == LOW)
                ;
            }
            else
            {
              Serial.print(F("Adding to Database ["));
              Serial.print(sDNASequence);
              Serial.print(F("] = ["));
              Serial.print(sNewName);
              Serial.println(F("]"));

              // Check for Bad Word
              bool bBadWord = false;
              for (int i=0; i<NMB_BAD_WORDS; i++)
              {
                char sQuad[5] = "    ";
                for (int j=0; j<13; j++)
                {
                  int k;
                  strncpy(sQuad, &sNewName[j], 4);                
                  //generate the MD5 hash for our string
                  unsigned char* hash=MD5::make_hash(sQuad);
                  
                  for (k=0; k<16; k++)
                  {
                    if (hash[k] == ucBadWords[i][k])
                      continue;
                    break;
                  }
                  if (k == 16)
                    bBadWord = true;                  
                  free(hash);
                  if (bBadWord)
                    break;
                }
                if (bBadWord)
                  break;
              }
              if (bBadWord)
              {
                lcd.setCursor(0, 0);
                lcd.print("Bad Word in Name");
                lcd.setCursor(0, 1);
                lcd.print(" Push Button    ");
Serial.println("BAD WORD!");                
                while (GetButtonPressed() == false)
                  ;
                while (digitalRead(SW) == LOW)
                  ;
              }
              else
              {
                // Now add to DATABASE
                int iOffset = (EEPROM_LEGO_DNA_ENTRY_SIZE * iNumberOfEEPROM);
                for (int i = 0; i <= 11; i++)
                {
                  sLEGO_DNA_Sequence[iLEGO_DNA_Number][i] = sDNASequence[i];
                  EEPROM[EEPROM_LEGO_DNA + iOffset] = sDNASequence[i];
                  iOffset = iOffset + 1;
                }
                iOffset = iOffset - 1;
                for (int i = 0; i <= 17; i++)
                {
                  sLEGO_DNA_Name[iLEGO_DNA_Number][i] = sNewName[i];
                  EEPROM[EEPROM_LEGO_DNA + iOffset] = sNewName[i];
                  iOffset = iOffset + 1;
                }
  
                iNumberOfEEPROM = iNumberOfEEPROM + 1;
                EEPROM[EEPROM_NUMBER_OF_LEGO_DNA] = iNumberOfEEPROM;

                iLEGO_DNA_Number = iLEGO_DNA_Number + 1;
              }
            }
          }
        }
      }
      iState = STATE_LOAD_TRAY;
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
