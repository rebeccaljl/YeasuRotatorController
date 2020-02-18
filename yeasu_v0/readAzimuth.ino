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
