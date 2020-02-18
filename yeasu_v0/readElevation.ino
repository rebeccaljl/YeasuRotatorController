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
