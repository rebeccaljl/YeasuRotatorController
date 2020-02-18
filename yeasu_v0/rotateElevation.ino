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
