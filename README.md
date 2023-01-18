# LEGO DNA Sequencer and DIY Construction

![LEGO DNA Sequencer](https://github.com/TomRolander/LEGO_DNA_Sequencer/blob/main/Images/LEGO_DNA_Sequencer.jpg)

# Table of Contents
  
&emsp;[Overview](#overview)  
&emsp;[Documentation](#documentation)  
&emsp;&emsp;[LEGO DNA Sequencer and DIY Construction](Documentation/LEGO%20DNA%20Sequencer%20and%20DIY%20Construction.pdf)  
&emsp;&emsp;[LEGO DNA Sequencer User Manual](Documentation/LEGO%20DNA%20Sequencer%20User%20Manual.pdf)  
&emsp;[Arduino Source Code](Arduino%20Code)  
&emsp;[Arduino Breadboard](#arduino-breadboard)  
&emsp;[Arduino Schematic](#arduino-schematic)  
&emsp;[Python Remote LCD](#python-remote-lcd)  
&emsp;[STL Files for 3D Printing](STL)  


## Overview
The purpose of this LEGO DNA Sequencer and DIY Construction is to provide a complete blueprint for an open source tool that can be used to explain the complex DNA sequencing operation with a colorful easy to understand model.

All of the necessary information is provided to build your own LEGO DNA Sequencer.  The electronics assembly, 3D printed parts, software, and assembly steps are described in detail.  Also a complete Users Manual describes the operation of the LEGO DNA Sequencer.

DNA sequencing determines the order of the four chemical building blocks - called "bases" - that make up the DNA molecule.  The building blocks are composed of A (adenine), C (cytosine), G (guanine), and T (thymine).  These building blocks are represented by LEGO® 2x2 bricks of Red for G, Blue for T, Green for A, and Yellow for C. Actual DNA sequencing is greatly simplified in this model by using genomes of only 10 building blocks to identify a species.

It is beyond the scope of this document to provide more details regarding actual DNA sequencing.

[LEGO DNA Sequencer Video Demo](Images/LEGO_DNA_Sequencer%20Demo.mp4)

 ## Documentation
 The documentation for the LEGO DNA Sequencer includes a complete PDF with the DIY and also a separate extracted User Manual.

[LEGO DNA Sequencer and DIY Construction](Documentation/LEGO%20DNA%20Sequencer%20and%20DIY%20Construction.pdf)

[LEGO DNA Sequencer User Manual](Documentation/LEGO%20DNA%20Sequencer%20User%20Manual.pdf)

## Arduino Breadboard

![Arduino Breadboard](Images/LEGO%20DNA%20Sequencer_bb.jpg)

## Arduino Schematic

![Arduino Schematic](Images/LEGO%20DNA%20Sequencer_schem.jpg)

## Python Remote LCD

In a classroom setting for demonstrations with the LEGO DNA Sequencer this Python code will emulate the Arduino LCD on a large screen.

![Remote LCD](Images/RemoteLCD.jpg)

[RemoteLCD PySimpleGUI Version](Python%20Remote%20LCD/RemoteLCD_PySimpleGUI.py)

[RemoteLCD tkinter Version](Python%20Remote%20LCD/RemoteLCD_tkinter.py)