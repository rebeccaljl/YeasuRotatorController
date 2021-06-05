This repository keep track the development process of the Yaesu Arduino Controller. The controller consist of three parts:

1. The Yaesu Rotator driver G5500 and the Yaesu Rotator, datasheet see here: https://www.yaesu.com/downloadFile.cfm?FileID=8814&FileCatID=155&FileName=G%2D5500%5FIM%5FENG%5FE12901004.pdf&FileContentType=application%2Fpdf
2. The rotator and computer interface - namely our Arduino controller.
3. The PC which has a satellite tracking application and is able to port out satellite predicted data to the COM port (USB port)

The circuit of the Arduino Controller goes like this:

Yaesu Rotator External Control Port (Din8):
PIN 1 --> Provide analogue voltage from 2 to 4.5VDC corresponding to 0 to 180 Elevation degree\
PIN 2 --> Short this pin to ground to move zimuth clockwis\
PIN 3 <-- Short this pin to ground to move up\
PIN 4 <-- Short this pin to gorund to move azimuth counterclockwise\
PIN 5 <-- Short this pin to ground to move down\
PIN 6 --> Provide analogue voltage from 2 to 4.5VDC corresponding to 0 to 450 Azimuth degree\
PIN 7 --- Provide 13VDC to 6VDC @ 200mA\
PIN 8 --- Commond Ground with arduino\

--------------------------------------------------------------------------------------------------------

Arduino
A0 <-- Read azimuth degree\
A1 <-- Read elevation degree\
D4 --> Write to Yaesu Controller (Left)\
D5 --> Write to Yaesu Controller (Right)\
D6 --> Write to Yaesu Controller (Up)\
D7 --> Write to Yaesu Controller (Down)\
Ground --- Common ground with Arduino\
USB Serial Port <-- Receiving Satellite information from laptop

--------------------------------------------------------------------------------------------------------

Laptop
Connect USB port and satellite prediction software, such as SATPC32, and start sending satellite position to Arduino.

You might need a circuit to invert the logic in the code, because physically you need to write low in order
for the controller to rotate, but our code write "HIGH" (5V). 

The circuit can refer to: https://blog.radioartisan.com/yaesu-rotator-computer-serial-interface/

Regards,
JY
