
#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

#include <LiquidCrystal.h>
#include <SoftwareSerial.h> // use software uart library

const long _azAdZeroOffset   =   325;   // adjust to zero out lcd az reading when control box az = 0
const long _elAdZeroOffset   =   0;   // adjust to zero out lcd el reading when control box el = 0

const long _azScaleFactor =  232;  //  adjust as needed
const long _elScaleFactor =  568;  //  adjust as needed 

// lcd display control
const byte _backLightOn = 0x11;   // lcd back light on
const byte _cursorOff = 0x16;     // lcd cursor off
const byte _clearScreen = 0x0C;   // lcd clear screen
const byte _line0 = 0x80;         // lcd line 0 - top line
const byte _line1 = 0x94;         // lcd line 1 - bottom line

// pins
const byte _azimuthInputPin = A0;   // azimuth analog signal from G5500
const byte _elevationInputPin = A1; // elevation analog signal from G5500
const byte _G5500UpPin = 8;        // elevation rotor up control line
const byte _G5500DownPin = 9;      // elevation rotor down control line
const byte _G5500LeftPin = 10;      // azimuth rotor left control line
const byte _G5500RightPin = 11;     // azimuth rotor right control line

const byte _LcdTxPin = 7;          // software uart lcd tx pin
const byte _LcdRxPin = 6;          // software uart lcd rx pin - pin not used

// take care if you lower this value -  wear or dirt on the pots in your rotors
// or A/D converter jitter may cause hunting if the value is too low. 
long _closeEnough = 100;   // tolerance for az-el match in rotor move in degrees * 100

// ------------------------------------------------------------
// ------ values from here down should not need adjusting -----
// ------------------------------------------------------------

// rotor
const long _maxRotorAzimuth = 45000L;  // maximum rotor azimuth in degrees * 100
const long _maxRotorElevation = 18000L; // maximum rotor elevation in degrees * 100

long _rotorAzimuth = 0L;       // current rotor azimuth in degrees * 100
long _rotorElevation = 0L;     // current rotor azimuth in degrees * 100
long _azimuthTemp = 0L;        // used for gs232 azimuth decoding
long _elevationTemp = 0L;      // used for gs232 elevation decoding  
long _newAzimuth = 0L;         // new azimuth for rotor move
long _newElevation = 0L;       // new elevation for rotor move
long _previousRotorAzimuth = 0L;       // previous rotor azimuth in degrees * 100
long _previousRotorElevation = 0L;     // previous rotor azimuth in degrees * 100

unsigned long _rtcLastDisplayUpdate = 0UL;      // rtc at start of last loop
unsigned long _rtcLastRotorUpdate = 0UL;        // rtc at start of last loop
unsigned long _displayUpdateInterval = 500UL;   // display update interval in mS
unsigned long _rotorMoveUpdateInterval = 100UL; // rotor move check interval in mS

boolean _gs232WActice = false;  // gs232 W command in process
int _gs232AzElIndex = 0;        // position in gs232 Az El sequence
long _gs232Azimuth = 0;          // gs232 Azimuth value
long _gs232Elevation = 0;        // gs232 Elevation value
boolean _azimuthMove = false;     // azimuth move needed
boolean _elevationMove = false;   // elevation move needed

String azRotorMovement;   // string for az rotor move display
String elRotorMovement;   // string for el rotor move display

// create instance of NewSoftSerial 
//SoftwareSerial lcdSerial =  SoftwareSerial(_LcdRxPin, _LcdTxPin);
LiquidCrystal lcd(8, 9, 4, 5, 6, 7);
void setup()
{
   // initialize rotor control pins as outputs
   pinMode(_G5500UpPin, OUTPUT);
   pinMode(_G5500DownPin, OUTPUT);
   pinMode(_G5500LeftPin, OUTPUT);
   pinMode(_G5500RightPin, OUTPUT);
   
   // set all the rotor control outputs low
   digitalWrite(_G5500UpPin, LOW);
   digitalWrite(_G5500DownPin, LOW);
   digitalWrite(_G5500LeftPin, LOW);
   digitalWrite(_G5500RightPin, LOW);
    
   Serial.begin(9600);
   lcd.begin(16, 2);
   lcd.setCursor(0, 0);  
   lcd.print("          ");
    
    // initialize software uart used for lcd display
    //pinMode(_LcdTxPin, OUTPUT);
    //lcdSerial.begin(9600);
    // initialize lcd display
   //lcdSerial.write(_backLightOn);   // backlight on
   // lcdSerial.write(_cursorOff);     // cursor off
    //lcdSerial.write(_clearScreen);   // clear screen
    //delay(100);                         // wait for clear screen  
    //lcdSerial.println("   W9KE V1.8    ");
    //delay(2000);
    //lcdSerial.write(_clearScreen);   // clear screen    

    
    
    readAzimuth(); // get current azimuth from G-5500
    _previousRotorAzimuth = _rotorAzimuth + 1000;
    readElevation(); // get current elevation from G-5500
   _previousRotorElevation = _rotorElevation + 1000;    
}



// main program loop

void loop() 
{      
    // check for serial data
    if (Serial.available() > 0)
    {
      decodeGS232(Serial.read()); 
      lcd.print("    test            ");
    }
    
    unsigned long rtcCurrent = millis(); // get current rtc value
    
    // check for rtc overflow - skip this cycle if overflow
    if (rtcCurrent > _rtcLastDisplayUpdate) // overflow if not true    _rotorMoveUpdateInterval
    {
      // update rotor movement if necessary
      if (rtcCurrent - _rtcLastRotorUpdate > _rotorMoveUpdateInterval)
      {
         _rtcLastRotorUpdate = rtcCurrent; // reset rotor move timer base
         
         // AZIMUTH       
         readAzimuth(); // get current azimuth from G-5500
         // see if azimuth move is required
         if ( (abs(_rotorAzimuth - _newAzimuth) > _closeEnough) && _azimuthMove ) 
         {
            updateAzimuthMove();
         }
        else  // no move required - turn off azimuth rotor
         {
           digitalWrite(_G5500LeftPin, LOW);
           digitalWrite(_G5500RightPin, LOW);
           _azimuthMove = false;
           azRotorMovement = "        ";
         }
         
         // ELEVATION       
         readElevation(); // get current elevation from G-5500
         // see if aelevation move is required
         if ( abs(_rotorElevation - _newElevation) > _closeEnough && _elevationMove ) // move required
         {
            updateElevationMove();
         }
        else  // no move required - turn off elevation rotor
         {
            digitalWrite(_G5500UpPin, LOW);
            digitalWrite(_G5500DownPin, LOW);
            _elevationMove = false;
            elRotorMovement = "        ";
         }            
      } // end of update rotor move
      
      
      // update display if necessary
      if (rtcCurrent - _rtcLastDisplayUpdate > _displayUpdateInterval) 
      {
        // update rtcLast 
        _rtcLastDisplayUpdate = rtcCurrent;  // reset display update counter base
        displayAzEl(_rotorAzimuth, _rotorElevation);
      } 
    }
    else // rtc overflow - just in case
    {
      // update rtcLast 
      _rtcLastDisplayUpdate = rtcCurrent;
    } 
}


//
// update elevation rotor move
//
void updateElevationMove()
{          
   // calculate rotor move 
   long rotorMoveEl = _newElevation - _rotorElevation;
   
   if (rotorMoveEl > 0)
   {
      elRotorMovement = "  U ";
      elRotorMovement = elRotorMovement + String(_newElevation / 100);
      digitalWrite(_G5500DownPin, LOW);
      digitalWrite(_G5500UpPin, HIGH);      
   }
   else
   {           
     if (rotorMoveEl < 0)
     {
       elRotorMovement = "  D ";
       elRotorMovement = elRotorMovement + String(_newElevation / 100);
       digitalWrite(_G5500UpPin, LOW);
       digitalWrite(_G5500DownPin, HIGH);       
     } 
   } 
}


// update azimuth rotor move

void updateAzimuthMove()
{          
     // calculate rotor move 
     long rotorMoveAz = _newAzimuth - _rotorAzimuth;
     // adjust move if necessary
     if (rotorMoveAz > 18000) // adjust move if > 180 degrees
     {
        rotorMoveAz = rotorMoveAz - 180;
     }
     else
     {           
       if (rotorMoveAz < -18000) // adjust move if < -180 degrees
       {
         rotorMoveAz = rotorMoveAz + 18000;
       }
     }
     
     if (rotorMoveAz > 0)
     {
        azRotorMovement = "  R ";
        azRotorMovement = azRotorMovement + String(_newAzimuth / 100);
        digitalWrite(_G5500LeftPin, LOW);
        digitalWrite(_G5500RightPin, HIGH);        
     }
     else
     {           
       if (rotorMoveAz < 0)
       {
         azRotorMovement = "  L ";
         azRotorMovement = azRotorMovement + String(_newAzimuth / 100);
         digitalWrite(_G5500RightPin, LOW); 
         digitalWrite(_G5500LeftPin, HIGH);         
       } 
     }            
}



// read azimuth from G5500
void readElevation()
{
   long sensorValue = analogRead(_elevationInputPin);
   _rotorElevation = ((sensorValue * 10000) / _elScaleFactor) - _elAdZeroOffset;
}



// read azimuth from G5500
void readAzimuth()
{
  long sensorValue = analogRead(_azimuthInputPin);
  _rotorAzimuth = ((sensorValue * 10000) / _azScaleFactor) - _azAdZeroOffset;
}



// decode gs232 commands
void decodeGS232(char character)
{
    switch (character)
    {
       case 'w':  // gs232 W command
       case 'W':
       {
          {
            _gs232WActice = true;
            _gs232AzElIndex = 0;
          }
          break;
       }
       
       // numeric - azimuth and elevation digits
       case '0':  case '1':   case '2':  case '3':  case '4': 
       case '5':  case '6':   case '7':  case '8':  case '9':
       {
          if ( _gs232WActice)
          {
            processAzElNumeric(character);          
          }
       }   
       
       default:
       {
          // ignore everything else
       }
     }
}



// process az el numeric characters from gs232 W command

void processAzElNumeric(char character)
{
      switch(_gs232AzElIndex)
      {
         case 0: // first azimuth character
        {
            _azimuthTemp =(character - 48) * 100;
            _gs232AzElIndex++;
            break;
        } 
        
        case 1:
        {
            _azimuthTemp = _azimuthTemp + (character - 48) * 10;
            _gs232AzElIndex++;
            break;
        } 
        
        case 2: // final azimuth character
        {
            _azimuthTemp = _azimuthTemp + (character - 48);
            _gs232AzElIndex++;
            
            // check for valid azimuth 
            if ((_azimuthTemp * 100) > _maxRotorAzimuth)
            {
              _gs232WActice = false;
              _newAzimuth = 0L;
              _newElevation = 0L;
            }           
            break;
        }  
        
        case 3: // first elevation character
        {
            _elevationTemp =(character - 48) * 100;
            _gs232AzElIndex++;
            break;
        } 
        
        case 4:
        {
            _elevationTemp = _elevationTemp + (character - 48) * 10;
            _gs232AzElIndex++;
            break;
        } 
        
        case 5: // last elevation character
        {
            _elevationTemp = _elevationTemp + (character - 48);
            _gs232AzElIndex++;
            
            // check for valid elevation 
            if ((_elevationTemp * 100) > _maxRotorElevation)
            {
              _gs232WActice = false;
              _newAzimuth = 0L;
              _newElevation = 0L;
            }
            else // both azimuth and elevation are ok
            {
              // set up for rotor move
              _newAzimuth = _azimuthTemp * 100;
              _newElevation = _elevationTemp * 100;
              _azimuthMove = true;
              _elevationMove = true;
            }            
            break;
        }             
          
        default:
        {
           // should never get here
        }         
      } 
}



// display az el on display
void displayAzEl(long az, long el)
{ 
    // display azimuth - filter A/D noise
    if (abs(_rotorAzimuth - _previousRotorAzimuth) > 50)
    {
      _previousRotorAzimuth = _rotorAzimuth;
      displayAz(az);
    }
    
    
    // display elevation - filter A/D noise
    if (abs(_rotorElevation - _previousRotorElevation) > 50)
    {
      _previousRotorElevation = _rotorElevation;
      displayEl(el);
    }    
   
}


void displayEl(long el)
{
  // clear elevation line  lcdSerial
  //lcd.write(_line1);
  lcd.setCursor(0, 1);
  lcd.print("                ");  
  //  adjust value for display
  double elFloat = el;
  elFloat = elFloat / 100.0;
  //lcd.write(_line1); 
  lcd.setCursor(0, 1);
  lcd.print("EL ");  
  
  if (elFloat < 10.0)
  {
    //lcd.print(" ");
    lcd.write("                ");
  }  
  if (elFloat < 100.0)
  {
    //lcd.print(" ");
    lcd.write("                ");
    
  }   
  //lcd.print(elFloat, 1);
  //lcd.print(elRotorMovement);
  lcd.write(elFloat);
  lcd.write(elRotorMovement);
   
}


void displayAz(long az)
{
  //lcd.write(_line0);
  lcd.clear();
  lcd.setCursor(0, 0);
  //lcd.print("                ");
  lcd.write("                ");
  double azFloat = az;
  azFloat = azFloat / 100.0;
  //lcd.write(_line0);
  lcd.setCursor(0, 0);     
  //lcd.print("AZ ");
  lcd.write(" AZ");
  if (azFloat < 10.0)
  {
    //lcd.print(" ");
    lcd.write("                ");
  }
  if (azFloat < 100.0)
  {
    //lcd.print(" ");
    lcd.write("                ");
  }  
  //lcd.print(azFloat, 1);
  lcd.write(azFloat);
  //lcd.print(azRotorMovement); 
  lcd.write(azRotorMovement);
}
