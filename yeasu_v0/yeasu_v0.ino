#include <LiquidCrystal.h>

//Pins configuration
//Analogue pins for reading Azimuth and El from G5500
#define AZG5500 A1
#define ELG5500 A2

//Digital pins for outputing Azimuth and El to G5500
#define UPG5500 2
#define DWG5500 3
#define LFG5500 11
#define RHG5500 12

//Rotator pre-set constant
#define MAX_AZIMUTH 450
#define MAX_ELEVATION 180

//Define interval for checking rotator coordinate and refresh LCD Screen
#define REFRESH_LCD_INTERVAL 500 //every 500ms refresh the screen
#define READING_ROTOR_INTERVAL 100 //every 100ms read the AZ/EL from rotor

//Variable to store current coordinate of G5500
long rotorAzimuth = 0L;
long rotorElevation = 0L;
long previousRotorAzimuth = 0L; //used delta method to determine if rotor need to rotate
long previousRotorElevation = 0L;

//Variable to store real time clock value
unsigned long rtcLastDisplayUpdate = 0UL;
unsigned long rtcLastRotorUpdate = 0UL;

//Variable to store targeted Az/El coor. from SATPC32
bool satelliteOnSky = false;
bool allowAzimuthMove = false;
bool allowElevationMove = false;
char azimuthMovement;
char elevationMovement;
long targetAzimuth = 0L;
long targetElevation = 0L;
char buff;
int counter;
int azElBuffer[6];

LiquidCrystal lcd(8, 9, 4, 5, 6, 7);

void setup()
{
  //Initialize serial com
  Serial.begin(9600);

  //Initialize lcd screen
  lcd.begin(16, 2);

  //Initialize pin for Output to G5500
  pinMode(UPG5500, OUTPUT); digitalWrite(UPG5500, LOW);
  pinMode(DWG5500, OUTPUT); digitalWrite(DWG5500, LOW);
  pinMode(LFG5500, OUTPUT); digitalWrite(LFG5500, LOW);
  pinMode(RHG5500, OUTPUT); digitalWrite(RHG5500, LOW);

  //Get the current coordinate of our antenna
  //This is because we need to see if motor is required to rotate
  //Also we can set the rotor to a default position at here
  rotorAzimuth = analogRead(AZG5500);
  previousRotorAzimuth = rotorAzimuth;
  rotorElevation = analogRead(ELG5500);
  previousRotorElevation = rotorElevation;
}

void loop()
{
  //Parse command from SATPC32, save targeted Az/El
  if (Serial.available() > 0)
  {
    buff = Serial.read();
    switch (buff)
    {
      case 'w':
      case 'W':
        {
          satelliteOnSky = true;
          counter = 0;
          break;
        }
      // numeric - azimuth and elevation digits
      case '0':  case '1':   case '2':  case '3':  case '4':
      case '5':  case '6':   case '7':  case '8':  case '9':
        {
          if (satelliteOnSky)

          {
            azElBuffer[counter] = buff;
            counter++;
          }

          //Only has 6 digits to read, i.e wxxx yyy
          if (counter == 6)
          {
            //Test if azimuth/elevation exceeded range
            if (targetAzimuth > MAX_AZIMUTH || targetElevation > MAX_ELEVATION) {
              satelliteOnSky = false;
              allowAzimuthMove = false;
              targetAzimuth = 0;
              targetElevation = 0;
            } else
            {
              //Extract AZ/EL value from buffer array
              targetAzimuth = azElBuffer[0] * 100 + azElBuffer[1] * 10 + azElBuffer[2] * 1;
              targetElevation = azElBuffer[3] * 100 + azElBuffer[4] * 10 + azElBuffer[5] * 1;
              allowAzimuthMove = true;
              allowElevationMove = true;
            }
            counter = 0;
          }
        }
      default:
        {
          //ignore everything, e.g. C2 or "\r" or " "
        }
    }
  }

  //Every N ms, we check for rotator direction, to see if rotation is needed
  unsigned long rtcCurrent = millis();
  if (rtcCurrent < rtcLastDisplayUpdate)
  {
    //if enter this, meaning the real time clock variable is overflown
    //i.e count up to 10 and count back from 1, so 1 < 9 (assume 9 is last count), then discard this loop
    //reset the rtcCurrent
    rtcLastDisplayUpdate = rtcCurrent;
  } else
  {
    //Update rotor reading every N interval
    if (rtcCurrent - rtcLastRotorUpdate >= READING_ROTOR_INTERVAL)
    {
      //reset last update timer
      rtcLastRotorUpdate = rtcCurrent;

      //Read azimuth from AZG5500
      rotorAzimuth = analogRead(AZG5500);

      //Check if rotation is required, i.e if very close to target location then no need to rotate
      //Check for azimuth
      if ( abs(targetAzimuth - rotorAzimuth) > 5 && allowAzimuthMove)
      {
        //Check if turn left or right
        if (targetAzimuth - rotorAzimuth < 0) { // turn left or counterclockwise
          digitalWrite(RHG5500, LOW);
          digitalWrite(LFG5500, HIGH);
          //azimuthMovement = "L";
          azimuthMovement = 'L';
        } else // turn right
        {
          digitalWrite(RHG5500, HIGH);
          digitalWrite(LFG5500, LOW);
          azimuthMovement = 'R';
        }
      } else
      {
        allowAzimuthMove = false;
        digitalWrite(LFG5500, LOW);
        digitalWrite(RHG5500, LOW);
      }

      //Read Elevation from G5500
      rotorElevation = analogRead(ELG5500);
      //test for elevation
      if ( abs(targetElevation - rotorElevation) > 5 && allowElevationMove)
      {
        //Check if turn up or down
        if (targetElevation - rotorElevation < 0) { // turn down
          digitalWrite(UPG5500, LOW);
          digitalWrite(DWG5500, HIGH);
          elevationMovement = 'U';
        } else // turn up
        {
          digitalWrite(UPG5500, HIGH);
          digitalWrite(DWG5500, LOW);
          elevationMovement = 'D';
        }
      } else
      {
        allowElevationMove = false;
        digitalWrite(UPG5500, LOW);
        digitalWrite(DWG5500, LOW);
      }
      //end of update rotor movement
    }

    //Update LCD reading
    if (rtcCurrent - rtcLastDisplayUpdate >= REFRESH_LCD_INTERVAL)
    {
      rtcLastDisplayUpdate = rtcCurrent;
      displayAzEl(rotorAzimuth, rotorElevation);
    }
  }
}

void displayAzEl(long rotorAzimuth, long rotorElevation) {
  // display azimuth - filter A/D noise
  // if reading from rotator is too close, then no need to update
  if (abs(rotorAzimuth - previousRotorAzimuth) > 5)
  {
    previousRotorAzimuth = rotorAzimuth;
    lcd.setCursor(0, 0);
    lcd.print("A" + String(rotorAzimuth) + " " + azimuthMovement + " T-A" + String(targetAzimuth));
  }

  // display elevation - filter A/D noise
  if (abs(rotorElevation - previousRotorElevation) > 5)
  {
    previousRotorElevation = rotorElevation;
    lcd.setCursor(0, 1);
    lcd.print("E" + String(rotorElevation) + " " + elevationMovement + " T-E" + String(targetElevation));
  }
}

/*TODO
  1. Constrain the voltage level reading from G5500 to 2 - 4.5 and map to corresponding degree.
   Arduino has 12-bit ADC, 0 - 5 correspond to 0-1023, so what about 2 - 4.5?
  2. The alignment of the LCD words is sucks, need to be fixed.
  TODO*/
