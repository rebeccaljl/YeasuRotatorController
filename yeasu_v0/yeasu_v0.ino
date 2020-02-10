#include <LiquidCrystal.h>

//Pins configuration
//Analogue pins for reading Azimuth and El from G5500
#define AZG5500 A1
#define ELG5500 A2

//Digital pins for outputing Azimuth and El to G5500
#define UPG5500 10
#define DWG5500 11
#define LFG5500 12
#define RHG5500 13

//Rotator pre-set constant
#define MAX_AZIMUTH 450
#define MAX_ELEVATION 180
#define CLOSEENOUGH 2

//Define interval for checking rotator coordinate and refresh LCD Screen
#define REFRESH_LCD_INTERVAL 500 //every 500ms refresh the screen
#define READING_ROTOR_INTERVAL 1000 //every 100ms read the AZ/EL from rotor

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
char azimuthMovement = 'N';
char elevationMovement = 'N';
long targetAzimuth = 0L;
long targetElevation = 0L;
char buff;
int counter;
int azElBuffer[6];

LiquidCrystal lcd(8, 9, 4, 5, 6, 7);

void setup()
{
  //Initialize serial com for debugging
  Serial.begin(9600);

  //Initialize serial1 com for SatPC32
  Serial1.begin(9600);

  //Initialize lcd screen
  lcd.begin(16, 2);

  //Initialize pin for Input from G5500
  pinMode(AZG5500, INPUT);
  pinMode(ELG5500, INPUT);

  //Initialize pin for Output to G5500
  pinMode(UPG5500, OUTPUT); digitalWrite(UPG5500, HIGH); //HIGH to rotate, low to stay still
  pinMode(DWG5500, OUTPUT); digitalWrite(DWG5500, HIGH);
  pinMode(LFG5500, OUTPUT); digitalWrite(LFG5500, HIGH);
  pinMode(RHG5500, OUTPUT); digitalWrite(RHG5500, HIGH);

  //Get the current coordinate of our antenna
  //This is because we need to see if motor is required to rotate
  //Also we can set the rotor to a default position at here
  rotorAzimuth = readAzimuth();
  previousRotorAzimuth = rotorAzimuth;
  rotorElevation = readElevation();
  previousRotorElevation = rotorElevation;
}

void loop()
{
  if (Serial1.available() > 0) parseSATPC32Command(Serial1.read());

  //Every N ms, we check for rotator direction, to see if rotation is needed
  unsigned long rtcCurrent = millis();
  if (rtcCurrent < rtcLastDisplayUpdate)
  {
    //if enter this, meaning the real time clock variable is overflown
    rtcLastDisplayUpdate = rtcCurrent;
    Serial.println("RTC Overflow");
  } else {
    //Update rotor reading every N interval
    if (rtcCurrent - rtcLastRotorUpdate >= READING_ROTOR_INTERVAL)
    {
      //reset last update timer
      rtcLastRotorUpdate = rtcCurrent;

      rotateAzimuth();

      rotateElevation();
    }

    //Update LCD reading
    if (rtcCurrent - rtcLastDisplayUpdate >= REFRESH_LCD_INTERVAL)
    {
      rtcLastDisplayUpdate = rtcCurrent;
      displayAzEl(rotorAzimuth, rotorElevation);
    }
  }
}

int readAzimuth() {
  //Read voltage store into buffer
  int azimuthVoltage;
  azimuthVoltage = analogRead(AZG5500);
  //voltage provided varies from 2 to 4.5 <==> 410 to 922
  azimuthVoltage = constrain(azimuthVoltage, 410, 922);
  //map voltage 2 to 4.5v to angle 0 to 450 degree
  azimuthVoltage = map(azimuthVoltage, 410, 922, 0, 450);
  return azimuthVoltage;
}

int readElevation() {
  //Read voltage store into buffer
  int elevationVoltage;
  elevationVoltage = analogRead(ELG5500);
  //voltage provided varies from 2 to 4.5 <==> 410 to 922
  elevationVoltage = constrain(elevationVoltage, 410, 922);
  //map voltage 2 to 4.5v to angle 0 to 450 degree
  elevationVoltage = map(elevationVoltage, 410, 922, 0, 450);
  return elevationVoltage;
}

void parseSATPC32Command(char buff) {
  //Parse command from SATPC32, save targeted Az/El
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
        if (satelliteOnSky) azElBuffer[counter++] = buff;;
        //Process array when buffer is full
        //i.e wxxxyyy
        //     012345
        if (counter == 6)
        {
          //Extract AZ/EL value from buffer array
          targetAzimuth = (azElBuffer[0] - 48) * 100 + (azElBuffer[1] - 48) * 10 + (azElBuffer[2] - 48) * 1;
          targetElevation = (azElBuffer[3] - 48) * 100 + (azElBuffer[4] - 48) * 10 + (azElBuffer[5] - 48) * 1;
          //Discard if azimuth/elevation exceeded range
          if (targetAzimuth > MAX_AZIMUTH || targetElevation > MAX_ELEVATION) {
            satelliteOnSky = false;
            allowAzimuthMove = false;
            targetAzimuth = 0;
            targetElevation = 0;
          } else
          {
            allowAzimuthMove = true;
            allowElevationMove = true;
          }
          counter = 0;
        }
        break;
      }
    default:
      {
        //ignore everything, e.g. C2 or "\r" or " "
        satelliteOnSky = false;
      }
  }
}

void rotateAzimuth() {
  //Read azimuth from AZG5500
  rotorAzimuth = readAzimuth();

  //Check if rotation is required, i.e if very close to target location then no need to rotate
  if ( abs(targetAzimuth - rotorAzimuth) > CLOSEENOUGH && allowAzimuthMove)
  {
    //Check if turn left or right
    if (targetAzimuth - rotorAzimuth < 0) { // turn left or counterclockwise
      digitalWrite(RHG5500, HIGH);
      digitalWrite(LFG5500, LOW);
      azimuthMovement = 'L';
    } else // turn right
    {
      digitalWrite(RHG5500, LOW);
      digitalWrite(LFG5500, HIGH);
      azimuthMovement = 'R';
    }
  } else
  {
    allowAzimuthMove = false;
    elevationMovement = "N";
    digitalWrite(LFG5500, HIGH);
    digitalWrite(RHG5500, HIGH);
  }
}

void rotateElevation() {
  //Read Elevation from G5500
  rotorElevation = readElevation();
  //test for elevation
  if ( abs(targetElevation - rotorElevation) > CLOSEENOUGH && allowElevationMove)
  {
    //Check if turn up or down
    if (targetElevation - rotorElevation < 0) { // turn down
      digitalWrite(UPG5500, HIGH);
      digitalWrite(DWG5500, LOW);
      elevationMovement = 'D';
    } else // turn up
    {
      digitalWrite(UPG5500, LOW);
      digitalWrite(DWG5500, HIGH);
      elevationMovement = 'U';
    }
  } else
  {
    allowElevationMove = false;
    elevationMovement = "N";
    digitalWrite(UPG5500, HIGH);
    digitalWrite(DWG5500, HIGH);
  }
  //end of update rotor movement
}

void displayAzEl(long rotorAzimuth, long rotorElevation) {
  // display azimuth - filter A/D noise
  // if reading from rotator is too close, then no need to update
  if (abs(rotorAzimuth - previousRotorAzimuth) > CLOSEENOUGH)
  {
    previousRotorAzimuth = rotorAzimuth;
    //lcd.setCursor(0, 0);
    //lcd.print("A" + String(rotorAzimuth) + " " + azimuthMovement + " T-A" + String(targetAzimuth));
    Serial.print("Rotor Azimuth: ");
    Serial.print(rotorAzimuth);
    Serial.print(" Target Azimuth: ");
    Serial.print(targetAzimuth);
    Serial.print(" Azimuth Movement: ");
    Serial.println(azimuthMovement);
  }

  // display elevation - filter A/D noise
  if (abs(rotorElevation - previousRotorElevation) > CLOSEENOUGH)
  {
    previousRotorElevation = rotorElevation;
    //lcd.setCursor(0, 1);
    //lcd.print("E" + String(rotorElevation) + " " + elevationMovement + " T-E" + String(targetElevation));
    Serial.print("Rotor Elevation: ");
    Serial.print(rotorElevation);
    Serial.print(" Target Elevation: ");
    Serial.print(targetElevation);
    Serial.print(" Elevation Movement: ");
    Serial.println(elevationMovement);
    Serial.println(" ");
  }
}
