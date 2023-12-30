/**
 * Codigo fuente de un marcador de pelota vasca hecho a partir de una tira led aRGB y que puede utilizar uno de los siguientes modos de funcionamiento:
 * Por cable y botones; Por un mando infrarrojos.
**/

#define MODE_WIRED 0
#define MODE_IR 1
#define MODE_RF 2
#define CYCLIC_BEHAVIOUR 0
#define LINEAL_BEHAVIOUR 1

//////////////////////////////////////////     SETTINGS     ////////////////////////////////////////////////
#define COUNTER_BEHAVIOUR CYCLIC_BEHAVIOUR // El modo del contador, ciclico: cuando llega al maximo, vuelve al minimo y viceversa; lineal: al llegar al maximo/minimo se queda en el
#define MODE_OF_INTERACTION MODE_RF // Aqui se selecciona el modo de interaccion
#define ALWAYS_USE_2_DIGITS true // a true para tener numeros de 0-9 de la forma 0X, y false para tenerlos de la forma X
#define WHITE false // si quieres probar la conexion con leds blancos, pon esto a true
#define DELAY 500 // el tiempo que tarda desde que pulsas un boton hasta que el arduino reconoce la siguente pulsacion
//////////////////////////////////////////     SETTINGS     ////////////////////////////////////////////////

//////////////////////////////////////////     I/O     /////////////////////////////////////////////////////
#define LEDS_DATA_PIN 6
#define NUM_LEDS 252
#define MAX_NUM 99
#define MIN_NUM 0

#if MODE_OF_INTERACTION == MODE_IR
  #define IR_RECEIVE_PIN 2 // To be compatible with interrupt example, pin 2 is chosen here.
  #define DECODE_NEC // Includes Apple and Onkyo

  #define RED_UP_IR_CMD 0x5E
  #define RED_DOWN_IR_CMD 0x4A
  #define BLUE_UP_IR_CMD 0xC
  #define BLUE_DOWN_IR_CMD 0x42
  #define RESET_IR_CMD 0x1C

  #include <IRremote.hpp>
#elif MODE_OF_INTERACTION == MODE_WIRED
  const byte redUp = 8;
  const byte redDown = 9;
  const byte blueUp = 10;
  const byte blueDown = 11;
  const byte reset = 12;
#elif MODE_OF_INTERACTION == MODE_RF
  #define RF_RECEIVE_INTERRUPT 0 // This goes to pin 2
  #define GDO0_PIN 9
  #define GDO2_PIN 2

  #define CMDs XX

  #include <ELECHOUSE_CC1101_SRC_DRV.h>
  #include <RCSwitch.h>

  RCSwitch mySwitch = RCSwitch();
#endif
//////////////////////////////////////////     I/O     /////////////////////////////////////////////////////

typedef enum ActionEnum {NO_ACTION = 0, RED_UP_ACTION, RED_DOWN_ACTION, BLUE_UP_ACTION, BLUE_DOWN_ACTION, RESET_ACTION} Action;

#include <FastLED.h>

class Digit
{
  public:
    enum segments {top = 0, topLeft, topRight, center, bottom, bottomLeft, bottomRight};

    Digit(int ledsPerSegment, int firstLedPos, struct CRGB *ledsDataArray)
    {
      this->ledsPerSegment = ledsPerSegment;
      this->firstLedPos = firstLedPos;
      this->ledsDataArray = ledsDataArray;
      enum segments segmentOrderList[7] = {topLeft, top, topRight, bottomRight, bottom, bottomLeft, center};
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

Action action = NO_ACTION;
struct CRGB leds[NUM_LEDS];
Digit lBlue(9,0,leds);
Digit rBlue(9,63,leds);
Digit lRed(9,126,leds);
Digit rRed(9,189,leds);

byte red = MIN_NUM;
byte blue = MIN_NUM;

void updateSign()
{
  byte num = (blue/10)%10;
  #if !ALWAYS_USE_2_DIGITS
    if(num == 0)
    {
      num = ' ';
    }
  #endif
  lBlue.setDigit(num,0,0,255,0);
  rBlue.setDigit(blue%10,0,0,255,0);

  num = (red/10)%10;
  #if !ALWAYS_USE_2_DIGITS
    if(num == 0)
    {
      num = ' ';
    }
  #endif
  lRed.setDigit(num,255,0,0,0);
  rRed.setDigit(red%10,255,0,0,0);

  FastLED.show();
}

void setup()
{
  FastLED.addLeds<NEOPIXEL,LEDS_DATA_PIN>(leds, NUM_LEDS);
  fill_solid(leds, NUM_LEDS,CRGB::White);
  FastLED.show();
  #if WHITE
    while (true);
  #endif

  Serial.begin(115200);
  delay(1000);
  Serial.println(F("STARTING"));
  
  lBlue.setDigit('H',0,255,30);
  rBlue.setDigit('D',0,255,30);
  lRed.setDigit('P',0,255,30);
  rRed.setDigit(' ',0,255,30);
  FastLED.show();

  #if MODE_OF_INTERACTION == MODE_WIRED
    pinMode(redUp,INPUT_PULLUP);
    pinMode(redDown,INPUT_PULLUP);
    pinMode(blueUp,INPUT_PULLUP);
    pinMode(blueDown,INPUT_PULLUP);
    pinMode(reset,INPUT_PULLUP);
  #elif MODE_OF_INTERACTION == MODE_IR
    IrReceiver.begin(IR_RECEIVE_PIN, ENABLE_LED_FEEDBACK);
  #elif MODE_OF_INTERACTION == MODE_RF
    if (ELECHOUSE_cc1101.getCC1101())
    {
      Serial.println("Connection OK");
    }
    else
    {
      Serial.println("Connection Error");
      lBlue.setDigit('E',255,0,30);
      rBlue.setDigit('R',255,0,30);
      lRed.setDigit('R',255,0,30);
      rRed.setDigit('C',255,255,255);
      FastLED.show();
    }

    ELECHOUSE_cc1101.setGDO(GDO0_PIN, GDO2_PIN);
    //CC1101 Settings:                (Settings with "//" are optional!)
    ELECHOUSE_cc1101.Init();            // must be set to initialize the cc1101!
  //ELECHOUSE_cc1101.setRxBW(812.50);  // Set the Receive Bandwidth in kHz. Value from 58.03 to 812.50. Default is 812.50 kHz.
  //ELECHOUSE_cc1101.setPA(10);       // set TxPower. The following settings are possible depending on the frequency band.  (-30  -20  -15  -10  -6    0    5    7    10   11   12)   Default is max!
    ELECHOUSE_cc1101.setMHZ(433.92); // Here you can set your basic frequency. The lib calculates the frequency automatically (default = 433.92).The cc1101 can: 300-348 MHZ, 387-464MHZ and 779-928MHZ. Read More info from datasheet.

    mySwitch.enableReceive(RF_RECEIVE_INTERRUPT);  // Receiver on
    ELECHOUSE_cc1101.SetRx();  // set Receive on
  #endif

  delay(5000);
  updateSign();
  Serial.println(F("READY"));
}

void loop()
{
  #if MODE_OF_INTERACTION == MODE_IR
    if (IrReceiver.decode())
    {
      if(IrReceiver.decodedIRData.command == RED_UP_IR_CMD)
      {
        action = RED_UP_ACTION;
      }
      else if(IrReceiver.decodedIRData.command == RED_DOWN_IR_CMD)
      {
        action = RED_DOWN_ACTION;
      }
      else if(IrReceiver.decodedIRData.command == BLUE_UP_IR_CMD)
      {
        action = BLUE_UP_ACTION;
      }
      else if(IrReceiver.decodedIRData.command == BLUE_DOWN_IR_CMD)
      {
        action = BLUE_DOWN_ACTION;
      }
      else if(IrReceiver.decodedIRData.command == RESET_IR_CMD)
      {
        action = RESET_ACTION;
      }
      else
      {
        IrReceiver.resume(); // Discard other values
      }
    }
  #elif MODE_OF_INTERACTION == MODE_WIRED
    if(!digitalRead(redUp))
    {
      action = RED_UP_ACTION;
    }
    else if(!digitalRead(redDown))
    {
      action = RED_DOWN_ACTION;
    }
    else if(!digitalRead(blueUp))
    {
      action = BLUE_UP_ACTION;
    }
    else if(!digitalRead(blueDown))
    {
      action = BLUE_DOWN_ACTION;
    }
    else if(!digitalRead(reset))
    {
      action = RESET_ACTION;
    }
  #elif MODE_OF_INTERACTION == MODE_RF // TODO receive & interpret commands
    if(mySwitch.available())
    {
      Serial.print("Received ");
      Serial.print( mySwitch.getReceivedValue() & 0b11111111 ,DEC); // Get the data from the receiver
      Serial.print(" / ");
      Serial.print( mySwitch.getReceivedBitlength() );
      Serial.print("bit ");
      Serial.print("Protocol: ");
      Serial.println( mySwitch.getReceivedProtocol() );

      mySwitch.resetAvailable();
    }
  #endif

  if(action != NO_ACTION)
  {
    switch(action)
    {
      case RED_UP_ACTION:
        Serial.println(F("RED UP"));
        #if COUNTER_BEHAVIOUR == CYCLIC_BEHAVIOUR
          red = red==MAX_NUM?MIN_NUM:red+1;
        #elif COUNTER_BEHAVIOUR == LINEAL_BEHAVIOUR
          red = red==MAX_NUM?MAX_NUM:red+1;
        #endif
        break;

      case RED_DOWN_ACTION:
        Serial.println(F("RED DOWN"));
        #if COUNTER_BEHAVIOUR == CYCLIC_BEHAVIOUR
          red = red==MIN_NUM?MAX_NUM:red-1;
        #elif COUNTER_BEHAVIOUR == LINEAL_BEHAVIOUR
          red = red==MIN_NUM?MIN_NUM:red-1;
        #endif
        break;

      case BLUE_UP_ACTION:
        Serial.println(F("BLUE UP"));
        #if COUNTER_BEHAVIOUR == CYCLIC_BEHAVIOUR
          blue = blue==MAX_NUM?MIN_NUM:blue+1;
        #elif COUNTER_BEHAVIOUR == LINEAL_BEHAVIOUR
          blue = blue==MAX_NUM?MAX_NUM:blue+1;
        #endif
        break;

      case BLUE_DOWN_ACTION:
        Serial.println(F("BLUE DOWN"));
        #if COUNTER_BEHAVIOUR == CYCLIC_BEHAVIOUR
          blue = blue==MIN_NUM?MAX_NUM:blue-1;
        #elif COUNTER_BEHAVIOUR == LINEAL_BEHAVIOUR
          blue = blue==MIN_NUM?MIN_NUM:blue-1;
        #endif
        break;

      case RESET_ACTION:
        Serial.println(F("RESET"));
        red = MIN_NUM;
        blue = MIN_NUM;
    }
    updateSign();
    delay(DELAY);
    action = NO_ACTION;
	#if MODE_OF_INTERACTION == MODE_IR
	  IrReceiver.resume(); // Enable receiving of the next value
	#endif
  }
}
