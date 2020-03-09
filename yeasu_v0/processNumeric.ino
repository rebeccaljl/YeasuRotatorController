void processNumeric(char character)
{
  switch (count)
  {
    case 0:
      {
        tempAZ = (character - 48) * 100 ; count++; break;
      }
    case 1:
      {
        tempAZ = tempAZ + (character - 48) * 10 ; count++; break;
      }
    case 2:
      {
        tempAZ = tempAZ + (character - 48); count++;
        if ((tempAZ) > maxAzimuth)
        {
          satellite = false;
          newAZ = 0L; newEL = 0L;
        }
        break;
      }
    case 3:
      {
        tempEL = (character - 48) ; count++; break;
      }
    case 4:
      {
        tempEL = tempEL + (character - 48) * 10; count++; break;
      }
    case 5:
      {
        tempEL = tempEL + (character - 48); count++;
        if ((tempEL) > maxElevation)
        {
          satellite = false;
          newAZ = 0L; newEL = 0L;
        }
        else

        {
          newAZ = tempAZ; newEL = tempEL;
          moveazimuth = true; moveelevation = true;
        }
        break;
      }
    default: { }
  }
}
