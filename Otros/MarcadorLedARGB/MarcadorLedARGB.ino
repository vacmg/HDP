#include <FastLED.h>

#define NUM_LEDS 63
#define DATA_PIN 6

class Digit
{
  public:
    enum segments {top = 1, topLeft = 2, topRight = 3, center = 4, bottom = 5, bottomLeft = 6, bottomRight = 7};

    Digit(int ledsPerSegment, int firstLedPos, struct CRGB *ledsDataArray)
    {
      this->ledsPerSegment = ledsPerSegment;
      this->firstLedPos = firstLedPos;
      this->ledsDataArray = ledsDataArray;
      enum segments segmentOrderList[7] = {bottomRight, bottom, bottomLeft, topLeft, top, topRight, center};
      for(int i = 0; i<7;i++)
      {
          segmentPosOffset[segmentOrderList[i]] = i;
      }
    }

    Digit(int ledsPerSegment, int firstLedPos, struct CRGB *ledsDataArray, enum segments segmentOrderList[])
    {
      this->ledsPerSegment = ledsPerSegment;
      this->firstLedPos = firstLedPos;
      this->ledsDataArray = ledsDataArray;
      for(int i = 0; i<7;i++)
      {
          segmentPosOffset[segmentOrderList[i]] = i;
      }
    }

  void setSegment(enum segments segment, int r, int g, int b, bool show = true)
  {
    int pos = (segmentPosOffset[segment]*ledsPerSegment)+firstLedPos;
    for (int i=0; i<ledsPerSegment;i++)
    {
      ledsDataArray[pos].r = r;
      ledsDataArray[pos].g = g;
      ledsDataArray[pos].b = b;
      pos++;
    }
    if (show)
      FastLED.show();
  }

  void setDigit(char digit, int r, int g, int b, bool show = true)
  {
    switch(digit)
    {
      case 0: case '0':
      setSegment(top,r,g,b,0);
      setSegment(topLeft,r,g,b,0);
      setSegment(topRight,r,g,b,0);
      setSegment(bottom,r,g,b,0);
      setSegment(bottomLeft,r,g,b,0);
      setSegment(bottomRight,r,g,b,0);
      setSegment(center,0,0,0,0);
      break;
      case 1: case '1':
      setSegment(top,0,0,0,0);
      setSegment(topLeft,0,0,0,0);
      setSegment(topRight,r,g,b,0);
      setSegment(bottom,0,0,0,0);
      setSegment(bottomLeft,0,0,0,0);
      setSegment(bottomRight,r,g,b,0);
      setSegment(center,0,0,0,0);
      break;
      case 2: case '2':
      setSegment(top,r,g,b,0);
      setSegment(topLeft,0,0,0,0);
      setSegment(topRight,r,g,b,0);
      setSegment(bottom,r,g,b,0);
      setSegment(bottomLeft,r,g,b,0);
      setSegment(bottomRight,0,0,0,0);
      setSegment(center,r,g,b,0);
      break;
      case 3: case '3':
      setSegment(top,r,g,b,0);
      setSegment(topLeft,0,0,0,0);
      setSegment(topRight,r,g,b,0);
      setSegment(bottom,r,g,b,0);
      setSegment(bottomLeft,0,0,0,0);
      setSegment(bottomRight,r,g,b,0);
      setSegment(center,r,g,b,0);
      break;
      case 4: case '4':
      setSegment(top,0,0,0,0);
      setSegment(topLeft,r,g,b,0);
      setSegment(topRight,r,g,b,0);
      setSegment(bottom,0,0,0,0);
      setSegment(bottomLeft,0,0,0,0);
      setSegment(bottomRight,r,g,b,0);
      setSegment(center,r,g,b,0);
      break;
      case 5: case '5':
      setSegment(top,r,g,b,0);
      setSegment(topLeft,r,g,b,0);
      setSegment(topRight,0,0,0,0);
      setSegment(bottom,r,g,b,0);
      setSegment(bottomLeft,0,0,0,0);
      setSegment(bottomRight,r,g,b,0);
      setSegment(center,r,g,b,0);
      break;
      case 6: case '6':
      setSegment(top,r,g,b,0);
      setSegment(topLeft,r,g,b,0);
      setSegment(topRight,0,0,0,0);
      setSegment(bottom,r,g,b,0);
      setSegment(bottomLeft,r,g,b,0);
      setSegment(bottomRight,r,g,b,0);
      setSegment(center,r,g,b,0);
      break;
      case 7: case '7':
      setSegment(top,r,g,b,0);
      setSegment(topLeft,0,0,0,0);
      setSegment(topRight,r,g,b,0);
      setSegment(bottom,0,0,0,0);
      setSegment(bottomLeft,0,0,0,0);
      setSegment(bottomRight,r,g,b,0);
      setSegment(center,0,0,0,0);
      break;
      case 8: case '8':
      setSegment(top,r,g,b,0);
      setSegment(topLeft,r,g,b,0);
      setSegment(topRight,r,g,b,0);
      setSegment(bottom,r,g,b,0);
      setSegment(bottomLeft,r,g,b,0);
      setSegment(bottomRight,r,g,b,0);
      setSegment(center,r,g,b,0);
      break;
      case 9: case '9':
      setSegment(top,r,g,b,0);
      setSegment(topLeft,r,g,b,0);
      setSegment(topRight,r,g,b,0);
      setSegment(bottom,r,g,b,0);
      setSegment(bottomLeft,0,0,0,0);
      setSegment(bottomRight,r,g,b,0);
      setSegment(center,r,g,b,0);
      break;
      

      case 'a': case 'A':
      setSegment(top,r,g,b,0);
      setSegment(topLeft,r,g,b,0);
      setSegment(topRight,r,g,b,0);
      setSegment(bottom,0,0,0,0);
      setSegment(bottomLeft,r,g,b,0);
      setSegment(bottomRight,r,g,b,0);
      setSegment(center,r,g,b,0);
      break;
      case 'b': case 'B':
      setSegment(top,0,0,0,0);
      setSegment(topLeft,r,g,b,0);
      setSegment(topRight,0,0,0,0);
      setSegment(bottom,r,g,b,0);
      setSegment(bottomLeft,r,g,b,0);
      setSegment(bottomRight,r,g,b,0);
      setSegment(center,r,g,b,0);
      break;
      case 'c': case 'C':
      setSegment(top,r,g,b,0);
      setSegment(topLeft,r,g,b,0);
      setSegment(topRight,0,0,0,0);
      setSegment(bottom,r,g,b,0);
      setSegment(bottomLeft,r,g,b,0);
      setSegment(bottomRight,0,0,0,0);
      setSegment(center,0,0,0,0);
      break;
      case 'd': case 'D':
      setSegment(top,0,0,0,0);
      setSegment(topLeft,0,0,0,0);
      setSegment(topRight,r,g,b,0);
      setSegment(bottom,r,g,b,0);
      setSegment(bottomLeft,r,g,b,0);
      setSegment(bottomRight,r,g,b,0);
      setSegment(center,r,g,b,0);
      break;
      case 'e': case 'E':
      setSegment(top,r,g,b,0);
      setSegment(topLeft,r,g,b,0);
      setSegment(topRight,0,0,0,0);
      setSegment(bottom,r,g,b,0);
      setSegment(bottomLeft,r,g,b,0);
      setSegment(bottomRight,0,0,0,0);
      setSegment(center,r,g,b,0);
      break;
      case 'f': case 'F':
      setSegment(top,r,g,b,0);
      setSegment(topLeft,r,g,b,0);
      setSegment(topRight,0,0,0,0);
      setSegment(bottom,0,0,0,0);
      setSegment(bottomLeft,r,g,b,0);
      setSegment(bottomRight,0,0,0,0);
      setSegment(center,r,g,b,0);
      break;
      case 'g': case 'G':
      setSegment(top,r,g,b,0);
      setSegment(topLeft,r,g,b,0);
      setSegment(topRight,0,0,0,0);
      setSegment(bottom,r,g,b,0);
      setSegment(bottomLeft,r,g,b,0);
      setSegment(bottomRight,r,g,b,0);
      setSegment(center,0,0,0,0);
      break;
      case 'h': case 'H':
      setSegment(top,0,0,0,0);
      setSegment(topLeft,r,g,b,0);
      setSegment(topRight,r,g,b,0);
      setSegment(bottom,0,0,0,0);
      setSegment(bottomLeft,r,g,b,0);
      setSegment(bottomRight,r,g,b,0);
      setSegment(center,r,g,b,0);
      break;
      case 'i': case 'I':
      setSegment(top,0,0,0,0);
      setSegment(topLeft,0,0,0,0);
      setSegment(topRight,r,g,b,0);
      setSegment(bottom,0,0,0,0);
      setSegment(bottomLeft,0,0,0,0);
      setSegment(bottomRight,r,g,b,0);
      setSegment(center,0,0,0,0);
      break;
      case 'j': case 'J':
      setSegment(top,r,g,b,0);
      setSegment(topLeft,0,0,0,0);
      setSegment(topRight,r,g,b,0);
      setSegment(bottom,r,g,b,0);
      setSegment(bottomLeft,0,0,0,0);
      setSegment(bottomRight,r,g,b,0);
      setSegment(center,0,0,0,0);
      break;
      case 'k': case 'K':
      setSegment(top,0,0,0,0);
      setSegment(topLeft,r,g,b,0);
      setSegment(topRight,r,g,b,0);
      setSegment(bottom,0,0,0,0);
      setSegment(bottomLeft,r,g,b,0);
      setSegment(bottomRight,r,g,b,0);
      setSegment(center,r,g,b,0);
      break;
      case 'l': case 'L':
      setSegment(top,0,0,0,0);
      setSegment(topLeft,r,g,b,0);
      setSegment(topRight,0,0,0,0);
      setSegment(bottom,r,g,b,0);
      setSegment(bottomLeft,r,g,b,0);
      setSegment(bottomRight,0,0,0,0);
      setSegment(center,0,0,0,0);
      break;
      case 'm': case 'M':
      setSegment(top,r,g,b,0);
      setSegment(topLeft,r,g,b,0);
      setSegment(topRight,r,g,b,0);
      setSegment(bottom,0,0,0,0);
      setSegment(bottomLeft,r,g,b,0);
      setSegment(bottomRight,r,g,b,0);
      setSegment(center,r,g,b,0);
      break;
      case 'n': case 'N':
      setSegment(top,0,0,0,0);
      setSegment(topLeft,0,0,0,0);
      setSegment(topRight,0,0,0,0);
      setSegment(bottom,0,0,0,0);
      setSegment(bottomLeft,r,g,b,0);
      setSegment(bottomRight,r,g,b,0);
      setSegment(center,r,g,b,0);
      break;
      case 'o': case 'O':
      setSegment(top,0,0,0,0);
      setSegment(topLeft,0,0,0,0);
      setSegment(topRight,0,0,0,0);
      setSegment(bottom,r,g,b,0);
      setSegment(bottomLeft,r,g,b,0);
      setSegment(bottomRight,r,g,b,0);
      setSegment(center,r,g,b,0);
      break;
      case 'p': case 'P':
      setSegment(top,r,g,b,0);
      setSegment(topLeft,r,g,b,0);
      setSegment(topRight,r,g,b,0);
      setSegment(bottom,0,0,0,0);
      setSegment(bottomLeft,r,g,b,0);
      setSegment(bottomRight,0,0,0,0);
      setSegment(center,r,g,b,0);
      break;
      case 'q': case 'Q':
      setSegment(top,r,g,b,0);
      setSegment(topLeft,r,g,b,0);
      setSegment(topRight,r,g,b,0);
      setSegment(bottom,0,0,0,0);
      setSegment(bottomLeft,0,0,0,0);
      setSegment(bottomRight,r,g,b,0);
      setSegment(center,r,g,b,0);
      break;
      case 'r': case 'R':
      setSegment(top,0,0,0,0);
      setSegment(topLeft,0,0,0,0);
      setSegment(topRight,0,0,0,0);
      setSegment(bottom,0,0,0,0);
      setSegment(bottomLeft,r,g,b,0);
      setSegment(bottomRight,0,0,0,0);
      setSegment(center,r,g,b,0);
      break;
      case 's': case 'S':
      setSegment(top,r,g,b,0);
      setSegment(topLeft,r,g,b,0);
      setSegment(topRight,0,0,0,0);
      setSegment(bottom,r,g,b,0);
      setSegment(bottomLeft,0,0,0,0);
      setSegment(bottomRight,r,g,b,0);
      setSegment(center,r,g,b,0);
      break;
      case 't': case 'T':
      setSegment(top,0,0,0,0);
      setSegment(topLeft,r,g,b,0);
      setSegment(topRight,0,0,0,0);
      setSegment(bottom,r,g,b,0);
      setSegment(bottomLeft,r,g,b,0);
      setSegment(bottomRight,0,0,0,0);
      setSegment(center,r,g,b,0);
      break;
      case 'u': case 'U':
      setSegment(top,0,0,0,0);
      setSegment(topLeft,r,g,b,0);
      setSegment(topRight,r,g,b,0);
      setSegment(bottom,r,g,b,0);
      setSegment(bottomLeft,r,g,b,0);
      setSegment(bottomRight,r,g,b,0);
      setSegment(center,0,0,0,0);
      break;
      case 'v': case 'V':
      setSegment(top,0,0,0,0);
      setSegment(topLeft,0,0,0,0);
      setSegment(topRight,0,0,0,0);
      setSegment(bottom,r,g,b,0);
      setSegment(bottomLeft,r,g,b,0);
      setSegment(bottomRight,r,g,b,0);
      setSegment(center,0,0,0,0);
      break;
      case 'w': case 'W':
      setSegment(top,0,0,0,0);
      setSegment(topLeft,0,0,0,0);
      setSegment(topRight,0,0,0,0);
      setSegment(bottom,r,g,b,0);
      setSegment(bottomLeft,r,g,b,0);
      setSegment(bottomRight,r,g,b,0);
      setSegment(center,0,0,0,0);
      break;
      case 'x': case 'X':
      setSegment(top,0,0,0,0);
      setSegment(topLeft,r,g,b,0);
      setSegment(topRight,r,g,b,0);
      setSegment(bottom,0,0,0,0);
      setSegment(bottomLeft,r,g,b,0);
      setSegment(bottomRight,r,g,b,0);
      setSegment(center,r,g,b,0);
      break;
      case 'y': case 'Y':
      setSegment(top,0,0,0,0);
      setSegment(topLeft,r,g,b,0);
      setSegment(topRight,r,g,b,0);
      setSegment(bottom,0,0,0,0);
      setSegment(bottomLeft,0,0,0,0);
      setSegment(bottomRight,r,g,b,0);
      setSegment(center,r,g,b,0);
      break;
      case 'z': case 'Z':
      setSegment(top,r,g,b,0);
      setSegment(topLeft,0,0,0,0);
      setSegment(topRight,r,g,b,0);
      setSegment(bottom,r,g,b,0);
      setSegment(bottomLeft,r,g,b,0);
      setSegment(bottomRight,0,0,0,0);
      setSegment(center,r,g,b,0);
      break;

      default:
      setSegment(top,0,0,0,0);
      setSegment(topLeft,0,0,0,0);
      setSegment(topRight,0,0,0,0);
      setSegment(bottom,0,0,0,0);
      setSegment(bottomLeft,0,0,0,0);
      setSegment(bottomRight,0,0,0,0);
      setSegment(center,0,0,0,0);
    }
    if (show)
      FastLED.show();
  }

  private:
    struct CRGB *ledsDataArray;
    int ledsPerSegment, firstLedPos;
    int segmentPosOffset[7];
};

struct CRGB leds[NUM_LEDS];
Digit digit = Digit(9,0,leds);

void setup()
{
  FastLED.addLeds<NEOPIXEL,DATA_PIN>(leds, NUM_LEDS);
  fill_solid(leds, NUM_LEDS,CRGB::Black);
  
  digit.setSegment(Digit::top,255,0,0);
  digit.setSegment(Digit::bottom,0,0,255);
  digit.setSegment(Digit::topLeft,0,255,0);
  digit.setSegment(Digit::bottomLeft,0,255,0);
  delay(3000);
}

void loop()
{
  for (char i = 0;i<=9;i++)
  {
    digit.setDigit(i,255,30,5);
    delay(1000);
  }
  for (char i = 'a';i<='z';i++)
  {
    digit.setDigit(i,5,255,50);
    delay(1000);
  }
}
