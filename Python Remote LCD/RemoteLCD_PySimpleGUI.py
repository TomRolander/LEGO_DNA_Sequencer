"""
/**************************************************************************
  RemoteLDC_PySimpleGUI
    The purpose of this Python program is to remotely display the
  contents of an Arduino 1602 LCD.  It was created to facilitate 
  a large group of persons watching the LEGO DNA Sequencer.

  Original Code:  2023-01-08
  Revision:       2022-01-10

  Tom Rolander, MSEE
  Mentor, Circuit Design & Software
  Miller Library, Fabrication Lab
  Hopkins Marine Station, Stanford University,
  120 Ocean View Blvd, Pacific Grove, CA 93950
  +1 831.915.9526 | rolander@stanford.edu

 **************************************************************************/

"""
Program = "RemoteLCD_PySimpleGUI"
Version = "Ver 0.3"
RevisionDate = "2023-01-16"

import sys
import os
import time
import serial
import argparse
from types import NoneType
import PySimpleGUI as sg

def getbackgroundcolor(x):
    if chr(x) == 'A':
        return 'green'
    if chr(x) == 'C':
        return 'yellow'
    if chr(x) == 'G':
        return 'red'
    if chr(x) == 'T':
        return 'blue'

def gettextcolor(x):
    if chr(x) == 'C':
        return 'black'
    else:
        return 'white'

print (Program, Version, RevisionDate)

parser = argparse.ArgumentParser("RemoteLCD")
parser.add_argument('--comport', type=str, required=False, help="COM port")
parser.add_argument('--showports', required=False, choices=('True','False'))
args = parser.parse_args()

if type(args.showports) is not NoneType:
    os.system("python -m serial.tools.list_ports")
    exit(0)

if type(args.comport) is NoneType:
    print ("--comport <COMPORT> is required!")
    print ("--showports True   will show available comports")
    exit(1)

print ("Connecting to LEGO DNA Sequencer on Arduino on Port",args.comport)

SerialObj = serial.Serial(args.comport) # COMxx   format on Windows
#SerialObj = serial.Serial('COM4') # COMxx   format on Windows
                                   # ttyUSBx format on Linux

SerialObj.baudrate = 115200  # set Baud rate to 9600
SerialObj.bytesize = 8     # Number of data bits = 8
SerialObj.parity   ='N'    # No parity
SerialObj.stopbits = 1     # Number of Stop bits = 1


sg.theme('Black')   
layout = [  [sg.Text('LEGO DNA Sqncr', font=("Courier",120), key='LINE1')],
            [sg.Text(' Push Button', font=("Courier",120), key='LINE2'),
            sg.Text(' ', font=("Courier",120), key='LINE2-COL0'),
            sg.Text(' ', font=("Courier",120), key='LINE2-COL1'),
            sg.Text(' ', font=("Courier",120), key='LINE2-COL2'),
            sg.Text(' ', font=("Courier",120), key='LINE2-COL3'),
            sg.Text(' ', font=("Courier",120), key='LINE2-COL4'),
            sg.Text(' ', font=("Courier",120), key='LINE2-COL5'),
            sg.Text(' ', font=("Courier",120), key='LINE2-COL6'),
            sg.Text(' ', font=("Courier",120), key='LINE2-COL7'),
            sg.Text(' ', font=("Courier",120), key='LINE2-COL8'),
            sg.Text(' ', font=("Courier",120), key='LINE2-COL9'),
            sg.Text(' ', font=("Courier",120), key='LINE2-COL10'),
            sg.Text(' ', font=("Courier",120), key='LINE2-COL11'),
            sg.Text(' ', font=("Courier",120), key='LINE2-COL12'),
            sg.Text(' ', font=("Courier",120), key='LINE2-COL13'),
            sg.Text(' ', font=("Courier",120), key='LINE2-COL14'),
            sg.Text(' ', font=("Courier",120), key='LINE2-COL15')],
            [sg.Text('                ', font=("Courier",120), key='LINE3')]]

# Create the Window
window = sg.Window('LEGO DNA Sequencer', layout, margins=(50,250)).Finalize()
window.Maximize()

# Event Loop to process "events"
while True:
    event, values = window.read(timeout=10)
    if event == sg.WIN_CLOSED: # if user closes window
        break
    if SerialObj.in_waiting != 0:
        letter = SerialObj.read()
        if letter == b'[':
            lcd1 = SerialObj.read(16)
            SerialObj.read(2)   #Ignore the '\r\n'
            lcd2 = SerialObj.read(16)
            SerialObj.read(1)   #Ignore the ']'
            window['LINE1'].update(str(lcd1, 'UTF-8'))
            if str(lcd1, 'UTF-8') == "Successful match":
                window['LINE3'].update(str(lcd2, 'UTF-8'))
            else:
                if str(lcd1, 'UTF-8') == "Sequencing DNA  " or str(lcd1, 'UTF-8') == "Unloading tray  ":
                    window['LINE2'].update('  ')
                    window['LINE3'].update('                ')
                    for x in range(10):
                        window['LINE2-COL'+str(x)].update(background_color='black')
                        window['LINE2-COL'+str(x)].update(' ')
                    n = range(10, 16)
                    for x in n:
                        window['LINE2-COL'+str(x)].update(background_color='black')
                        window['LINE2-COL'+str(x)].update(' ')
                    for x in range(10):
                        backgroundcolor = getbackgroundcolor(lcd2[x])
                        window['LINE2-COL'+str(x)].update(background_color=backgroundcolor)
                        textcolor = gettextcolor(lcd2[x])
                        window['LINE2-COL'+str(x)].update(text_color=textcolor)
                        window['LINE2-COL'+str(x)].update(font=("Courier",120, 'bold'))                    
                        window['LINE2-COL'+str(x)].update(chr(lcd2[x]))
                else:
                    window['LINE2'].update(background_color='black')
                    window['LINE2'].update(text_color='white')
                    window['LINE2'].update(font=("Courier",120, 'normal'))                    
                    window['LINE2'].update(str(lcd2, 'UTF-8')+'        ')
                    window['LINE3'].update('                ')

window.close()