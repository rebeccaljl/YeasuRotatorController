void readElevation()
{
  long inputValue = analogRead(G5500EL);
  rotorElevation = ((inputValue) / elfactor);
}
