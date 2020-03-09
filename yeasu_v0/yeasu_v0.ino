#include<LiquidCrystal.h>
#include <LiquidCrystal_I2C.h>

#define G5500UP 7
#define G5500DN 6
#define G5500LF 4
#define G5500RG 5
#define G5500AZ A0
#define G5500EL A1
#define maxAzimuth 450L
#define maxElevation 180L
#define closeEnough 1

const long azfactor = 2.27;
const long elfactor = 5.68;

long rotorAzimuth = 0L;
long rotorElevation = 0L;
long previousRotorAzimuth = 0L;
long previousRotorElevation = 0L;
long tempAZ = 0L;
long tempEL = 0L;
long newAZ = 0L;
long newEL = 0L;

unsigned long rtcLastDisplayUpdate = 0UL;
unsigned long rtcLastRotorUpdate = 0UL;
unsigned long displayInterval = 500UL;
unsigned long rotorInterval = 100UL;

//SATPC32 - Yeasu GS232
boolean satellite = false;
boolean moveazimuth = false;
boolean moveelevation = false;
String azMovement;
String elMovement;

int count = 0;
LiquidCrystal_I2C lcd(0x27, 16, 2);

void setup()
{
  Serial.begin(9600);

  /*initialise & set pins (up,down,left & righ) as outputs*/
  pinMode(G5500UP, OUTPUT);
  pinMode(G5500DN, OUTPUT);
  pinMode(G5500LF, OUTPUT);
  pinMode(G5500RG, OUTPUT);

  digitalWrite(G5500UP, HIGH);
  digitalWrite(G5500DN, HIGH);
  digitalWrite(G5500LF, HIGH);
  digitalWrite(G5500RG, HIGH);

  /*initialise lcd*/
  lcd.init();
  lcd.backlight();
  lcd.print("ASCS");
  delay(2000);
  lcd.clear();

  /*Getting current azimuth & elevation from G5500*/
  readAzimuth();
  previousRotorAzimuth = rotorAzimuth + 1000 ;
  readElevation();
  previousRotorElevation = rotorElevation + 1000;
}

void loop()

{
  if (Serial.available() > 0)
  {
    decodeSATPC32(Serial.read());
  }
  
  unsigned long rtcCurrent = millis();

  if (rtcCurrent > rtcLastDisplayUpdate)
  {
    if (rtcCurrent - rtcLastRotorUpdate > rotorInterval)
    {
      rtcLastRotorUpdate = rtcCurrent;
      
      readAzimuth();
      if ( (abs(rotorAzimuth - newAZ) > closeEnough) && moveazimuth )
      {
        rotateAzimuth();
      }
      else
      {
        digitalWrite(G5500LF, LOW);
        digitalWrite(G5500RG, LOW);
        moveazimuth = false;
        azMovement = "        ";
      }
      
      readElevation();
      if ( abs(rotorElevation - newEL) > closeEnough && moveelevation )
      {
        rotateElevation();
      }
      else
      {
        digitalWrite(G5500UP, LOW);
        digitalWrite(G5500DN, LOW);
        moveelevation = false;
        elMovement = "        ";
      }
    }

    if (rtcCurrent - rtcLastDisplayUpdate > displayInterval)
    {
      rtcLastDisplayUpdate = rtcCurrent;
      displayAzEl(rotorAzimuth, rotorElevation);
    }
  }
  else
  {
    rtcLastDisplayUpdate = rtcCurrent;
  }
}
