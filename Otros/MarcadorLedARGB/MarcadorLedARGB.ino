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
#define ENABLE_EXTENDED_CAPABILITIES true // Activa esto (ponlo a true) para poder utilizar el modo de pantalla numerica de 4 digitos además del modo marcador
#define COUNTER_BEHAVIOUR CYCLIC_BEHAVIOUR // El modo del contador, ciclico: cuando llega al maximo, vuelve al minimo y viceversa; lineal: al llegar al maximo/minimo se queda en el
#define MODE_OF_INTERACTION MODE_IR // Aqui se selecciona el modo de interaccion
#define ALWAYS_USE_2_DIGITS true // a true para tener numeros de 0-9 de la forma 0X, y false para tenerlos de la forma X
#define WHITE false // si quieres probar la conexion con leds blancos, pon esto a true
#define DELAY 500 // el tiempo que tarda desde que pulsas un boton hasta que el arduino reconoce la siguente pulsacion
//////////////////////////////////////////     SETTINGS     ////////////////////////////////////////////////

//////////////////////////////////////////     GUARDS       ////////////////////////////////////////////////

// Modo cableado sin soporte (deprecated)
#if MODE_OF_INTERACTION == MODE_WIRED && ENABLE_EXTENDED_CAPABILITIES == true
  #error Wired connection is not supported while using the extended capabilities (aka extra modes)
#endif

// Modo por radio no desarrollado aun
#if MODE_OF_INTERACTION == MODE_RF
  #error RF capabilities not developed yet
#endif
//////////////////////////////////////////     GUARDS       ////////////////////////////////////////////////

//////////////////////////////////////////     I/O          ////////////////////////////////////////////////
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
  #define CHANGE_MODE_IR_CMD 0x33 // TODO poner el valor real del comando

  #include <IRremote.hpp>
#elif MODE_OF_INTERACTION == MODE_WIRED
  const byte redUpPin = 8;
  const byte redDownPin = 9;
  const byte blueUpPin = 10;
  const byte blueDownPin = 11;
  const byte reset = 12;
#elif MODE_OF_INTERACTION == MODE_RF

#endif

#include <FastLED.h>
#include "Digit.h"

struct CRGB leds[NUM_LEDS];
Digit lBlue(9,0,leds);
Digit rBlue(9,63,leds);
Digit lRed(9,126,leds);
Digit rRed(9,189,leds);
Digit digits[] = {lBlue,rBlue,lRed,rRed};
//////////////////////////////////////////     I/O          ////////////////////////////////////////////////

//////////////////////////////////////////     MENU         ////////////////////////////////////////////////
#if ENABLE_EXTENDED_CAPABILITIES
  typedef enum OperationMode {SHUTDOWN = 0, TRANSITION_TO_SHUTDOWN, SCOREBOARD, DISPLAY, MAX_OPERATION_MODE} Mode;
  Mode operationMode = TRANSITION_TO_SHUTDOWN;

  void changeOperationMode()
  {
    operationMode++;
    if(operationMode >= MAX_OPERATION_MODE)
    {
      operationMode = TRANSITION_TO_SHUTDOWN;
    }
  }
#endif
//////////////////////////////////////////     MENU         ////////////////////////////////////////////////

//////////////////////////////////////////     SCOREBOARD   ////////////////////////////////////////////////
typedef enum ActionEnum {NO_ACTION = 0, RED_UP_ACTION, RED_DOWN_ACTION, BLUE_UP_ACTION, BLUE_DOWN_ACTION, RESET_ACTION} Action;
Action action = NO_ACTION;
byte redTeamScore = MIN_NUM;
byte blueTeamScore = MIN_NUM;

void updateSign()
{
  byte num = (blueTeamScore/10)%10;
  #if !ALWAYS_USE_2_DIGITS
    if(num == 0)
    {
      num = ' ';
    }
  #endif
  lBlue.setDigit(num,0,0,255,0);
  rBlue.setDigit(blueTeamScore%10,0,0,255,0);

  num = (redTeamScore/10)%10;
  #if !ALWAYS_USE_2_DIGITS
    if(num == 0)
    {
      num = ' ';
    }
  #endif
  lRed.setDigit(num,255,0,0,0);
  rRed.setDigit(redTeamScore%10,255,0,0,0);

  FastLED.show();
}

void scoreboardLoop()
{
  action = NO_ACTION;

  #if MODE_OF_INTERACTION == MODE_IR
    if (IrReceiver.decode())
    {
      Serial.print(F("Decoded command: 0x"));
      Serial.println(IrReceiver.decodedIRData.command);
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
      #if ENABLE_EXTENDED_CAPABILITIES
        else if(IrReceiver.decodedIRData.command == CHANGE_MODE_IR_CMD)
        {
          changeOperationMode();
        }
      #endif
      else
      {
        IrReceiver.resume(); // Discard other values
      }
    }
  #elif MODE_OF_INTERACTION == MODE_WIRED
    if(!digitalRead(redUpPin))
    {
      action = RED_UP_ACTION;
    }
    else if(!digitalRead(redDownPin))
    {
      action = RED_DOWN_ACTION;
    }
    else if(!digitalRead(blueUpPin))
    {
      action = BLUE_UP_ACTION;
    }
    else if(!digitalRead(blueDownPin))
    {
      action = BLUE_DOWN_ACTION;
    }
    else if(!digitalRead(reset))
    {
      action = RESET_ACTION;
    }
  #elif MODE_OF_INTERACTION == MODE_IR

  #endif

  if(action != NO_ACTION)
  {
    switch(action)
    {
      case RED_UP_ACTION:
        Serial.println(F("RED UP"));
        #if COUNTER_BEHAVIOUR == CYCLIC_BEHAVIOUR
          redTeamScore = redTeamScore==MAX_NUM?MIN_NUM:redTeamScore+1;
        #elif COUNTER_BEHAVIOUR == LINEAL_BEHAVIOUR
          redTeamScore = redTeamScore==MAX_NUM?MAX_NUM:redTeamScore+1;
        #endif
        break;

      case RED_DOWN_ACTION:
        Serial.println(F("RED DOWN"));
        #if COUNTER_BEHAVIOUR == CYCLIC_BEHAVIOUR
          redTeamScore = redTeamScore==MIN_NUM?MAX_NUM:redTeamScore-1;
        #elif COUNTER_BEHAVIOUR == LINEAL_BEHAVIOUR
          redTeamScore = redTeamScore==MIN_NUM?MIN_NUM:redTeamScore-1;
        #endif
        break;

      case BLUE_UP_ACTION:
        Serial.println(F("BLUE UP"));
        #if COUNTER_BEHAVIOUR == CYCLIC_BEHAVIOUR
          blueTeamScore = blueTeamScore==MAX_NUM?MIN_NUM:blueTeamScore+1;
        #elif COUNTER_BEHAVIOUR == LINEAL_BEHAVIOUR
          blueTeamScore = blueTeamScore==MAX_NUM?MAX_NUM:blueTeamScore+1;
        #endif
        break;

      case BLUE_DOWN_ACTION:
        Serial.println(F("BLUE DOWN"));
        #if COUNTER_BEHAVIOUR == CYCLIC_BEHAVIOUR
          blueTeamScore = blueTeamScore==MIN_NUM?MAX_NUM:blueTeamScore-1;
        #elif COUNTER_BEHAVIOUR == LINEAL_BEHAVIOUR
          blueTeamScore = blueTeamScore==MIN_NUM?MIN_NUM:blueTeamScore-1;
        #endif
        break;

      case RESET_ACTION:
        Serial.println(F("RESET"));
        redTeamScore = MIN_NUM;
        blueTeamScore = MIN_NUM;
    }
    updateSign();
    delay(DELAY);
	#if MODE_OF_INTERACTION == MODE_IR
	  IrReceiver.resume(); // Enable receiving of the next value
	#endif
  }
}
//////////////////////////////////////////     SCOREBOARD   ////////////////////////////////////////////////

//////////////////////////////////////////     DISPLAY      ////////////////////////////////////////////////
#if ENABLE_EXTENDED_CAPABILITIES
  void displayLoop()
  {
    // TODO change mode && everything
  }
#endif
//////////////////////////////////////////     DISPLAY      ////////////////////////////////////////////////

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
  // TODO replace with setDigitArray
  FastLED.show();

  #if MODE_OF_INTERACTION == MODE_WIRED
    pinMode(redUpPin,INPUT_PULLUP);
    pinMode(redDownPin,INPUT_PULLUP);
    pinMode(blueUpPin,INPUT_PULLUP);
    pinMode(blueDownPin,INPUT_PULLUP);
    pinMode(reset,INPUT_PULLUP);
  #elif MODE_OF_INTERACTION == MODE_IR
    IrReceiver.begin(IR_RECEIVE_PIN, ENABLE_LED_FEEDBACK);
  #elif MODE_OF_INTERACTION == MODE_RF

  #endif

  delay(5000);
  updateSign();
  Serial.println(F("READY"));
}

void loop()
{
  #if ENABLE_EXTENDED_CAPABILITIES
    if(operationMode == TRANSITION_TO_SHUTDOWN)
    {
      // TODO TRANSITION_TO_SHUTDOWN
      operationMode = SHUTDOWN;
    }
    else if(operationMode == SHUTDOWN)
    {
      // TODO change mode
    }
    else if(operationMode == SCOREBOARD)
    {
      scoreboardLoop();
    }
    else if(operationMode ==  DISPLAY)
    {
      displayLoop();
    }
  #else
    scoreboardLoop();
  #endif
  
}
