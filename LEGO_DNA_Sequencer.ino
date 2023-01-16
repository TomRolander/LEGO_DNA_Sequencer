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
#define VERSION F("Ver 0.9 2023-01-16")
#define PROGRAM_SHORT F("LEGO DNA Sqncr  ")
#define VERSION_SHORT F("Ver 0.9 01-16-23")

#define DEBUG_OUTPUT 0
#define DEBUG_MODE   0

#define FILTER_BAD_WORDS  0

#define FLORA 0

#define STATE_START       0
#define STATE_LOAD_TRAY   1
#define STATE_SEQUENCING  2

// Proc to reset the Arduino
void (* re_set)(void) = 0x00;

static bool bRemoteLCDSerial = false;

static int iState = STATE_START;

static int iStepPosition = 0;

static bool bAutomated = true;

#define BUZZER        A0

#define LED_TCS34725  A3

#if DEBUG_MODE
static bool bCommandLineMode = false;
#endif

// Rotary Encoder Inputs
#define CLK A1
#define DT A2
#define SW 13

static int iRotaryEncoder_Counter = 0;
static int iRotaryEncoder_CurrentStateCLK;
static int iRotaryEncoder_LastStateCLK;
static bool bRotaryEncoder_CurrentDirectionClockwise;
static unsigned long lastButtonPress = 0;

char sNameChars[40] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789 <*";
//                     0123456789012345678901234567890123456789
//                     0         1         2         3
#define NMB_NAME_CHARS  39

char sNumberChars[13] = "0123456789<*";
//                       0123456789012
//                       0         1
#define NMB_NUMBER_CHARS  12


#if FILTER_BAD_WORDS
#include <MD5.h>
#define NMB_BAD_WORDS 10
const unsigned char ucBadWords[NMB_BAD_WORDS][16] =
{
  {0x84, 0xb9, 0x06, 0x03, 0x0d, 0x48, 0x52, 0x7c, 0xe0, 0x5c, 0x36, 0x82, 0x0f, 0x94, 0xc9, 0xa3},
  {0x01, 0x2e, 0x3b, 0x42, 0x29, 0xed, 0x77, 0x78, 0x85, 0x07, 0x44, 0xbb, 0x32, 0x7f, 0xa5, 0x54},
  {0x71, 0xb4, 0x74, 0xfa, 0x0a, 0x68, 0x85, 0x32, 0x55, 0x2a, 0x0a, 0x95, 0x46, 0x4a, 0x9a, 0x00},
  {0x4f, 0x79, 0x1f, 0x1f, 0xc2, 0xf1, 0xbc, 0x1a, 0x86, 0xab, 0xc3, 0x8d, 0x46, 0x78, 0x9b, 0x34},
  {0x00, 0x94, 0xcf, 0x00, 0x63, 0x7a, 0xee, 0x49, 0xb2, 0x3c, 0x4f, 0x44, 0xba, 0xaf, 0xe0, 0xde},
  {0x4a, 0x46, 0x6a, 0x30, 0x2e, 0x55, 0xdd, 0x2e, 0x4e, 0x8d, 0xf6, 0xde, 0xa7, 0xaa, 0xc3, 0x2f},
  {0xbe, 0x97, 0x62, 0x73, 0xca, 0x31, 0x79, 0xad, 0x77, 0x72, 0x63, 0xfb, 0x26, 0xac, 0x08, 0xd4},
  {0x27, 0x50, 0xfe, 0xdb, 0xda, 0x13, 0x41, 0xe9, 0x84, 0x2b, 0xdd, 0xb7, 0x44, 0xc8, 0x62, 0x03},
  {0xa0, 0x15, 0x4f, 0x58, 0xd6, 0xa4, 0x46, 0x1c, 0x9d, 0x35, 0x3d, 0x04, 0xd6, 0x3d, 0xb0, 0x8b},
  {0xf7, 0x3e, 0x87, 0x12, 0x37, 0x3e, 0xba, 0xab, 0xa7, 0xf8, 0xb6, 0x91, 0xa1, 0x92, 0x24, 0xeb}
};
#endif

#include <EEPROM.h>

#define EEPROM_LEGO_SIGNATURE F("LEGO");

// EEPROM state variables
#define EEPROM_SIZE 512

#define EEPROM_SIGNATURE                    0 // 000-003  Signature 'LEGO'
#define EEPROM_CLEAR_CHANNEL_THRESHHOLD     4 // 004-005  iRED_CHANNEL_THRESHHOLD
#define EEPROM_RED_CHANNEL_THRESHHOLD       6 // 006-007  iRED_CHANNEL_THRESHHOLD
#define EEPROM_BLUE_CHANNEL_THRESHHOLD      8 // 008-009  iBLUE_CHANNEL_THRESHHOLD
#define EEPROM_REMOTE_LCD_SERIAL           10 // 010-010  bRemoteLCDSerial
#define EEPROM_NUMBER_OF_LEGO_DNA          11 // 011-011  iNumberOfEEPROM
#define EEPROM_LEGO_DNA                    12 // 012-511  LEGO_DNAs

#define EEPROM_LEGO_DNA_MAX                4
#define EEPROM_LEGO_DNA_ENTRY_SIZE         28

#define LEGO_DNA_MAX  11

static int iNumberOfEEPROM = 0;

#define NMB_LEGO_BRICKS 10

int iBrickSequencing[NMB_LEGO_BRICKS][4];
char sDNASequence[NMB_LEGO_BRICKS + 1] = "";


int iLEGO_DNA_Number = 7;
char sLEGO_DNA_Sequence[LEGO_DNA_MAX][NMB_LEGO_BRICKS + 1] =
{ "CCGGTTAACC",
  "ATTGGTCATT",
  "TGCTCCTACA",
  "CACAATCTAC",
  "GCTCCCGGGT",
  "CAAATCTTAG",
  "CGTCTACCAA"
};
char sLEGO_DNA_Name[LEGO_DNA_MAX][17] =
{ " YYRRBBGGYY TEST",
  "  MAKO SHARK    ",
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

#define OFFSET_TO_FIRST_BRICK 460
#define STEPS_PER_LEGO_BRICK  550
#define STEPS_PER_AUTO_POSITION   25
#define OFFSET_AUTO_POSITION  200

// Creates an instance of stepper class
// Pins entered in sequence IN1-IN3-IN2-IN4 for proper step sequence
Stepper myStepper = Stepper(stepsPerRevolution, STEPPER_PIN_1, STEPPER_PIN_3, STEPPER_PIN_2, STEPPER_PIN_4);

#if DEBUG_MODE
#define MAINTENANCE_NMB_OPS           10
#else
#define MAINTENANCE_NMB_OPS           9
#endif
#define MAINTENANCE_OP_CANCEL         0
#if DEBUG_MODE
#define MAINTENANCE_OP_COMMAND_LINE   1
#define MAINTENANCE_OP_UNLOAD_TRAY    2
#define MAINTENANCE_OP_LOAD_TRAY      3
#define MAINTENANCE_OP_POSITION_TRAY  4
#define MAINTENANCE_OP_ZERO_NEW_DNA   5
#define MAINTENANCE_OP_BUZZER_OFF     6
#define MAINTENANCE_OP_BUZZER_ON      7
#define MAINTENANCE_OP_ADVANCED       8
#define MAINTENANCE_OP_VERSION_INFO   9
#endif
#define MAINTENANCE_OP_UNLOAD_TRAY    1
#define MAINTENANCE_OP_LOAD_TRAY      2
#define MAINTENANCE_OP_POSITION_TRAY  3
#define MAINTENANCE_OP_ZERO_NEW_DNA   4
#define MAINTENANCE_OP_BUZZER_OFF     5
#define MAINTENANCE_OP_BUZZER_ON      6
#define MAINTENANCE_OP_ADVANCED       7
#define MAINTENANCE_OP_VERSION_INFO   8

char sMaintenanceOp[MAINTENANCE_NMB_OPS][17] =
{ "  CANCEL        ",
#if DEBUG_MODE
  "  COMMAND LINE  ",
#endif
  "  UNLOAD TRAY   ",
  "  LOAD TRAY     ",
  "  POSITION TRAY ",
  "  ZERO NEW DNA  ",
  "  BUZZER OFF    ",
  "  BUZZER ON     ",
  "  ADVANCED      ",
  "  VERSION INFO  "
};

#define ADVANCED_NMB_OPS            9

#define ADVANCED_OP_CANCEL          0
#define ADVANCED_OP_CLEAR_CHANNEL   1
#define ADVANCED_OP_RED_CHANNEL     2
#define ADVANCED_OP_BLUE_CHANNEL    3
#define ADVANCED_OP_AUTO_CHANNEL    4
#define ADVANCED_OP_AUTO_POSITION   5
#define ADVANCED_OP_REMOTE_LCD_ON   6
#define ADVANCED_OP_REMOTE_LCD_OFF  7
#define ADVANCED_OP_REBOOT          8

char sAdvancedOp[ADVANCED_NMB_OPS][17] =
{ "  CANCEL        ",
  "  CLEAR CHANNEL ",
  "  RED CHANNEL   ",
  "  BLUE CHANNEL  ",
  "  AUTO CHANNEL  ",
  "  AUTO POSITION ",
  "  REMOTE LCD ON ",
  "  REMOTE LCD OFF",
  "  REBOOT        "
};


static bool bBuzzer = true;

#include <Wire.h>
#include "Adafruit_TCS34725.h"

#define CLEAR   -1
#define RED     'G'
#define GREEN   'A'
#define BLUE    'T'
#define YELLOW  'C'

/* Initialise with specific int time and gain values */
Adafruit_TCS34725 tcs = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_50MS, TCS34725_GAIN_16X);

#if FLORA
#define CLEAR_CHANNEL_THRESHHOLD  130
#define RED_CHANNEL_THRESHHOLD  35
#define BLUE_CHANNEL_THRESHHOLD  30
#else
#define CLEAR_CHANNEL_THRESHHOLD  900
#define RED_CHANNEL_THRESHHOLD  170
#define BLUE_CHANNEL_THRESHHOLD  220
#endif

static int iCLEAR_CHANNEL_THRESHHOLD = CLEAR_CHANNEL_THRESHHOLD;
static int iRED_CHANNEL_THRESHHOLD = RED_CHANNEL_THRESHHOLD;
static int iBLUE_CHANNEL_THRESHHOLD = BLUE_CHANNEL_THRESHHOLD;

void setup() {
  Serial.begin(115200);
  delay(1000);
  
#if DEBUG_OUTPUT
  Serial.println();
  Serial.println(PROGRAM);
  Serial.println(VERSION);
#endif

  pinMode(BUZZER, OUTPUT); //initialize the buzzer pin as an output

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
#if DEBUG_OUTPUT
    Serial.println(F("Found sensor"));
#endif    
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
  lcd.print(F("LEGO DNA Sqncr  "));
  lcd.setCursor(0, 1);
  lcd.print(F("Debugger Console"));
#else
  StartSequencer();
#endif

#if DEBUG_OUTPUT
  for (int i = 0; i < 128; i++)
  {
    int iByte = EEPROM[i];
    if (i % 16 == 0)
      Serial.println("");
    if (iByte < 16)
      Serial.print(F("0"));
    Serial.print(iByte, HEX);
    Serial.print(F(" "));
  }
  Serial.println("");
#endif

  LoadEEPROM();
  
  Beeps(3, 10, 200);  // 3 Beeps, 10MS ON, 200MS OFF
}

void LoadEEPROM()
{
  if (EEPROM[EEPROM_SIGNATURE + 0] == 'L' &&
      EEPROM[EEPROM_SIGNATURE + 1] == 'E' &&
      EEPROM[EEPROM_SIGNATURE + 2] == 'G' &&
      EEPROM[EEPROM_SIGNATURE + 3] == 'O')
  {
    iCLEAR_CHANNEL_THRESHHOLD = readUnsignedIntFromEEPROM(EEPROM_CLEAR_CHANNEL_THRESHHOLD);
    iRED_CHANNEL_THRESHHOLD = readUnsignedIntFromEEPROM(EEPROM_RED_CHANNEL_THRESHHOLD);
    iBLUE_CHANNEL_THRESHHOLD = readUnsignedIntFromEEPROM(EEPROM_BLUE_CHANNEL_THRESHHOLD);
    bRemoteLCDSerial= EEPROM[EEPROM_REMOTE_LCD_SERIAL];
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
#if DEBUG_OUTPUT
      Serial.println(F("EEPROM Data"));
      Serial.print(F("Index "));
      Serial.print(iLEGO_DNA_Number);
      Serial.print(F(" ["));
      Serial.print(&sLEGO_DNA_Sequence[iLEGO_DNA_Number][0]);
      Serial.print(F("] ["));
      Serial.print(&sLEGO_DNA_Name[iLEGO_DNA_Number][0]);
      Serial.println(F("]"));
#endif
      iLEGO_DNA_Number = iLEGO_DNA_Number + 1;
    }
  }
  else
  {
#if DEBUG_OUTPUT
    Serial.println(F("EEPROM LEGO not found"));
#endif
    SetupEEPROM();
  }
#if DEBUG_OUTPUT
  Serial.println(F("EEPROM LEGO found"));
  Serial.print(F("bRemoteLCDSerial = "));
  Serial.println(bRemoteLCDSerial);
  Serial.print(F("iNumberOfEEPROM = "));
  Serial.println(iNumberOfEEPROM);
  Serial.print(F("Clear Channel Threshhold = "));
  Serial.println(iCLEAR_CHANNEL_THRESHHOLD);
  Serial.print(F("Red   Channel Threshhold = "));
  Serial.println(iRED_CHANNEL_THRESHHOLD);
  Serial.print(F("Blue  Channel Threshhold = "));
  Serial.println(iBLUE_CHANNEL_THRESHHOLD);
#endif  
}

void writeUnsignedIntIntoEEPROM(int address, unsigned int number)
{
  EEPROM[address] = number >> 8;
  EEPROM[address + 1] = number & 0xFF;
}

unsigned int readUnsignedIntFromEEPROM(int address)
{
  return (EEPROM[address] << 8) + EEPROM[address + 1];
}

void SetupEEPROM()
{
  EEPROM[EEPROM_SIGNATURE + 0] = 'L';
  EEPROM[EEPROM_SIGNATURE + 1] = 'E';
  EEPROM[EEPROM_SIGNATURE + 2] = 'G';
  EEPROM[EEPROM_SIGNATURE + 3] = 'O';
  writeUnsignedIntIntoEEPROM(EEPROM_CLEAR_CHANNEL_THRESHHOLD, iCLEAR_CHANNEL_THRESHHOLD);
  writeUnsignedIntIntoEEPROM(EEPROM_RED_CHANNEL_THRESHHOLD, iRED_CHANNEL_THRESHHOLD);
  writeUnsignedIntIntoEEPROM(EEPROM_BLUE_CHANNEL_THRESHHOLD, iBLUE_CHANNEL_THRESHHOLD);
  EEPROM[EEPROM_REMOTE_LCD_SERIAL] = bRemoteLCDSerial;
  EEPROM[EEPROM_NUMBER_OF_LEGO_DNA] = iNumberOfEEPROM;
  

#if DEBUG_OUTPUT
  Serial.println(F("EEPROM LEGO initialized"));
#endif

}
void UpdateLCD(char cColor, int index)
{
  char lcd1[17] = "Sequencing DNA  ";
  char lcd2[17] = "                ";
  lcd2[index] = cColor;
  for(int i=0; i<index; i++)
    lcd2[i] = sDNASequence[i];
  RemoteLCDSerial(lcd1, lcd2);
  lcd.print(cColor);
}

char GetLEGOColor(int index)
{
  char cRetcode = '\0';

  uint16_t r, g, b, c;

  tcs.getRawData(&r, &g, &b, &c);

  iBrickSequencing[index][0] = r;
  iBrickSequencing[index][1] = g;
  iBrickSequencing[index][2] = b;
  iBrickSequencing[index][3] = c;

#if DEBUG_OUTPUT
  Serial.print(iStepPosition, DEC); Serial.print(F(","));
  Serial.print(r, DEC); Serial.print(F(","));
  Serial.print(g, DEC); Serial.print(F(","));
  Serial.print(b, DEC); Serial.print(F(","));
  Serial.print(c, DEC); Serial.print(F(","));
#endif

  if (c >= iCLEAR_CHANNEL_THRESHHOLD)
  {
    UpdateLCD(YELLOW, index);
    cRetcode = YELLOW;
#if DEBUG_OUTPUT
    Serial.println(F("Y,C"));
#endif    
  }
  else if (r >= iRED_CHANNEL_THRESHHOLD)
  {
    UpdateLCD(RED, index);
    cRetcode = RED;
#if DEBUG_OUTPUT
    Serial.println(F("R,G"));
#endif    
  }
  else if (b >= iBLUE_CHANNEL_THRESHHOLD)
  {
    UpdateLCD(BLUE, index);
    cRetcode = BLUE;
#if DEBUG_OUTPUT
    Serial.println(F("B,T"));
#endif    
  }
  else
    /* if (g > b && g > r) */
  {
    UpdateLCD(GREEN, index);
    cRetcode = GREEN;
#if DEBUG_OUTPUT
    Serial.println(F("G,A"));
#endif    
  }

  return (cRetcode);
}

void StartSequencer()
{
  lcd.setCursor(0, 0);
  lcd.print(F("LEGO DNA Sqncr  "));
  lcd.setCursor(0, 1);
  lcd.print(F(" Push Button    "));
  F_RemoteLCDSerial(F("LEGO DNA Sqncr  "),F(" Push Button    "));
}

void help()
{
#if DEBUG_OUTPUT
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
#endif  
}

bool GetButtonPressed()
{
  // Read the button state
  int btnState = digitalRead(SW);

  //If we detect LOW signal, button is pressed
  if (btnState == LOW)
  {
    //if 50ms have passed since last LOW pulse, it means that the
    //button has been pressed, released and pressed again
    if (millis() - lastButtonPress > 50) {
      //      lcd.setCursor(0, 1);
      //      lcd.print("Button pressed! ");
    }

    // Wait for button to be released
    while (digitalRead(SW) == LOW)
      ;

    // Remember last button press event
    lastButtonPress = millis();
    return (true);
  }
  return (false);
}

bool GetYesOrNo(bool bInitialResponse, char *sQuestion)
{
  bool bResponse = bInitialResponse;

  char lcd1[17] = "                ";
  char lcd2[17];
  strncpy(lcd1, sQuestion, strlen(sQuestion));
  
  lcd.setCursor(0, 0);
  lcd.print(F("                "));
  lcd.setCursor(0, 0);
  lcd.print(sQuestion);
  lcd.setCursor(0, 1);
  if (bInitialResponse == false)
  {
    lcd.print(F("[NO]  YES       "));
    strcpy(lcd2,"[NO]  YES       ");
  }
  else
  {
    lcd.print(F(" NO  [YES]      "));
    strcpy(lcd2," NO  [YES]      ");
  }
  if (bRemoteLCDSerial)
  {
    delay(500);
    RemoteLCDSerial(lcd1, lcd2);
  }

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
          lcd.print(F("[NO]  YES       "));
          strcpy(lcd2,"[NO]  YES       ");
        }
        bResponse = false;
      }
      else
      {
        if (bResponse != true)
        {
          lcd.setCursor(0, 1);
          lcd.print(F(" NO  [YES]      "));
          strcpy(lcd2," NO  [YES]      ");
        }
        bResponse = true;
      }
      if (bRemoteLCDSerial)
      {
        delay(500);
        RemoteLCDSerial(lcd1, lcd2);
      }
   }
  }
  while (digitalRead(SW) == LOW)
    ;

  return (bResponse);
}

///////////////////////////////////////////////
int GetNumber(int iCurrentValue, char *iName, int iOffsetEEPROM)
{
  lcd.setCursor(0, 0);
  lcd.print(F("Enter "));
  lcd.write((byte) 0xff);
  lcd.print(F(" to end  "));
  lcd.setCursor(0, 1);
  lcd.print(F("                "));
  lcd.setCursor(0, 1);
  lcd.print(iName);
  lcd.print(F("="));

  int iOffset = strlen(iName) + 1;
  char sNewNumber[5] = "";
  char cNextChar;

  sprintf(sNewNumber, "%d", iCurrentValue);
  lcd.print(sNewNumber);

  int index = strlen(sNewNumber);
  while (index < 5)
  {
    cNextChar = GetNextNumber(index + iOffset);
    if (cNextChar == '\0')
    {
      break;
    }
    if (cNextChar == '<')
    {
      if (index > 0)
      {
        lcd.setCursor(index + iOffset, 1);
        lcd.print(' ');
        index = index - 1;
        continue;
      }
    }
    sNewNumber[index] = cNextChar;
    index = index + 1;
    sNewNumber[index] = '\0';
  }

  if (strlen(sNewNumber) != 0)
  {
    int iValue = 0;
    for (int i = 0; i < 4; i++)
    {
      if (sNewNumber[i] == '\0')
        break;
      iValue = (iValue * 10) + (sNewNumber[i] - '0');
    }
    if (iValue != iCurrentValue)
    {
      char sTemp[17] = "SET ";
      strcat(sTemp, iName);
      strcat(sTemp, "=");
      strcat(sTemp, sNewNumber);
      if (GetYesOrNo(false, sTemp))
      {
        writeUnsignedIntIntoEEPROM(iOffsetEEPROM, iValue);
        return (iValue);
      }
    }
  }
  return (-1);
}
///////////////////////////////////////////////

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
  delay(10);
  return (bResponse);
}

void loop()
{
#if DEBUG_MODE
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
            for (int i = 0; i < 512; i++)
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
            GetLEGOColor(0);
            break;

          case 'K':
            while (Serial.available() <= 0)
            {
              GetLEGOColor(0);
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
        MoveTray(iValue);
        GetLEGOColor(0);
      }
    }
    return;
  }
#endif

  if (iState == STATE_START)
  {
    if (GetButtonPressed())
    {
      //      while (digitalRead(SW) == LOW)
      //        ;
      iState = STATE_LOAD_TRAY;
    }
    else
    {
      return;
    }
  }

  if (iState == STATE_LOAD_TRAY)
  {
    F_RemoteLCDSerial(F("Load LEGO Tray  "), F(" Push Button    ")); 
    lcd.setCursor(0, 0);
    lcd.print(F("Load LEGO Tray  "));
    lcd.setCursor(0, 1);
    lcd.print(F(" Push Button    "));

    Beeps(1, 5, 0);  // 1 Beep, 5MS ON, 0MS OFF

    while (GetButtonPressed() == false)
    {
      if (CheckRotaryEncoder())
      {
        lcd.setCursor(0, 0);
        lcd.print(F("Select Option:  "));

        int iMaintenanceOp = MAINTENANCE_OP_CANCEL;

        while (true)
        {
          lcd.setCursor(0, 1);
          lcd.print(sMaintenanceOp[iMaintenanceOp]);

          if (GetButtonPressed())
            break;

          int iCounter = iRotaryEncoder_Counter;
          if (CheckRotaryEncoder())
          {
            if (iRotaryEncoder_Counter > iCounter)
            {
              iMaintenanceOp++;
              if (iMaintenanceOp  >= MAINTENANCE_NMB_OPS)
                iMaintenanceOp = MAINTENANCE_OP_CANCEL;
            }
            else
            {
              iMaintenanceOp--;
              if (iMaintenanceOp < 0)
                iMaintenanceOp = MAINTENANCE_OP_VERSION_INFO;
            }
          }
        }

        switch (iMaintenanceOp)
        {
          case MAINTENANCE_OP_CANCEL:
#if DEBUG_OUTPUT
            Serial.println(F("Cancel"));
#endif            
            StartSequencer();
            iState = STATE_START;
#if DEBUG_MODE
            bCommandLineMode = false;
#endif
            break;

#if DEBUG_MODE
          case MAINTENANCE_OP_COMMAND_LINE:
            Serial.println(F("Enter Command Line Mode"));
            bCommandLineMode = true;
            break;
#endif

          case MAINTENANCE_OP_UNLOAD_TRAY:
#if DEBUG_OUTPUT
            Serial.println(F("Unload Tray"));
#endif            
            myStepper.step(-4000);
            iStepPosition = 0;
            break;

          case MAINTENANCE_OP_LOAD_TRAY:
#if DEBUG_OUTPUT
            Serial.println(F("Load Tray"));
#endif            
            myStepper.step(1640);
            iStepPosition = 0;
            break;

          case MAINTENANCE_OP_POSITION_TRAY:
#if DEBUG_OUTPUT
            Serial.println(F("Position Tray"));
#endif            
            {
              int iCounter = iRotaryEncoder_Counter;
              while (GetButtonPressed() == false)
              {
                if (CheckRotaryEncoder())
                {
#if DEBUG_OUTPUT
#if 0
                  Serial.print(F("iCounter = "));
                  Serial.print(iCounter);
                  Serial.print(F("  iRotaryEncoder_Counter = "));
                  Serial.println(iRotaryEncoder_Counter);
#endif
#endif
                  if (iRotaryEncoder_Counter > iCounter)
                    MoveTray(50);
                  else
                    MoveTray(-50);
                  iCounter = iRotaryEncoder_Counter;
                }
              }
            }
            break;

          case MAINTENANCE_OP_ZERO_NEW_DNA:
            iNumberOfEEPROM = 0;
            iLEGO_DNA_Number = 6;
            for (int i = EEPROM_NUMBER_OF_LEGO_DNA; i < 512; i++)
              EEPROM[i] = '\0';
            SetupEEPROM();
            break;

          case MAINTENANCE_OP_BUZZER_ON:
            bBuzzer = true;
            break;

          case MAINTENANCE_OP_BUZZER_OFF:
            bBuzzer = false;
            break;

          case MAINTENANCE_OP_ADVANCED:
            Advanced();
            break;

          case MAINTENANCE_OP_VERSION_INFO:
            lcd.setCursor(0, 0);
            lcd.print(PROGRAM_SHORT);
            lcd.setCursor(0, 1);
            lcd.print(VERSION_SHORT);
            while (GetButtonPressed() == false &&
                   CheckRotaryEncoder() == false)
            {
              delay(100);
            }
            break;
        }

        return;
      }
    }
    iState = STATE_SEQUENCING;
  }

  lcd.setCursor(0, 0);
  lcd.print(F("Sequencing DNA  "));
  lcd.setCursor(0, 1);
  lcd.print(F("                "));

  F_RemoteLCDSerial(F("Sequencing DNA  "),F("                "));

  lcd.setCursor(0, 1);


  // STATE_SEQUENCING
  tcs.setInterrupt(false);
  digitalWrite (LED_TCS34725, HIGH);

  myStepper.setSpeed(10);
  iStepPosition = 0;
  MoveTray(OFFSET_TO_FIRST_BRICK);

  sDNASequence[NMB_LEGO_BRICKS] = '\0';

#if DEBUG_OUTPUT
  Serial.println("");
  Serial.println(F("---------- CSV ----------"));
  Serial.println(F("P,R,G,B,Y,M,DNA"));
#endif

  for (int i = 0; i < NMB_LEGO_BRICKS; i++)
  {
    if (bAutomated == false)
    {
      lcd.setCursor(0, 0);
      lcd.print(F("Position LEGO # "));
      lcd.print(i);
      lcd.setCursor(0, 1);
      lcd.print(F(" Push Button    "));

      while (GetButtonPressed() == false)
        ;
    }

    delay(100); // Delay a little for visual effect
    sDNASequence[i] = GetLEGOColor(i);

    if (i < 9)
    {
      Beeps(1, 2, 0);   // 1 Beep, 2MS ON, 0MS OFF
      MoveTray(STEPS_PER_LEGO_BRICK);
    }
    else
    {
      tcs.setInterrupt(true);
      digitalWrite (LED_TCS34725, LOW);

      lcd.setCursor(0, 0);
      lcd.print(F("Unloading tray  "));

      char lcd1[17] = "Unloading tray  ";
      char lcd2[17] = "                ";
      for(int i=0; i<NMB_LEGO_BRICKS; i++)
        lcd2[i] = sDNASequence[i];
      RemoteLCDSerial(lcd1, lcd2);       

      Beeps(2, 5, 250);  // 2 Beeps, 5MS ON, 250MS OFF

      myStepper.setSpeed(15);
      MoveTray(-5410);

#if DEBUG_MODE
      Serial.print(F("["));
      Serial.print(sDNASequence);
      Serial.println(F("]"));
      for (int i = 0; i < NMB_LEGO_BRICKS; i++)
      {
        for (int j = 0; j < 4; j++)
        {
          Serial.print(iBrickSequencing[i][j]);
          Serial.print(F(" "));
        }
        Serial.println("");
      }
#endif
      myStepper.setSpeed(10);
      iState = STATE_LOAD_TRAY;

      if ((strcmp(sDNASequence, "AAAAAAAAAA") == 0) ||
          (strcmp(sDNASequence, "CCCCCCCCCC") == 0) ||
          (strcmp(sDNASequence, "GGGGGGGGGG") == 0) ||
          (strcmp(sDNASequence, "TTTTTTTTTT") == 0))
      {
        lcd.setCursor(0, 0);
        lcd.print(F("EMPTY TRAY ERROR"));
        lcd.setCursor(0, 1);
        lcd.print(F("  PUSH BUTTON   "));

        Beeps(4, 10, 200);  // 4 Beeps, 10MS ON, 200MS OFF

        while (GetButtonPressed() == false)
          ;
      }
      else
      {
        int index;
        for (index = 0; index < iLEGO_DNA_Number; index++)
        {
          if (strcmp(sDNASequence, sLEGO_DNA_Sequence[index]) == 0)
            break;
        }

        if (index < iLEGO_DNA_Number)
        {
          lcd.setCursor(0, 0);
          lcd.print(F("Successful match"));
          lcd.setCursor(0, 1);
          lcd.print(sLEGO_DNA_Name[index]);

          char lcd1[17] = "Successful match";
          char lcd2[17] = "";
          strcpy(lcd2, sLEGO_DNA_Name[index]);
          RemoteLCDSerial(lcd1, lcd2);                

          Beeps(3, 5, 200);  // 3 Beeps, 5MS ON, 200MS OFF
        }
        else
        {
          lcd.setCursor(0, 0);
          lcd.print(F("DNA not a match "));
          lcd.setCursor(0, 1);
          lcd.print(F("  UNKNOWN DNA!  "));
          F_RemoteLCDSerial(F("DNA not a match "), F("  UNKNOWN DNA!  "));                

          Beeps(4, 10, 200);  // 4 Beeps, 10MS ON, 200MS OFF
        }

        while (GetButtonPressed() == false)
          ;

        if (index >= iLEGO_DNA_Number)
        {
          if (GetYesOrNo(false, "Add to Database?"))
          {
#if DEBUG_OUTPUT
            Serial.println(F("Add to Database!"));
#endif            
            lcd.setCursor(0, 0);
            lcd.print(F("Name:  * to end "));
            lcd.setCursor(0, 1);
            lcd.print(F("                "));
            F_RemoteLCDSerial(F("Name:  * to end "), F("                "));                

            char sNewName[17] = "";
            char cNextChar;

            int index = 0;
            while (index < 16)
            {
              cNextChar = GetNextChar(index);
              if (cNextChar == '\0')
              {
                break;
              }
              if (cNextChar == '<')
              {
                if (index > 0)
                {
                  lcd.setCursor(index, 1);
                  lcd.print(' ');
                  index = index - 1;
                  char lcd1[17] = "Name:  * to end ";
                  char lcd2[17] = "                ";
                  strncpy(lcd2, sNewName, strlen(sNewName));
                  RemoteLCDSerial(lcd1, lcd2);              
                  continue;
                }
              }
              sNewName[index] = cNextChar;
              index = index + 1;
              sNewName[index] = '\0';

              char lcd1[17] = "Name:  * to end ";
              char lcd2[17] = "                ";
              strncpy(lcd2, sNewName, strlen(sNewName));
              RemoteLCDSerial(lcd1, lcd2);              
            }
            for (; index < 16; index++)
            {
              sNewName[index] = ' ';
            }
            sNewName[16] = '\0';

            if (strcmp(sNewName, "                ") != 0)
              if (GetYesOrNo(false, sNewName))
              {
                if (iNumberOfEEPROM >= (EEPROM_LEGO_DNA_MAX - 1))
                {
                  lcd.setCursor(0, 0);
                  lcd.print(F("EEPROM DB full! "));
                  lcd.setCursor(0, 1);
                  lcd.print(F(" Push Button    "));

                  while (GetButtonPressed() == false)
                    ;
                }
                else
                {
#if DEBUG_OUTPUT
                  Serial.print(F("Adding to Database ["));
                  Serial.print(sDNASequence);
                  Serial.print(F("] = ["));
                  Serial.print(sNewName);
                  Serial.println(F("]"));
#endif

#if FILTER_BAD_WORDS
                  // Check for Bad Word
                  bool bBadWord = false;
                  for (int i = 0; i < NMB_BAD_WORDS; i++)
                  {
                    char sQuad[5] = "    ";
                    for (int j = 0; j < 13; j++)
                    {
                      int k;
                      strncpy(sQuad, &sNewName[j], 4);
                      //generate the MD5 hash for our string
                      unsigned char* hash = MD5::make_hash(sQuad);

                      for (k = 0; k < 16; k++)
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
                    lcd.print(F("Bad Word in Name"));
                    lcd.setCursor(0, 1);
                    lcd.print(F(" Push Button    "));
#if DEBUG_OUTPUT
                    Serial.println(F("BAD WORD!"));
#endif                    
                    while (GetButtonPressed() == false)
                      ;
                  }
                  else
#endif                  
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
      }
      iState = STATE_LOAD_TRAY;
    }
  }
}

void Advanced()
{
  while (GetButtonPressed() == false)
  {
    while (true)
    {
      lcd.setCursor(0, 0);
      lcd.print(F("Select Option:  "));

      int iAdvancedOp = ADVANCED_OP_CANCEL;

      while (true)
      {
        lcd.setCursor(0, 1);
        lcd.print(sAdvancedOp[iAdvancedOp]);

        if (GetButtonPressed())
          break;

        int iCounter = iRotaryEncoder_Counter;
        if (CheckRotaryEncoder())
        {
          if (iRotaryEncoder_Counter > iCounter)
          {
            iAdvancedOp++;
            if (iAdvancedOp  >= ADVANCED_NMB_OPS)
              iAdvancedOp = ADVANCED_OP_CANCEL;
          }
          else
          {
            iAdvancedOp--;
            if (iAdvancedOp < 0)
              iAdvancedOp = ADVANCED_OP_REBOOT;
          }
        }
      }
      
      int iResult;
      switch (iAdvancedOp)
      {
        case ADVANCED_OP_CANCEL:
          return;

        case ADVANCED_OP_CLEAR_CHANNEL:
          iResult = GetNumber(iCLEAR_CHANNEL_THRESHHOLD, "CLEAR", EEPROM_CLEAR_CHANNEL_THRESHHOLD);
          if (iResult > 0)
            iCLEAR_CHANNEL_THRESHHOLD = iResult;
          break;

        case ADVANCED_OP_RED_CHANNEL:
          iResult = GetNumber(iRED_CHANNEL_THRESHHOLD, "RED", EEPROM_RED_CHANNEL_THRESHHOLD);
          if (iResult > 0)
            iRED_CHANNEL_THRESHHOLD = iResult;
          break;

        case ADVANCED_OP_BLUE_CHANNEL:
          iResult = GetNumber(iBLUE_CHANNEL_THRESHHOLD, "BLUE", EEPROM_BLUE_CHANNEL_THRESHHOLD);
          if (iResult > 0)
            iBLUE_CHANNEL_THRESHHOLD = iResult;
          break;

        case ADVANCED_OP_AUTO_CHANNEL:
          if (sDNASequence[0] == '\0')
          {
            lcd.setCursor(0, 0);
            lcd.print(F("SCAN TEST LEGOS!"));
            lcd.setCursor(0, 1);
            lcd.print(F("PUSH BUTTON     "));
            while (GetButtonPressed() == false)
            {
              delay(100);
            }
            break;
          }
          if (GetYesOrNo(false, "SCAN YYRRBBGGYY?"))
          {
            char sTemp[17] = "";
            sprintf(sTemp,"C%d R%d B%d",iCLEAR_CHANNEL_THRESHHOLD, iRED_CHANNEL_THRESHHOLD,iBLUE_CHANNEL_THRESHHOLD);
            lcd.setCursor(0, 0);
            lcd.print(F("CURRENT CHANNELS"));
            lcd.setCursor(0, 1);
            lcd.print(F("                "));
            lcd.setCursor(0, 1);
            lcd.print(sTemp);
            while (GetButtonPressed() == false)
            {
              delay(100);
            }
            int iClear = iBrickSequencing[0][3];
            int iRed = iBrickSequencing[2][0];
            int iBlue = iBrickSequencing[4][2];
            if (iBrickSequencing[1][3] < iClear)
              iClear = iBrickSequencing[1][3];
            if (iBrickSequencing[3][0] < iRed)
              iRed = iBrickSequencing[3][0];
            if (iBrickSequencing[5][2] < iBlue)
              iBlue = iBrickSequencing[5][2];
            iClear = iClear - ((iClear * 1)/10);
            iRed = iRed - ((iRed * 1)/10);
            iBlue = iBlue - ((iBlue * 1)/10);
            sprintf(sTemp,"C%d R%d B%d?", iClear, iRed, iBlue);
            if (GetYesOrNo(false, sTemp))
            {
              iCLEAR_CHANNEL_THRESHHOLD = iClear;
              iRED_CHANNEL_THRESHHOLD = iRed;
              iBLUE_CHANNEL_THRESHHOLD = iBlue;
              writeUnsignedIntIntoEEPROM(EEPROM_CLEAR_CHANNEL_THRESHHOLD, iCLEAR_CHANNEL_THRESHHOLD);
              writeUnsignedIntIntoEEPROM(EEPROM_RED_CHANNEL_THRESHHOLD, iRED_CHANNEL_THRESHHOLD);
              writeUnsignedIntIntoEEPROM(EEPROM_BLUE_CHANNEL_THRESHHOLD, iBLUE_CHANNEL_THRESHHOLD);
            }
          }     
          break;

        case ADVANCED_OP_AUTO_POSITION:
        {
            lcd.setCursor(0, 0);
            lcd.print(F("USE TEST LEGO   "));
            lcd.setCursor(0, 1);
            lcd.print(F(" Push Button    "));
            while (GetButtonPressed() == false)
            {
              delay(100);
            }
            // Advance 25 steps at a time until red found and then red lost
            lcd.setCursor(0, 0);
            lcd.print(F("AUTO POSITIONING"));
            lcd.setCursor(0, 1);
            lcd.print(F("                "));
            
            tcs.setInterrupt(false);
            digitalWrite (LED_TCS34725, HIGH);

            MoveTray(OFFSET_TO_FIRST_BRICK);
            MoveTray(STEPS_PER_LEGO_BRICK);

            iStepPosition = 0;
            bool bWaitingForRED = true;
            int iStartPositionRED = 0;
            int iEndPositionRED = 0;
            while (iStepPosition < 4000)
            {
              MoveTray(STEPS_PER_AUTO_POSITION);
              lcd.setCursor(0, 1);
              char cColor = GetLEGOColor(0);
              if (bWaitingForRED == true &&
                  cColor == RED)
              {
//Serial.print("RED = "); Serial.println(iStepPosition);
                iStartPositionRED = iStepPosition;
                bWaitingForRED = false;
              }
              else
              if (bWaitingForRED == false &&
                  cColor != RED)
              {
//Serial.print("BLUE = "); Serial.println(iStepPosition);
                iEndPositionRED = iStepPosition;
                break;
              }
            }
            tcs.setInterrupt(true);
            digitalWrite (LED_TCS34725, LOW);
            
            if (iStartPositionRED == 0 || iEndPositionRED == 0)
            {
              lcd.setCursor(0, 0);
              lcd.print(F("AUTO POSITIONING"));
              lcd.setCursor(0, 1);
              lcd.print(F("   FAILURE      "));
              while (GetButtonPressed() == false)
              {
                delay(100);
              }
              break;                     
            }
            else
            {
              int iSecondREDStart = (iEndPositionRED + iStartPositionRED)/2;
//Serial.print("2nd RED = "); Serial.println(iSecondREDStart);
              MoveTray(iSecondREDStart-iStepPosition);
              int iMove = 0 - (STEPS_PER_LEGO_BRICK *2);
//Serial.print("iMove = "); Serial.println(iMove);
              iMove = iMove - OFFSET_TO_FIRST_BRICK;
//Serial.print("iMove = "); Serial.println(iMove);
              iMove = iMove - OFFSET_AUTO_POSITION;
//Serial.print("iMove = "); Serial.println(iMove);
              MoveTray(iMove);
            }
          break;
        }

        case ADVANCED_OP_REMOTE_LCD_ON:
          bRemoteLCDSerial = true;
          EEPROM[EEPROM_REMOTE_LCD_SERIAL] = bRemoteLCDSerial;
          re_set();
          break;

        case ADVANCED_OP_REMOTE_LCD_OFF:        
          bRemoteLCDSerial = false;
          EEPROM[EEPROM_REMOTE_LCD_SERIAL] = bRemoteLCDSerial;
          re_set();
          break;

        case ADVANCED_OP_REBOOT:
          re_set();
          break;  

        default:
          break;
      }
    }
  }
}

char GetNextChar(int index)
{
  int iCurrentCharIndex = NMB_NAME_CHARS - 1;
  char cCurrentChar = sNameChars[NMB_NAME_CHARS - 1];

  lcd.setCursor(index, 1);
  lcd.print(cCurrentChar);

  int iCurrentCounter = iRotaryEncoder_Counter;
  while (GetButtonPressed() == false)
  {
    if (CheckRotaryEncoder())
    {
      if (iCurrentCounter != iRotaryEncoder_Counter)
      {
        if (iRotaryEncoder_Counter > iCurrentCounter)
        {
          iCurrentCharIndex = iCurrentCharIndex + (iRotaryEncoder_Counter - iCurrentCounter);
          if (iCurrentCharIndex >= NMB_NAME_CHARS)
            iCurrentCharIndex = 0;
        }
        else
        {
          iCurrentCharIndex = iCurrentCharIndex - (iCurrentCounter - iRotaryEncoder_Counter);
          if (iCurrentCharIndex < 0)
            iCurrentCharIndex = NMB_NAME_CHARS - 1;
        }
        lcd.setCursor(index, 1);
        cCurrentChar = sNameChars[iCurrentCharIndex];
        lcd.print(cCurrentChar);
        iCurrentCounter = iRotaryEncoder_Counter;
      }
    }
  }
  if (cCurrentChar == '*')
    cCurrentChar = '\0';
  return (cCurrentChar);
}

char GetNextNumber(int index)
{
  int iCurrentCharIndex = NMB_NUMBER_CHARS - 1;
  char cCurrentChar = sNumberChars[NMB_NUMBER_CHARS - 1];

  lcd.setCursor(index, 1);
  lcd.print(cCurrentChar);

  int iCurrentCounter = iRotaryEncoder_Counter;
  while (GetButtonPressed() == false)
  {
    if (CheckRotaryEncoder())
    {
      if (iCurrentCounter != iRotaryEncoder_Counter)
      {
        if (iRotaryEncoder_Counter > iCurrentCounter)
        {
          iCurrentCharIndex = iCurrentCharIndex + (iRotaryEncoder_Counter - iCurrentCounter);
          if (iCurrentCharIndex >= NMB_NUMBER_CHARS)
            iCurrentCharIndex = 0;
        }
        else
        {
          iCurrentCharIndex = iCurrentCharIndex - (iCurrentCounter - iRotaryEncoder_Counter);
          if (iCurrentCharIndex < 0)
            iCurrentCharIndex = NMB_NUMBER_CHARS - 1;
        }
        lcd.setCursor(index, 1);
        cCurrentChar = sNumberChars[iCurrentCharIndex];
        lcd.print(cCurrentChar);
        iCurrentCounter = iRotaryEncoder_Counter;
      }
    }
  }
  if (cCurrentChar == '*')
    cCurrentChar = '\0';
  return (cCurrentChar);
}

void Beeps(int iNmbBeeps, int iMSLengthON, int iMSLengthOFF)
{
  if (bBuzzer == false)
    return;

  for (int i = 0; i < iNmbBeeps; i++)
  {
    digitalWrite(BUZZER, HIGH);
    delay(iMSLengthON);//wait for 1ms
    digitalWrite(BUZZER, LOW);
    delay(iMSLengthOFF);//wait for 1ms
  }
}

void MoveTray(int iSteps)
{
  iStepPosition = iStepPosition + iSteps;
  myStepper.step(iSteps);
}

#if 0
void TestNameEntry()
{
  Serial.println(F("Add to Database!"));
  lcd.setCursor(0, 0);
  lcd.print(F("Name:  "));
  lcd.write((byte) 0xff);
  lcd.print(F(" to end "));
  lcd.setCursor(0, 1);
  lcd.print(F("                "));

  char sNewName[17] = "";
  char cNextChar;

  int index = 0;
  while (index < 16)
  {
    cNextChar = GetNextChar(index);
    if (cNextChar == '\0')
    {
      break;
    }
    if (cNextChar == '<')
    {
      if (index > 0)
      {
        lcd.setCursor(index, 1);
        lcd.print(' ');
        index = index - 1;
        continue;
      }
    }
    sNewName[index] = cNextChar;
    index = index + 1;
    sNewName[index] = '\0';
  }
  for (; index < 16; index++)
  {
    sNewName[index] = ' ';
  }
  sNewName[16] = '\0';
  Serial.println(sNewName);
}
#endif

bool F_RemoteLCDSerial(const __FlashStringHelper *lcd1, const __FlashStringHelper *lcd2)
{
  if (bRemoteLCDSerial == false)
    return (false);

  Serial.print('[');
  Serial.print(lcd1);
  Serial.println("");
  Serial.print(lcd2);
  Serial.print(']'); 
}

bool RemoteLCDSerial(char *lcd1, char *lcd2)
{
  if (bRemoteLCDSerial == false)
    return (false);

  Serial.print('[');
  Serial.print(lcd1);
  Serial.println("");
  Serial.print(lcd2);
  Serial.print(']'); 
}
