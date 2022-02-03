#include <FastLED.h>

#define NUM_LEDS 63

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

  void setSegment(enum segments segment, int r, int g, int b)
  {
    int pos = (segmentPosOffset[segment]*ledsPerSegment)+firstLedPos;
    for (int i=0; i<ledsPerSegment;i++)
    {
      ledsDataArray[pos].setRGB(r,g,b);
      pos++;
    }
  }

  private:
    struct CRGB *ledsDataArray;
    int ledsPerSegment, firstLedPos;
    int segmentPosOffset[7];
};

void setup()
{
  CRGBArray<NUM_LEDS> leds;
  FastLED.addLeds<NEOPIXEL,6>(leds, NUM_LEDS);
  Digit digit = Digit(9,0,leds);
  digit.setSegment(Digit::top,255,0,0);
  digit.setSegment(Digit::bottom,0,0,255);
  digit.setSegment(Digit::topLeft,0,255,0);
  digit.setSegment(Digit::bottomLeft,0,255,0);
}

void loop()
{

}
