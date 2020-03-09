void rotateElevation()
{
  long rotorMoveEl = newEL - rotorElevation;
  if (rotorMoveEl > 0)
  {
    digitalWrite(G5500DN, LOW);
    digitalWrite(G5500UP, HIGH);
    elMovement = "  U ";
    elMovement = elMovement + String(newEL);
  }
  else
  {
    digitalWrite(G5500UP, LOW);
    digitalWrite(G5500DN, HIGH);
    elMovement = "  D ";
    elMovement = elMovement + String(newEL);
  }
}
