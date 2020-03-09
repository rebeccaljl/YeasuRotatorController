/*Decode*/
void decodeSATPC32(char character)
{
  switch (character)
  {
    case 'w':
    case 'W':
      {
        {
          satellite = true; count = 0;
        }
        break;
      }
    case '0':  case '1':   case '2':  case '3':  case '4':
    case '5':  case '6':   case '7':  case '8':  case '9':
      {
        if ( satellite)
        {
          processNumeric(character);
        }
      }
  }
}
