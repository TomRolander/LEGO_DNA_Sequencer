
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
import tkinter as tk
import time
import serial
import argparse
from types import NoneType


def Draw():
    global text
    frame=tk.Frame(root,bd=1)
    frame.place(x=10,y=10)
    frame.pack(side="left")
    text=tk.Label(frame,text='', font=("Courier", 120))
    text.pack()
    text.configure(text= "  Welcome\n  LEGO DNA Sqncr")

def Refresher():
    global text

    if SerialObj.in_waiting != 0:
        letter = SerialObj.read()
        if letter == b'[':
            lcd1 = SerialObj.read(16)
            SerialObj.read(2)   #Ignore the '\r\n'
            lcd2 = SerialObj.read(16)
            SerialObj.read(1)   #Ignore the ']'
            #text.configure(text="0123456789012345\n" + time.asctime())
            text.configure(text= "  " + str(lcd1, 'UTF-8') + "\n  " + str(lcd2, 'UTF-8'))
            #text.configure(text= lcd1 + "\n" + lcd2)
    root.after(1000, Refresher) # every second...

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
    exit(1)

SerialObj = serial.Serial(args.comport) # COMxx   format on Windows

#SerialObj = serial.Serial('COM4') # COMxx   format on Windows
                                   # ttyUSBx format on Linux

SerialObj.baudrate = 115200  # set Baud rate to 9600
SerialObj.bytesize = 8     # Number of data bits = 8
SerialObj.parity   ='N'    # No parity
SerialObj.stopbits = 1     # Number of Stop bits = 1

root=tk.Tk()
root.title("LEGO DNA Sequencer")
root.geometry("1540x340")
root.attributes('-fullscreen', True)
label = tk.Label(root, text=" LEGO DNA Sequencer ", font=("Courier", 48), bg="black", fg="white")
label.pack()

Draw()
Refresher()
root.mainloop()