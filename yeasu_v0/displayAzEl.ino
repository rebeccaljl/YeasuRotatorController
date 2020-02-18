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
