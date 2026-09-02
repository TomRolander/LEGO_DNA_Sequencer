"""
/**************************************************************************
  RemoteLDC_FreeSimpleGUI
    The purpose of this Python program is to remotely display the
  contents of an Arduino 1602 LCD.  It was created to facilitate 
  a large group of persons watching the LEGO DNA Sequencer.

  Original Code:  2023-01-08
  Revision:       2025-01-10

  Tom Rolander, MSEE
  Mentor, Circuit Design & Software
  Miller Library, Fabrication Lab
  Hopkins Marine Station, Stanford University,
  120 Ocean View Blvd, Pacific Grove, CA 93950
  +1 831.915.9526 | rolander@stanford.edu

 **************************************************************************/

"""
Program = "RemoteLCD_FreeSimpleGUI"
Version = "Ver 0.5"
RevisionDate = "2025-02-27"

import sys
import os
import time
import serial
import argparse
from types import NoneType
import FreeSimpleGUI as sg

def get_scaled_dimensions(manual_scale=1.0, reference_width=1920, reference_height=1080):
    """
    Calculate scaling factor based on screen resolution.
    
    Args:
        manual_scale: Manual override scale factor
        reference_width: Reference screen width (default: 1920)
        reference_height: Reference screen height (default: 1080)
    
    Returns:
        Dictionary with scaled dimensions
    """
    screen_width, screen_height = sg.Window.get_screen_size()
    
    # Calculate automatic scale factor
    scale_x = screen_width / reference_width
    scale_y = screen_height / reference_height
    auto_scale = min(scale_x, scale_y)
    
    # Apply manual scale override
    final_scale = auto_scale * manual_scale
    
    print(f"Screen Resolution: {screen_width}x{screen_height}")
    print(f"Reference Resolution: {reference_width}x{reference_height}")
    print(f"Auto Scale Factor: {auto_scale:.2f}")
    print(f"Manual Scale Factor: {manual_scale:.2f}")
    print(f"Final Scale Factor: {final_scale:.2f}")
    
    return {
        'font_size': int(120 * final_scale),
        'margin_x': int(50 * final_scale),
        'margin_y': int(250 * final_scale),
        'scale_factor': final_scale
    }

def getbackgroundcolor(x):
    """Return background color for DNA base."""
    color_map = {
        'A': 'green',
        'C': 'yellow',
        'G': 'red',
        'T': 'blue'
    }
    return color_map.get(chr(x), 'black')

def gettextcolor(x):
    """Return text color for DNA base."""
    return 'black' if chr(x) == 'C' else 'white'

print(Program, Version, RevisionDate)

# Parse command line arguments
parser = argparse.ArgumentParser("RemoteLCD")
parser.add_argument('--comport', type=str, required=False, help="COM port")
parser.add_argument('--showports', action='store_true',
                    help="Show available COM ports")
parser.add_argument('--scale', type=float, default=1.0,
                    help="Manual scale factor (default: 1.0)")
parser.add_argument('--refwidth', type=int, default=1920,
                    help="Reference screen width (default: 1920)")
parser.add_argument('--refheight', type=int, default=1080,
                    help="Reference screen height (default: 1080)")
args = parser.parse_args()

if args.showports:
    # Use serial.tools.list_ports directly in Python
    import serial.tools.list_ports
    ports = serial.tools.list_ports.comports()
    print("\nAvailable COM ports:")
    for port in ports:
        print(f"  {port.device} - {port.description}")
    exit(0)

if type(args.comport) is NoneType:
    print("--comport <COMPORT> is required!")
    print("--showports        will show available comports")
    print("\nOptional scaling arguments:")
    print("--scale <factor>      Manual scale multiplier (e.g., 0.8 or 1.2)")
    print("--refwidth <pixels>   Reference width (default: 1920)")
    print("--refheight <pixels>  Reference height (default: 1080)")
    exit(1)

print("Connecting to LEGO DNA Sequencer on Arduino on Port", args.comport)

# Initialize serial connection
try:
    SerialObj = serial.Serial(args.comport)
    SerialObj.baudrate = 115200
    SerialObj.bytesize = 8
    SerialObj.parity = 'N'
    SerialObj.stopbits = 1
    print("Serial connection established successfully")
except Exception as e:
    print(f"Error connecting to serial port: {e}")
    exit(1)

# Get scaled dimensions
dims = get_scaled_dimensions(args.scale, args.refwidth, args.refheight)
font_size = dims['font_size']

print(f"Using font size: {font_size}")

# Setup GUI
sg.theme('Black')

# Build layout with scaled fonts
layout = [
    [sg.Text('LEGO DNA Sqncr', font=("Courier", font_size), key='LINE1')],
    [sg.Text(' Push Button', font=("Courier", font_size), key='LINE2')] +
    [sg.Text(' ', font=("Courier", font_size), key=f'LINE2-COL{i}') 
     for i in range(16)],
    [sg.Text('                ', font=("Courier", font_size), key='LINE3')]
]

# Create the Window
window = sg.Window('LEGO DNA Sequencer', 
                   layout, 
                   margins=(dims['margin_x'], dims['margin_y']),
                   resizable=True).Finalize()
window.Maximize()

print("Window created and maximized")

# Event Loop to process "events"
try:
    while True:
        event, values = window.read(timeout=10)
        if event == sg.WIN_CLOSED:  # if user closes window
            break
            
        if SerialObj.in_waiting != 0:
            letter = SerialObj.read()
            if letter == b'[':
                lcd1 = SerialObj.read(16)
                SerialObj.read(2)   # Ignore the '\r\n'
                lcd2 = SerialObj.read(16)
                SerialObj.read(1)   # Ignore the ']'
                
                window['LINE1'].update(str(lcd1, 'UTF-8'))
                
                if str(lcd1, 'UTF-8') == "Successful match":
                    window['LINE3'].update(str(lcd2, 'UTF-8'))
                else:
                    if str(lcd1, 'UTF-8') in ["Sequencing DNA  ", "Unloading tray  "]:
                        # Reset LINE2 and LINE3
                        window['LINE2'].update('  ')
                        window['LINE3'].update('                ')
                        
                        # Clear all column backgrounds
                        for x in range(16):
                            window[f'LINE2-COL{x}'].update(background_color='black')
                            window[f'LINE2-COL{x}'].update(' ')
                        
                        # Update first 10 columns with DNA base colors
                        for x in range(10):
                            backgroundcolor = getbackgroundcolor(lcd2[x])
                            textcolor = gettextcolor(lcd2[x])
                            
                            window[f'LINE2-COL{x}'].update(background_color=backgroundcolor)
                            window[f'LINE2-COL{x}'].update(text_color=textcolor)
                            window[f'LINE2-COL{x}'].update(font=("Courier", font_size, 'bold'))
                            window[f'LINE2-COL{x}'].update(chr(lcd2[x]))
                    else:
                        # Display normal text in LINE2
                        window['LINE2'].update(background_color='black')
                        window['LINE2'].update(text_color='white')
                        window['LINE2'].update(font=("Courier", font_size, 'normal'))
                        window['LINE2'].update(str(lcd2, 'UTF-8') + '        ')
                        window['LINE3'].update('                ')

except KeyboardInterrupt:
    print("\nProgram interrupted by user")
except Exception as e:
    print(f"\nError during execution: {e}")
finally:
    window.close()
    SerialObj.close()
    print("Program terminated cleanly")