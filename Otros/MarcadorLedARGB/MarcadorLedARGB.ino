/**
 * Codigo fuente de un marcador de pelota vasca hecho a partir de una tira led aRGB y que puede utilizar uno de los siguientes modos de funcionamiento:
 * Por cable y botones; Por un mando infrarrojos.
**/

#define MODE_SERIAL 0
#define MODE_WIRED 1
#define MODE_IR 2
#define MODE_RF 3
#define CYCLIC_BEHAVIOUR 0
#define LINEAL_BEHAVIOUR 1

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)
#define AT " AT " __FILE__ ":" TOSTRING(__LINE__) ": "
#define pause() do{Serial.println(AT"press any key to continue");while(!Serial.available());while(Serial.available()){Serial.read();}}while(0)

//////////////////////////////////////////     SETTINGS     ////////////////////////////////////////////////
#define USE_MODES true // true si quieres usar la nueva version con el modo teclado para escribir numeros
#define COUNTER_BEHAVIOUR CYCLIC_BEHAVIOUR // El modo del contador, ciclico: cuando llega al maximo, vuelve al minimo y viceversa; lineal: al llegar al maximo/minimo se queda en el
#define MODE_OF_INTERACTION MODE_IR // Aqui se selecciona el modo de interaccion
#define ALWAYS_USE_2_DIGITS true // a true para tener numeros de 0-9 de la forma 0X, y false para tenerlos de la forma X
#define WHITE false // si quieres probar la conexion con leds blancos, pon esto a true
#define DELAY 500 // el tiempo que tarda desde que pulsas un boton hasta que el arduino reconoce la siguente pulsacion
#define DARK_MODE false // Si vale true, disminuye el brillo de los leds 
//////////////////////////////////////////     SETTINGS     ////////////////////////////////////////////////

//////////////////////////////////////////     CHECKS     //////////////////////////////////////////////////

#if USE_MODES && MODE_OF_INTERACTION != MODE_SERIAL && MODE_OF_INTERACTION != MODE_IR
  #error Incompatible set of configurations USE_MODES == true && MODE_OF_INTERACTION != MODE_SERIAL && MODE_OF_INTERACTION != MODE_IR
#endif

#if MODE_OF_INTERACTION == MODE_RF
  #error MODE_RF NOT READY YET
#endif

//////////////////////////////////////////     CHECKS     //////////////////////////////////////////////////

//////////////////////////////////////////     I/O     /////////////////////////////////////////////////////
#define LEDS_DATA_PIN 6
#define NUM_LEDS 252
#define MAX_NUM 99
#define MIN_NUM 0

#if MODE_OF_INTERACTION == MODE_IR
  #define IR_RECEIVE_PIN 2 // To be compatible with interrupt example, pin 2 is chosen here.
  #define DECODE_NEC // Includes Apple and Onkyo

  #define RED_UP_IR_CMD 0x40 // NEXT
  #define RED_DOWN_IR_CMD 0x44 // PREV
  #define BLUE_UP_IR_CMD 0x15 // +
  #define BLUE_DOWN_IR_CMD 0x7 // -
  #define MATCH_RESET_IR_CMD 0x46 // CH
  #define MODE_CHANGE_IR_CMD 0x9 // EQ
  #define KEYBOARD_CLEAR_IR_CMD 0x43 // Play/Pause
  #define KEYBOARD_DELETE_NUM_IR_CMD 0x19 // +100
  #define KEYBOARD_VALIDATE_IR_CMD 0xD // +200
  #define KEYBOARD_NUM_0_IR_CMD 0x16
  #define KEYBOARD_NUM_1_IR_CMD 0xC
  #define KEYBOARD_NUM_2_IR_CMD 0x18
  #define KEYBOARD_NUM_3_IR_CMD 0x5E
  #define KEYBOARD_NUM_4_IR_CMD 0x8
  #define KEYBOARD_NUM_5_IR_CMD 0x1C
  #define KEYBOARD_NUM_6_IR_CMD 0x5A
  #define KEYBOARD_NUM_7_IR_CMD 0x42
  #define KEYBOARD_NUM_8_IR_CMD 0x52
  #define KEYBOARD_NUM_9_IR_CMD 0x4A

  // KEYBOARD_NUM_ACTION} Action;


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
      #if DARK_MODE
        ledsDataArray[pos].r = r==0?0:(r/10)+1;
        ledsDataArray[pos].g = g==0?0:(g/10)+1;
        ledsDataArray[pos].b = b==0?0:(b/10)+1;
      #else
        ledsDataArray[pos].r = r;
        ledsDataArray[pos].g = g;
        ledsDataArray[pos].b = b;
      #endif
      pos++;
    }
    if (show)
      FastLED.show();
  }

  void setDigit(char digit, int r, int g, int b, bool show = true)
  {
    switch(digit)
    {
      case ' ':
      setSegment(top,0,0,0,0);
      setSegment(topLeft,0,0,0,0);
      setSegment(topRight,0,0,0,0);
      setSegment(bottom,0,0,0,0);
      setSegment(bottomLeft,0,0,0,0);
      setSegment(bottomRight,0,0,0,0);
      setSegment(center,0,0,0,0);
      break;
      case '-':
      setSegment(top,0,0,0,0);
      setSegment(topLeft,0,0,0,0);
      setSegment(topRight,0,0,0,0);
      setSegment(bottom,0,0,0,0);
      setSegment(bottomLeft,0,0,0,0);
      setSegment(bottomRight,0,0,0,0);
      setSegment(center,r,g,b,0);
      break;
      case '_':
      setSegment(top,0,0,0,0);
      setSegment(topLeft,0,0,0,0);
      setSegment(topRight,0,0,0,0);
      setSegment(bottom,r,g,b,0);
      setSegment(bottomLeft,0,0,0,0);
      setSegment(bottomRight,0,0,0,0);
      setSegment(center,0,0,0,0);
      break;
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
      setSegment(top,r,g,b,0);
      setSegment(topLeft,0,0,0,0);
      setSegment(topRight,r,g,b,0);
      setSegment(bottom,0,0,0,0);
      setSegment(bottomLeft,r,g,b,0);
      setSegment(bottomRight,0,0,0,0);
      setSegment(center,r,g,b,0);
    }
    if (show)
      FastLED.show();
  }

  private:
    struct CRGB *ledsDataArray;
    int ledsPerSegment, firstLedPos;
    int segmentPosOffset[7];
};

typedef enum {NO_ACTION = 0, MODE_CHANGE_ACTION, MATCH_RED_UP_ACTION, MATCH_RED_DOWN_ACTION, MATCH_BLUE_UP_ACTION, MATCH_BLUE_DOWN_ACTION, MATCH_RESET_ACTION, KEYBOARD_CLEAR_ACTION, KEYBOARD_VALIDATE_ACTION, KEYBOARD_NUM_ACTION, KEYBOARD_DELETE_NUM_ACTION } Action;
Action action = NO_ACTION;

#if USE_MODES
typedef enum {MODE_OFF = 0, MODE_MATCH, MODE_KEYBOARD, MODE_NULL} Mode;
Mode mode = MODE_OFF;
#endif

struct CRGB leds[NUM_LEDS];
Digit lBlue(9,0,leds);
Digit rBlue(9,63,leds);
Digit lRed(9,126,leds);
Digit rRed(9,189,leds);

byte red = MIN_NUM;
byte blue = MIN_NUM;
int keyboardNum = -1;
byte keyboardDigit = 0;

void updateSign(byte red, byte blue)
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

void updateSign(uint16_t num, byte r = 255, byte g = 255, byte b = 255)
{
  byte digit = num%10;
  rRed.setDigit(digit+'0', r, g, b);

  num/=10;
  digit = num%10;
  lRed.setDigit(digit==0 && num == 0?' ':digit+'0', r, g, b);

  num/=10;
  digit = num%10;
  rBlue.setDigit(digit==0 && num == 0?' ':digit+'0', r, g, b);

  num/=10;
  digit = num%10;
  lBlue.setDigit(digit==0 && num == 0?' ':digit+'0', r, g, b);
}

void updateSign(const char str[5], byte r = 255, byte g = 255, byte b = 255)
{
  lBlue.setDigit(str[0], r, g, b);
  rBlue.setDigit(str[1], r, g, b);
  lRed.setDigit(str[2], r, g, b);
  rRed.setDigit(str[3], r, g, b);
}

void handleMatchAction()
{
  switch(action)
  {
    case MATCH_RED_UP_ACTION:
      Serial.println(F("MATCH RED UP"));
      #if COUNTER_BEHAVIOUR == CYCLIC_BEHAVIOUR
        red = red==MAX_NUM?MIN_NUM:red+1;
      #elif COUNTER_BEHAVIOUR == LINEAL_BEHAVIOUR
        red = red==MAX_NUM?MAX_NUM:red+1;
      #endif
      break;

    case MATCH_RED_DOWN_ACTION:
      Serial.println(F("MATCH RED DOWN"));
      #if COUNTER_BEHAVIOUR == CYCLIC_BEHAVIOUR
        red = red==MIN_NUM?MAX_NUM:red-1;
      #elif COUNTER_BEHAVIOUR == LINEAL_BEHAVIOUR
        red = red==MIN_NUM?MIN_NUM:red-1;
      #endif
      break;

    case MATCH_BLUE_UP_ACTION:
      Serial.println(F("MATCH BLUE UP"));
      #if COUNTER_BEHAVIOUR == CYCLIC_BEHAVIOUR
        blue = blue==MAX_NUM?MIN_NUM:blue+1;
      #elif COUNTER_BEHAVIOUR == LINEAL_BEHAVIOUR
        blue = blue==MAX_NUM?MAX_NUM:blue+1;
      #endif
      break;

    case MATCH_BLUE_DOWN_ACTION:
      Serial.println(F("MATCH BLUE DOWN"));
      #if COUNTER_BEHAVIOUR == CYCLIC_BEHAVIOUR
        blue = blue==MIN_NUM?MAX_NUM:blue-1;
      #elif COUNTER_BEHAVIOUR == LINEAL_BEHAVIOUR
        blue = blue==MIN_NUM?MIN_NUM:blue-1;
      #endif
      break;

    case MATCH_RESET_ACTION:
      Serial.println(F("MATCH RESET"));
      red = MIN_NUM;
      blue = MIN_NUM;
      break;
    default:
      Serial.print(F("ERROR: Unknown Action: "));
      Serial.println(action);
  }

  updateSign(red,blue);
  delay(DELAY);
}

#if USE_MODES
void handleKeyboardAction()
{
  switch(action)
  {
    case KEYBOARD_CLEAR_ACTION:
      Serial.println(F("KEYBOARD CLEAR SIGN"));
      updateSign("----");
      keyboardNum = -1;
      break;
    case KEYBOARD_VALIDATE_ACTION:
      Serial.println(F("KEYBOARD VALIDATE"));
      if(keyboardNum!=-1)
      {
        updateSign(keyboardNum,30,255,60);
        keyboardNum = -1;
      }
      break;
    case KEYBOARD_NUM_ACTION:
      Serial.print(F("KEYBOARD NUM SELECTED: "));Serial.println(keyboardDigit);
      if(keyboardNum == -1)
      {
        keyboardNum = keyboardDigit;
        updateSign(keyboardNum);
      }
      else if(keyboardNum<1000)
      {
        keyboardNum=(keyboardNum*10)+keyboardDigit;
        updateSign(keyboardNum);
      }
      break;
    case KEYBOARD_DELETE_NUM_ACTION:
      Serial.println(F("KEYBOARD DELETE NUM"));
      if(keyboardNum < 10)
      {
        action = KEYBOARD_CLEAR_ACTION;
        handleKeyboardAction();
      }
      else
      {
        keyboardNum/=10;
        updateSign(keyboardNum);
      }
      break;
    default:
      Serial.print(F("ERROR: Unknown Action: "));
      Serial.println(action);
  }
  delay(DELAY);
}
#endif

void setup()
{
  FastLED.addLeds<NEOPIXEL,LEDS_DATA_PIN>(leds, NUM_LEDS);
  #if DARK_MODE
    fill_solid(leds, NUM_LEDS,0x111111);
  #else
    fill_solid(leds, NUM_LEDS,CRGB::White);
  #endif
  FastLED.show();
  #if WHITE
    while (true);
  #endif

  Serial.begin(115200);
  delay(1000);
  Serial.println(F("STARTING"));
  #if USE_MODES
  updateSign("HDP2",0,255,30);
  #else
  updateSign("HDP ",0,255,30);
  #endif
  delay(2500);

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
      Serial.println("RF Connection Error");
      lBlue.setDigit('E',255,0,0);
      rBlue.setDigit('R',255,0,0);
      lRed.setDigit('R',255,0,0);
      rRed.setDigit('C',255,255,255);
      FastLED.show();
      while(1);
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

  delay(2500);
  #if USE_MODES
    updateSign("OFF ");
    delay(2000);
    updateSign("    ");
  #else
    action = MATCH_RESET_ACTION;
  #endif
  Serial.println(F("READY"));
}

void loop()
{
  #if USE_MODES
  if(action == MODE_CHANGE_ACTION)
  {
    Serial.print(F("Changed mode from ")); Serial.print(mode);
    mode = mode +1;
    Serial.print(F(" to ")); Serial.println(mode);

    switch(mode)
    {
      case MODE_MATCH:
        action = MATCH_RESET_ACTION;
        break;
      case MODE_KEYBOARD:
        action = KEYBOARD_CLEAR_ACTION;
        break;
      case MODE_NULL:
        mode = MODE_OFF;
      case MODE_OFF:
        updateSign("OFF ");
        delay(2000);
        updateSign("    ");
        action = NO_ACTION;
        #if MODE_OF_INTERACTION == MODE_IR
	        IrReceiver.resume(); // Enable receiving of the next value
	      #endif
        break;
    }
  }
  #endif
  if(action != NO_ACTION)
  {
    #if USE_MODES
    switch(mode)
    {
      case MODE_MATCH:
        Serial.println(F("Using MODE_MATCH"));
        handleMatchAction();
        break;
      case MODE_KEYBOARD:
      Serial.println(F("Using MODE_KEYBOARD"));
        handleKeyboardAction();
        break;
      case MODE_NULL:
      Serial.println(F("Using MODE_NULL"));
        mode = MODE_OFF;
        break;
    }
    #else
      handleMatchAction();
    #endif
    action = NO_ACTION;
	  #if MODE_OF_INTERACTION == MODE_IR
	    IrReceiver.resume(); // Enable receiving of the next value
	  #endif
  }


  #if MODE_OF_INTERACTION == MODE_IR
    if (IrReceiver.decode())
    {
      switch(IrReceiver.decodedIRData.command)
      {
        case RED_UP_IR_CMD:
          action = MATCH_RED_UP_ACTION;
          break;
        case RED_DOWN_IR_CMD:
          action = MATCH_RED_DOWN_ACTION;
          break;
        case BLUE_UP_IR_CMD:
          action = MATCH_BLUE_UP_ACTION;
          break;
        case BLUE_DOWN_IR_CMD:
          action = MATCH_BLUE_DOWN_ACTION;
          break;
        case MATCH_RESET_IR_CMD:
          action = MATCH_RESET_ACTION;
          break;
        case MODE_CHANGE_IR_CMD:
          action = MODE_CHANGE_ACTION;
          break;
        case KEYBOARD_CLEAR_IR_CMD:
          action = KEYBOARD_CLEAR_ACTION;
          break;
        case KEYBOARD_DELETE_NUM_IR_CMD:
          action = KEYBOARD_DELETE_NUM_ACTION;
          break;
        case KEYBOARD_VALIDATE_IR_CMD:
          action = KEYBOARD_VALIDATE_ACTION;
          break;
        case KEYBOARD_NUM_0_IR_CMD:
          keyboardDigit = 0;
          action = KEYBOARD_NUM_ACTION;
          break;
        case KEYBOARD_NUM_1_IR_CMD:
          keyboardDigit = 1;
          action = KEYBOARD_NUM_ACTION;
          break;
        case KEYBOARD_NUM_2_IR_CMD:
          keyboardDigit = 2;
          action = KEYBOARD_NUM_ACTION;
          break;
        case KEYBOARD_NUM_3_IR_CMD:
          keyboardDigit = 3;
          action = KEYBOARD_NUM_ACTION;
          break;
        case KEYBOARD_NUM_4_IR_CMD:
          keyboardDigit = 4;
          action = KEYBOARD_NUM_ACTION;
          break;
        case KEYBOARD_NUM_5_IR_CMD:
          keyboardDigit = 5;
          action = KEYBOARD_NUM_ACTION;
          break;
        case KEYBOARD_NUM_6_IR_CMD:
          keyboardDigit = 6;
          action = KEYBOARD_NUM_ACTION;
          break;
        case KEYBOARD_NUM_7_IR_CMD:
          keyboardDigit = 7;
          action = KEYBOARD_NUM_ACTION;
          break;
        case KEYBOARD_NUM_8_IR_CMD:
          keyboardDigit = 8;
          action = KEYBOARD_NUM_ACTION;
          break;
        case KEYBOARD_NUM_9_IR_CMD:
          keyboardDigit = 9;
          action = KEYBOARD_NUM_ACTION;
          break;
        default:
          Serial.print(F("Unknown detected IR command: 0x"));Serial.println(IrReceiver.decodedIRData.command, HEX);
          IrReceiver.resume(); // Discard other values
      }
    }
  #elif MODE_OF_INTERACTION == MODE_WIRED
    if(!digitalRead(redUp))
    {
      action = MATCH_RED_UP_ACTION;
    }
    else if(!digitalRead(redDown))
    {
      action = MATCH_RED_DOWN_ACTION;
    }
    else if(!digitalRead(blueUp))
    {
      action = MATCH_BLUE_UP_ACTION;
    }
    else if(!digitalRead(blueDown))
    {
      action = MATCH_BLUE_DOWN_ACTION;
    }
    else if(!digitalRead(reset))
    {
      action = MATCH_RESET_ACTION;
    }
  #elif MODE_OF_INTERACTION == MODE_RF // TODO receive & interpret commands
    if(mySwitch.available())
    {
      uint16_t value = mySwitch.getReceivedValue() & 0b11111111;
      Serial.print("Received ");
      Serial.print( value, DEC); // Get the data from the receiver
      Serial.print(" / ");
      Serial.print( mySwitch.getReceivedBitlength() );
      Serial.print("bit ");
      Serial.print("Protocol: ");
      Serial.println( mySwitch.getReceivedProtocol() );

      mySwitch.resetAvailable();
    }
  #elif MODE_OF_INTERACTION == MODE_SERIAL
    if(Serial.available())
    {
      char buff[64];
      delay(100);
      size_t len = Serial.readBytesUntil('\n',buff,sizeof(buff));
      buff[len] = '\0';

      Serial.print(F("Received: "));Serial.println(buff);
      
      if(strcmp(buff,"mrup") == 0)
      {
        action = MATCH_RED_UP_ACTION;
      }
      else if(strcmp(buff,"mrdown") == 0)
      {
        action = MATCH_RED_DOWN_ACTION;
      }
      else if(strcmp(buff,"mbup") == 0)
      {
        action = MATCH_BLUE_UP_ACTION;
      }
      else if(strcmp(buff,"mbdown") == 0)
      {
        action = MATCH_BLUE_DOWN_ACTION;
      }
      else if(strcmp(buff,"mreset") == 0)
      {
        action = MATCH_RESET_ACTION;
      }
      #if USE_MODES
      else if(strcmp(buff,"changemode") == 0)
      {
        action = MODE_CHANGE_ACTION;
      }
      else if(strcmp(buff,"kclear") == 0)
      {
        action = KEYBOARD_CLEAR_ACTION;
      }
      else if(strcmp(buff,"kok") == 0)
      {
        action = KEYBOARD_VALIDATE_ACTION;
      }
      else if(strcmp(buff,"kdel") == 0)
      {
        action = KEYBOARD_DELETE_NUM_ACTION;
      }
      else
      {
        keyboardDigit = buff[len-1] - '0';
        buff[len-1] = '\0';
        if(strcmp(buff,"knum") == 0 || strcmp(buff,"knum ") == 0)
        {
          action = KEYBOARD_NUM_ACTION;
        }
        else
        {
          Serial.println(F("Commands list:\nhelp: show this diagram\nchangemode: changes the operation mode between OFF, MATCH & KEYBOARD\nmrup, mrdown, mbup, mbdown: set the red/blue score 1 point up/down\nmreset: reset the match scores to 0\nkclear: clears the screen\nknum N: input the digit N\nkdel: deletes the last digit\nkok: validates the number"));
        }        
      }
      #else
      else
      {
        Serial.println(F("Commands list:\nhelp: show this diagram\nmrup, mrdown, mbup, mbdown: set the red/blue score 1 point up/down\nmreset: reset the match scores to 0"));
      }
      #endif
    }
  #endif
}
