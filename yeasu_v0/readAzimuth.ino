/* Reading current Azimuth & elevation of G5500*/
void readAzimuth()
{
  long inputValue = analogRead(G5500AZ);
  rotorAzimuth = ((inputValue) / azfactor);
}
