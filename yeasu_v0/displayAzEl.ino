/*Display Azimuth & Elevation*/
void displayAzEl(long az, long el)
{
  if (abs(rotorAzimuth - previousRotorAzimuth) > closeEnough)
  {
    previousRotorAzimuth = rotorAzimuth;
    displayAz(az);
  }
  
  if (abs(rotorElevation - previousRotorElevation) > closeEnough)
  {
    previousRotorElevation = rotorElevation;
    displayEl(el);
  }
}

void displayAz(int az)
{
  lcd.setCursor(0, 0);
  lcd.print("AZ "); lcd.print(az);
  lcd.print(azMovement);
}



void displayEl(int el)
{
  lcd.setCursor(0, 1);
  lcd.print("EL "); lcd.print(el);
  lcd.print(elMovement);
}
