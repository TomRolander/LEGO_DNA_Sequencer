"""
/**************************************************************************
  RemoteLDC
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
Program = "RemoteLCD"
Version = "Ver 0.1"
RevisionDate = "2023-01-10"

import sys
import os
import time
import serial
import argparse
from types import NoneType
import PySimpleGUI as sg

def getcolor(x):
    if chr(x) == 'A':
        return 'green'
    if chr(x) == 'C':
        return 'yellow'
    if chr(x) == 'G':
        return 'red'
    if chr(x) == 'T':
        return 'blue'



SerialObj = serial.Serial('COM4') # COMxx   format on Windows
                                   # ttyUSBx format on Linux

SerialObj.baudrate = 115200  # set Baud rate to 9600
SerialObj.bytesize = 8     # Number of data bits = 8
SerialObj.parity   ='N'    # No parity
SerialObj.stopbits = 1     # Number of Stop bits = 1


sg.theme('Black')   # Add a touch of color
# All the stuff inside your window.
layout = [  [sg.Text('LEGO DNA Sqncr', font=("Courier",120), key='LINE1')],
            [sg.Text(' Push Button', font=("Courier",120), key='LINE2')],
            [sg.Text(' ', font=("Courier",120), key='LINE3-0'), sg.Text(' ', font=("Courier",120), key='LINE3-1'), sg.Text(' ', font=("Courier",120), key='LINE3-2'), sg.Text(' ', font=("Courier",120), key='LINE3-3'), sg.Text(' ', font=("Courier",120), key='LINE3-4'), sg.Text(' ', font=("Courier",120), key='LINE3-5'), sg.Text(' ', font=("Courier",120), key='LINE3-6'), sg.Text(' ', font=("Courier",120), key='LINE3-7'), sg.Text(' ', font=("Courier",120), key='LINE3-8'), sg.Text(' ', font=("Courier",120), key='LINE3-9')]
         ]

# Create the Window
window = sg.Window('LEGO DNA Sequencer', layout, margins=(150,250)).Finalize()
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
            window['LINE1'].update(str(lcd1, 'UTF-8'));
            if str(lcd1, 'UTF-8') == "Sequencing DNA  ":
                #window['LINE3-0'].update(text_color='black');
                color = getcolor(lcd2[0])
                window['LINE3-0'].update(background_color=color);
                window['LINE3-0'].update(chr(lcd2[0]));
                color = getcolor(lcd2[1])
                window['LINE3-1'].update(background_color=color);
                window['LINE3-1'].update(chr(lcd2[1]));
                color = getcolor(lcd2[2])
                window['LINE3-2'].update(background_color=color);
                window['LINE3-2'].update(chr(lcd2[2]));
                color = getcolor(lcd2[3])
                window['LINE3-3'].update(background_color=color);
                window['LINE3-3'].update(chr(lcd2[3]));
                color = getcolor(lcd2[4])
                window['LINE3-4'].update(background_color=color);
                window['LINE3-4'].update(chr(lcd2[4]));
                color = getcolor(lcd2[5])
                window['LINE3-5'].update(background_color=color);
                window['LINE3-5'].update(chr(lcd2[5]));
                color = getcolor(lcd2[6])
                window['LINE3-6'].update(background_color=color);
                window['LINE3-6'].update(chr(lcd2[6]));
                color = getcolor(lcd2[7])
                window['LINE3-7'].update(background_color=color);
                window['LINE3-7'].update(chr(lcd2[7]));
                color = getcolor(lcd2[8])
                window['LINE3-8'].update(background_color=color);
                window['LINE3-8'].update(chr(lcd2[8]));
                color = getcolor(lcd2[9])
                window['LINE3-9'].update(background_color=color);
                window['LINE3-9'].update(chr(lcd2[9]));
                window['LINE2'].update("                ");
            else:
                window['LINE2'].update(str(lcd2, 'UTF-8'));

window.close()