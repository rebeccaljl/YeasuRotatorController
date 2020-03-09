/* Rotate Azimuth*/
void rotateAzimuth()
{
  long rotorMoveAz = newAZ - rotorAzimuth;
  if (rotorMoveAz < 0)
  {
    digitalWrite(G5500RG, LOW);
    digitalWrite(G5500LF, HIGH);
    azMovement = "  L ";
    azMovement = azMovement + String(newAZ );
  }
  else
  {
    digitalWrite(G5500LF, LOW);
    digitalWrite(G5500RG, HIGH);
    azMovement = "  R ";
    azMovement = azMovement + String(newAZ);
  }
}
