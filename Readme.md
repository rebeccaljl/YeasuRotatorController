This repository keep track the development process of the Yeasu Arduino Controller. The controller consist of three parts:

1. The Yeasu Rotator driver G5500 and the Yeasu Rotator, datasheet see here: https://www.yaesu.com/downloadFile.cfm?FileID=8814&FileCatID=155&FileName=G%2D5500%5FIM%5FENG%5FE12901004.pdf&FileContentType=application%2Fpdf
2. The rotator and computer interface - namely our Arduino controller.
3. The PC which has a satellite tracking application and is able to port out satellite predicted data to the COM port (USB port)

The circuit of the Arduino Controller goes like this:

|-------|PIN 1 --> Provide analogue voltage from 2 to 4.5VDC corresponding to 0 to 180 Elevation degree 
|Yeasu  |PIN 2 --> Short this pin to ground to move zimuth clockwise
|Rotator|PIN 3 <-- Short this pin to ground to move up
|Contro-|PIN 4 <-- Short this pin to gorund to move azimuth counterclockwise
|ller   |PIN 5 <-- Short this pin to ground to move down
|       |PIN 6 --> Provide analogue voltage from 2 to 4.5VDC corresponding to 0 to 450 Azimuth degree
|       |PIN 7 --- Provide 13VDC to 6VDC @ 200mA
|       |PIN 8 --- Commond Ground with arduino

--------------------------------------------------------------------------------------------------------

|-------|A0 <-- Read azimuth degree
|       |A1 <-- Read elevation degree
|Arduino|D4 --> Write to Yeasu Controller (Left)
|Mega   |D5 --> Write to Yeasu Controller (Right)
|       |D6 --> Write to Yeasu Controller (Up)
|-------|D7 --> Write to Yeasu Controller (Down)

You might need a circuit to invert the logic in the code, because physically you need to write low in order
for the controller to rotate, but our code write "HIGH" (5V). 

The circuit can refer to: https://blog.radioartisan.com/yaesu-rotator-computer-serial-interface/

Regards,
JY
