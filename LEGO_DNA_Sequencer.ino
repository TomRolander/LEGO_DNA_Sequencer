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
#define VERSION F("Ver 0.9 2023-01-05")
#define PROGRAM_SHORT F("LEGO DNA Sqncr  ")
#define VERSION_SHORT F("Ver 0.9 01-05-23")

#define DEBUG_OUTPUT 1
#define DEBUG_MODE   0

#define FLORA 0

#define STATE_START       0
#define STATE_LOAD_TRAY   1
#define STATE_SEQUENCING  2

static int iState = STATE_START;

static int iStepPosition = 0;

static bool bAutomated = true;

#define BUZZER        A0

#define LED_TCS34725  A3

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

char sNameChars[40] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789 <\xff";
//                     0123456789012345678901234567890123456789
//                     0         1         2         3
#define NMB_NAME_CHARS  39

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
#define EEPROM_C_THRESHHOLD                 4 // 004-005  iC_THRESHHOLD
#define EEPROM_R_THRESHHOLD                 6 // 006-007  iR_THRESHHOLD
#define EEPROM_B_THRESHHOLD                 8 // 008-009  iB_THRESHHOLD
#define EEPROM_NUMBER_OF_LEGO_DNA          10 // 010-010  iNumberOfEEPROM
#define EEPROM_LEGO_DNA                    11 // 011-511  LEGO_DNAs

#define EEPROM_LEGO_DNA_MAX                10
#define EEPROM_LEGO_DNA_ENTRY_SIZE         28

//#define LEGO_DNA_MAX  24
#define LEGO_DNA_MAX  16

static int iNumberOfEEPROM = 0;

#define NMB_LEGO_BRICKS 10

int iBrickSequencing[NMB_LEGO_BRICKS][4];
char sDNASequence[NMB_LEGO_BRICKS+1];


int iLEGO_DNA_Number = 7;
char sLEGO_DNA_Sequence[LEGO_DNA_MAX][NMB_LEGO_BRICKS+1] =
{ "ACGTACGTAC",
  "ATTGGTCATT",
  "TGCTCCTACA",
  "CACAATCTAC",
  "GCTCCCGGGT",
  "CAAATCTTAG",
  "CGTCTACCAA"
};
char sLEGO_DNA_Name[LEGO_DNA_MAX][17] =
{ " ACGTACGTAC TEST",
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

#define STEPS_PER_LEGO_BRICK  550

// Creates an instance of stepper class
// Pins entered in sequence IN1-IN3-IN2-IN4 for proper step sequence
Stepper myStepper = Stepper(stepsPerRevolution, STEPPER_PIN_1, STEPPER_PIN_3, STEPPER_PIN_2, STEPPER_PIN_4);

#define MAINTENANCE_NMB_OPS           9
#define MAINTENANCE_OP_CANCEL         0
#define MAINTENANCE_OP_COMMAND_LINE   1
#define MAINTENANCE_OP_UNLOAD_TRAY    2
#define MAINTENANCE_OP_LOAD_TRAY      3
#define MAINTENANCE_OP_POSITION_TRAY  4
#define MAINTENANCE_OP_ZERO_NEW_DNA   5
#define MAINTENANCE_OP_BUZZER_OFF     6
#define MAINTENANCE_OP_BUZZER_ON      7
#define MAINTENANCE_OP_VERSION_INFO   8

char sMaintenanceOp[MAINTENANCE_NMB_OPS][17] =
{ "  CANCEL        ",
  "  COMMAND LINE  ",
  "  UNLOAD TRAY   ",
  "  LOAD TRAY     ",
  "  POSITION TRAY ",
  "  ZERO NEW DNA  ",
  "  BUZZER OFF    ",
  "  BUZZER ON     ",
  "  VERSION INFO  "
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
#define C_THRESHHOLD  130
#define R_THRESHHOLD  35
#define B_THRESHHOLD  30
#else
#define C_THRESHHOLD  900
#define R_THRESHHOLD  170
#define B_THRESHHOLD  220
#endif

static int iC_THRESHHOLD = C_THRESHHOLD;
static int iR_THRESHHOLD = R_THRESHHOLD;
static int iB_THRESHHOLD = B_THRESHHOLD;

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println();
  Serial.println(PROGRAM);
  Serial.println(VERSION);

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
  lcd.print(F("LEGO DNA Sqncr  "));
  lcd.setCursor(0, 1);
  lcd.print(F("Debugger Console"));
#else
  StartSequencer();
#endif

for (int i=0; i<128; i++)
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

  if (EEPROM[EEPROM_SIGNATURE + 0] == 'L' &&
      EEPROM[EEPROM_SIGNATURE + 1] == 'E' &&
      EEPROM[EEPROM_SIGNATURE + 2] == 'G' &&
      EEPROM[EEPROM_SIGNATURE + 3] == 'O')
  {
    iC_THRESHHOLD = readUnsignedIntFromEEPROM(EEPROM_C_THRESHHOLD);
    iR_THRESHHOLD = readUnsignedIntFromEEPROM(EEPROM_R_THRESHHOLD);
    iB_THRESHHOLD = readUnsignedIntFromEEPROM(EEPROM_B_THRESHHOLD);
    
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
Serial.println(F("]"));
      
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
  Serial.print(F("iNumberOfEEPROM = "));
  Serial.println(iNumberOfEEPROM);
  Serial.print(F("C Threshhold = "));
  Serial.println(iC_THRESHHOLD);
  Serial.print(F("R Threshhold = "));
  Serial.println(iR_THRESHHOLD);
  Serial.print(F("B Threshhold = "));
  Serial.println(iB_THRESHHOLD);
#endif

  Beeps(3, 10, 200);  // 3 Beeps, 10MS ON, 200MS OFF
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
  writeUnsignedIntIntoEEPROM(EEPROM_C_THRESHHOLD, iC_THRESHHOLD);
  writeUnsignedIntIntoEEPROM(EEPROM_R_THRESHHOLD, iR_THRESHHOLD);
  writeUnsignedIntIntoEEPROM(EEPROM_B_THRESHHOLD, iB_THRESHHOLD);
  EEPROM[EEPROM_NUMBER_OF_LEGO_DNA] = iNumberOfEEPROM;

#if DEBUG_OUTPUT
  Serial.println(F("EEPROM LEGO initialized"));
#endif

}
void UpdateLCD(char cColor)
{
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
  
#if 0
  Serial.print(F("Red: ")); Serial.print(r, DEC); Serial.print(F(" "));
  Serial.print(F("Green: ")); Serial.print(g, DEC); Serial.print(F(" "));
  Serial.print(F("Blue: ")); Serial.print(b, DEC); Serial.print(F(" "));
  Serial.print(F("Color: ")); Serial.print(c, DEC); Serial.print(F(" "));
#else
  Serial.print(iStepPosition, DEC); Serial.print(F(","));
  Serial.print(r, DEC); Serial.print(F(","));
  Serial.print(g, DEC); Serial.print(F(","));
  Serial.print(b, DEC); Serial.print(F(","));
  Serial.print(c, DEC); Serial.print(F(","));
#endif



  if (c >= iC_THRESHHOLD)
  {
    UpdateLCD(YELLOW);
    cRetcode = YELLOW;
    Serial.println(F("Y,C"));
  }
  else
  if (r >= iR_THRESHHOLD)
  {
    UpdateLCD(RED);
    cRetcode = RED;
    Serial.println(F("R,G"));
  }
  else
  if (b >= iB_THRESHHOLD)
  {  
    UpdateLCD(BLUE);
    cRetcode = BLUE;
    Serial.println(F("B,T"));
  }
  else
  /* if (g > b && g > r) */
  {
    UpdateLCD(GREEN);
    cRetcode = GREEN;
    Serial.println(F("G,A"));    
  }  
  
  return (cRetcode);
}

void StartSequencer()
{
  lcd.setCursor(0, 0);
  lcd.print(F("LEGO DNA Sqncr  "));

  lcd.setCursor(0, 1);
  lcd.print(F(" Push Button    "));
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

  lcd.setCursor(0, 0);
  lcd.print(F("                "));
  lcd.setCursor(0, 0);
  lcd.print(sQuestion);
  lcd.setCursor(0, 1);
  if (bInitialResponse == false)
    lcd.print(F("[NO]  YES       "));
  else
    lcd.print(F(" NO  [YES]      "));

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
        }
        bResponse = false;
      }
      else
      {
        if (bResponse != true)
        {
          lcd.setCursor(0, 1);
          lcd.print(F(" NO  [YES]      "));
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
  delay(10);
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
    lcd.setCursor(0, 0);
    lcd.print(F("Load LEGO Tray  "));

    Beeps(1, 5, 0);  // 1 Beep, 5MS ON, 0MS OFF    

    lcd.setCursor(0, 1);
    lcd.print(F(" Push Button    "));

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
Serial.println(F("Cancel"));
            StartSequencer();
            iState = STATE_START;
            bCommandLineMode = false;
            break;

          case MAINTENANCE_OP_COMMAND_LINE:
Serial.println(F("Enter Command Line Mode"));
            bCommandLineMode = true;
            break;

          case MAINTENANCE_OP_UNLOAD_TRAY:
Serial.println(F("Unload Tray"));
            myStepper.step(-4000);
            iStepPosition = 0;
            break;
            
          case MAINTENANCE_OP_LOAD_TRAY:
Serial.println(F("Load Tray"));
            myStepper.step(1640);
            iStepPosition = 0;
            break;

          case MAINTENANCE_OP_POSITION_TRAY:
            {
             int iCounter = iRotaryEncoder_Counter;
              while (GetButtonPressed() == false)
              {
                if (CheckRotaryEncoder())
                {
#if DEBUG_OUTPUT                
Serial.print(F("iCounter = "));
Serial.print(iCounter);
Serial.print(F("  iRotaryEncoder_Counter = "));
Serial.println(iRotaryEncoder_Counter);
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
            for (int i=EEPROM_NUMBER_OF_LEGO_DNA; i<512; i++)
              EEPROM[i] = '\0';
            SetupEEPROM();
            break;

          case MAINTENANCE_OP_BUZZER_ON:
            bBuzzer = true;
            break;

          case MAINTENANCE_OP_BUZZER_OFF:
            bBuzzer = false;
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

  lcd.setCursor(0, 1);


  // STATE_SEQUENCING
  tcs.setInterrupt(false);
  digitalWrite (LED_TCS34725, HIGH);

  myStepper.setSpeed(10);
  iStepPosition = 0;
  MoveTray(460);
  
  sDNASequence[NMB_LEGO_BRICKS] = '\0';

  Serial.println("");
  Serial.println(F("---------- CSV ----------"));
  Serial.println(F("P,R,G,B,Y,M,DNA"));  

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

      Beeps(2, 5, 250);  // 2 Beeps, 5MS ON, 250MS OFF

      myStepper.setSpeed(15);
      MoveTray(-5410);

      Serial.print("[");
      Serial.print(sDNASequence);
      Serial.println("]");
      for (int i=0; i<NMB_LEGO_BRICKS; i++)
      {
        for (int j=0; j<4; j++)
        {
          Serial.print(iBrickSequencing[i][j]);
          Serial.print(" ");
        }
        Serial.println("");
      }
      
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
          
          Beeps(3, 5, 200);  // 3 Beeps, 5MS ON, 200MS OFF
        }
        else
        {
          lcd.setCursor(0, 0);
          lcd.print(F("DNA not a match "));
          lcd.setCursor(0, 1);
          lcd.print(F("  UNKNOWN DNA!  "));
  
          Beeps(4, 10, 200);  // 4 Beeps, 10MS ON, 200MS OFF
        }
      
        while (GetButtonPressed() == false)
          ;
  
        if (index >= iLEGO_DNA_Number)
        {
          if (GetYesOrNo(false, "Add to Database?"))
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
                  lcd.print(F("Bad Word in Name"));
                  lcd.setCursor(0, 1);
                  lcd.print(F(" Push Button    "));
  Serial.println(F("BAD WORD!"));                
                  while (GetButtonPressed() == false)
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
      }
      iState = STATE_LOAD_TRAY;
    }
  }
}

char GetNextChar(int index)
{
  int iCurrentCharIndex = NMB_NAME_CHARS-1;
  char cCurrentChar = sNameChars[NMB_NAME_CHARS-1];

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
            iCurrentCharIndex = NMB_NAME_CHARS-1;
        }
        lcd.setCursor(index, 1);
        cCurrentChar = sNameChars[iCurrentCharIndex];
        lcd.print(cCurrentChar);
        iCurrentCounter = iRotaryEncoder_Counter;
      }
    }
  }
  if (cCurrentChar == '\xff')
    cCurrentChar = '\0';
  return (cCurrentChar);
}

void Beeps(int iNmbBeeps, int iMSLengthON, int iMSLengthOFF)
{
  if (bBuzzer == false)
    return;

  for (int i=0; i<iNmbBeeps; i++)
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
