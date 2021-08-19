/*

*/

//LIBRERIAS

#include <EEPROM.h>

//LIBRERIAS

//AJUSTES

#define debug 0
#define getciclosread 0
#define getciclosreset 0
#define abriralarrancar 1
#define registrodefallos 1

#if getciclosread
  #define onlyvoltcontrol 1
  #define getcicloswrite 0
#else

  #define onlyvoltcontrol 0
  #define getcicloswrite 1
#endif

#if registrodefallos
  #define maxciclosaraddr 20
  #define maxciclosabaddr 21
  #define bombaarnosubeaguaaddr 22
  #define boyaseguridadactivadaaddr 23
  #define numeroderiegosaddr 24
#endif


const float vmax[2] = {15.7, 15.7}; // voltaje maximo que alcanzan los condensadores // en V 
const float vload[2] = {14.5, 14.5}; // voltaje de histeresis de los condensadores (a este voltaje comienzan a cargarse los condensadores) (a este voltaje el sistema pasa a modo 1 (ON)) // en V
const float vmin[2] = {12, 12}; // voltaje minimo de funcionamiento del sistema (con que uno de ellos llegue se para el sistema) // en V

#if debug
  const long printTime = 1000; // tiempo entre refresco de datos (solo afecta a la comunicacion serial) // en ms
#endif

const unsigned long tiempo1 = 50000; // tiempo maximo que sube agua al cubo pequeno // en ms
const unsigned long pausa2  = 30000; // tiempo que esta en pausa el programa cuando se sobrepasa tiempo1 (tiempo de recuperacion de la bomba de abajo) // en ms
const unsigned long pausa3  = 10000; // tiempo que esta en pausa el programa cuando termina de subir al cubo pequeno // en ms
const unsigned long tiempo4 = 40000; // tiempo que esta subiendo agua al cubo grande // en ms
const unsigned long pausa5  = 10000; // tiempo que esta en pausa el programa cuando termina de subir agua al cubo grande (despues de vaciar tambien lo hace) // en ms
const unsigned int maxciclosar = 20; // nº de subidas de agua al cubo grande antes de saltar error si no se llena (util para comprobar fugas o si la bomba de arriba funciona) // TODO implementar esto
const unsigned int maxciclosab = 2; // nº de subidas de agua al cubo pequeno antes de saltar error si no se llena (util para comprobar fugas o si la bomba de arriba funciona) // TODO implementar esto

//AJUSTES

//E/S

unsigned const int vRele[2] = {12,7}; // vrele[1] va invertido (apagado (0) = 0, (1) = 1)
unsigned const int llave = 5;
unsigned const int bombaAr = 6;
unsigned const int bombaAb = 4;

unsigned const int boyaSeg = 2;
unsigned const int boyaCuPe = 3;
unsigned const int boyaAr = 8;
unsigned const int boyaAb = 9; // invertida
unsigned const int microa = 10;
unsigned const int microc = 11;

unsigned const int voltPin[2] = {A0, A1};

const bool on[2] = {1,0}; // On para cada vRele

//E/S

//VARIABLES

byte mode = 0;
bool error = false;
unsigned int ciclosar = 0;
unsigned int ciclosab = 0;
unsigned int ciclosri = 0;
bool ciclosrisw = 0;

float volt[2];

bool ebombaAr;
bool ebombaAb;

bool eboyaSeg;
bool eboyaCuPe;
bool eboyaAr;
bool eboyaAb;
bool emicroa;
bool emicroc;

unsigned long prevmillis;

#if debug
  unsigned long printmillis;
#endif

//VARIABLES

//SETUP

void setup()
{
  #if debug || getciclosread
    Serial.begin(115200);
    delay(500);
    Serial.println(F("Iniciando..."));
    delay(1000);
  #endif
  
  ebombaAr = 0;
  ebombaAb = 0;
  digitalWrite(llave, 0);
  digitalWrite(vRele[0],0); // apagado
  digitalWrite(vRele[1],1); // apagado // invertido
  updateRelays();

  pinMode(vRele[0],OUTPUT);
  pinMode(vRele[1],OUTPUT);
  pinMode(llave,OUTPUT);
  pinMode(bombaAr,OUTPUT);
  pinMode(bombaAb,OUTPUT);
  pinMode(boyaSeg,INPUT);
  pinMode(boyaCuPe,INPUT);
  pinMode(boyaAr,INPUT);
  pinMode(boyaAb,INPUT);
  pinMode(microa,INPUT);
  pinMode(microc,INPUT);

  unsigned int addr = 0;

  #if getciclosreset
    Serial.println(F("Reset ciclos"));
    addr = 0;
    for(byte i = 0; i<2;i++)
    {
      EEPROM.put(addr,0);
      addr+=sizeof(bool);
      EEPROM.put(addr,0);
      addr+=sizeof(unsigned int);
    }

  #endif

  

  #if getciclosread
  Serial.println(F("Reading ciclos"));
    addr = 0;
    bool ok[2];
    unsigned int data[2];
    for(byte i = 0; i<2;i++)
    {
      EEPROM.get(addr,ok[i]);
      Serial.println(ok[i]);
      addr+=sizeof(bool);
      EEPROM.get(addr,data[i]);
      Serial.println(data[i]);
      addr+=sizeof(unsigned int);
    }
  #endif

#if registrodefallos
  int data;
  Serial.print(F("Superado el numero maximo de ciclos de cupe: "));
  Serial.println(EEPROM.get(maxciclosabaddr,data));
  Serial.print(F("Superado el numero maximo de ciclos de cubo arriba: "));
  Serial.println(EEPROM.get(maxciclosaraddr,data));
  Serial.print(F("No succiona agua la bomba de arriba: "));
  Serial.println(EEPROM.get(bombaarnosubeaguaaddr,data));
  Serial.print(F("Se ha activado la boya de seguridad: "));
  Serial.println(EEPROM.get(boyaseguridadactivadaaddr,data));
  Serial.print(F("Numero de riegos diarios del dia del anterior reset: "));
  Serial.println(EEPROM.get(numeroderiegosaddr,data));
  if(EEPROM.get(numeroderiegosaddr,ciclosri) == 0)
  {
    ciclosrisw = 1;
  }
#endif

#if abriralarrancar
  #if debug
    Serial.println(F("Vaciando el deposito antes de comenzar"));
  #endif
  regar(1);
  while (eboyaAb != 0)
    vital();
#endif
  regar(0);
  
}

//SETUP

//LOOP

void loop()
{
  vital();
  #if onlyvoltcontrol
    mode = 0;
  #endif
  switch (mode)
  {
    case 0: // apagado
    ebombaAb = 0;
    ebombaAr = 0;
    updateRelays();
    break;
    
    case 1: // subir cupe
    if (prevmillis + tiempo1 < millis())
    {
      ebombaAb = 0;
      mode = 2; // pausa bombaAb
      ciclosab++;
      #if debug
        Serial.print(F("MODE set to: "));
        Serial.println(mode);
        delay(2000);
      #endif
      prevmillis = millis();
    }
    else if (eboyaCuPe == 0)
    {
      ebombaAb = 1;
    }
    else
    {
      ebombaAb = 0;
      mode = 3; // pausa cupe --> arr
      ciclosab++;
      #if getcicloswrite
        bool aux;
        if (!EEPROM.get(0,aux))
        {
          EEPROM.put(0,true);
          EEPROM.put(0+sizeof(bool),ciclosab);
        }
      #endif
      ciclosab = 0;
      #if debug
        Serial.print(F("MODE set to: "));
        Serial.println(mode);
        delay(2000);
      #endif
      prevmillis = millis();
    }
    updateRelays();
    break;

    case 2: // pausa bombaAb
    if (prevmillis + pausa2 < millis())
    {
      mode = 1; // subir cupe
      #if debug
        Serial.print(F("MODE set to: "));
        Serial.println(mode);
        delay(2000);
      #endif
    }
    break;

    case 3: // pausa cupe --> arr
    if (prevmillis + pausa3 < millis())
    {
      mode = 4; // subir arr
      #if debug
        Serial.print(F("MODE set to: "));
        Serial.println(mode);
        delay(2000);
      #endif
      prevmillis = millis();
    }
    break;
    
    case 4: // subir arr
    if (prevmillis + tiempo4 < millis())
    {
      ebombaAr = 0;
      ciclosar++;
      mode = 5; // pausa arr --> cupe
      #if debug
        Serial.print(F("MODE set to: "));
        Serial.println(mode);
        delay(2000);
      #endif
      if(eboyaCuPe == 1)
      {
        error = 1;
        #if registrodefallos
          bool aux;
          if(EEPROM.get(maxciclosabaddr,aux) == 0)
          {
            EEPROM.put(maxciclosabaddr,1);
          }
        #endif
        #if debug
          Serial.println(F("La bomba de arriba no funciona, no sube agua al cubo grande"));
        #endif
      }
      prevmillis = millis();
    }
    else if (eboyaAr == 0)
    {
      ebombaAr = 1;
    }
    else
    {
      ebombaAr = 0;
      ciclosar++;
      #if getcicloswrite
        bool aux;
        if (!EEPROM.get(sizeof(bool)+sizeof(int),aux))
        {
          EEPROM.put(sizeof(bool)+sizeof(unsigned int),true);
          EEPROM.put(sizeof(bool)+sizeof(unsigned int)+sizeof(bool),ciclosar);
        }
      #endif
      ciclosar = 0;
      mode = 6; // vaciar
      #if debug
        Serial.print(F("MODE set to: "));
        Serial.println(mode);
        delay(2000);
      #endif
    }
    updateRelays();
    break;

    case 5: // pausa arr --> cupe
    if (prevmillis + pausa5 < millis())
    {
      mode = 1; // subir cupe
      #if debug
        Serial.print(F("MODE set to: "));
        Serial.println(mode);
        delay(2000);
      #endif
      prevmillis = millis();
    }
    break;

    case 6: // vaciar
    if (eboyaAb == 0) // invertido
    {
      regar(0);
      #if registrodefallos
        if (ciclosrisw)
        {
          ciclosri++;
          EEPROM.put(numeroderiegosaddr,ciclosri);
        }
      #endif
      mode = 5; // pausa arr --> cupe
      #if debug
        Serial.print(F("MODE set to: "));
        Serial.println(mode);
        delay(2000);
      #endif
      prevmillis = millis();
    }
    else if (eboyaAr == 1)
    {
      regar(1);
    }
    break;
  }
  #if !(getciclosread || getciclosreset || getcicloswrite)
    errorDetection();
  #endif
  delay(100);
}

//LOOP

//FUNCIONES

void vital()
{
  voltControl();
  readSensors();
  checkMode();
  #if debug
    if(printmillis + printTime < millis())
    {
      printData();
      printmillis = millis();
    }
  #endif
}

void checkMode()
{
  if (mode != 0 && (volt[0] < vmin[0] || volt[1] < vmin[1] || eboyaSeg == 0)) // apagar si
  {
    mode = 0;
    #if registrodefallos
      bool aux;
      if(eboyaSeg == 0 && EEPROM.get(boyaseguridadactivadaaddr,aux) == 0)
      {
        EEPROM.put(boyaseguridadactivadaaddr,1);
      }
    #endif
    #if debug
      Serial.print(F("MODE set to: "));
      Serial.print(mode);
      Serial.println(F(" by checkMode"));
      delay(2000);
    #endif
  }
  else if (!onlyvoltcontrol && mode == 0 && (volt[0] >= vload[0] && volt[1] >= vload[1]) && eboyaSeg == 1) // encender si
  {
    mode = 1;
    #if debug
      Serial.print(F("MODE set to: "));
      Serial.print(mode);
      Serial.println(F(" by checkMode"));
      delay(2000);
    #endif
    prevmillis = millis();
  }
}

void regar(bool x)
{
  waitForVolt((float*)vmax);
  readSensors();
  if (x == 1)
  {
    if (emicroa == 1)
    {
      #if debug
        Serial.println(F("Abierto\n"));
      #endif
    }
    else
    {
      digitalWrite(llave,1); //on
      #if debug
        Serial.println(F("Abriendo...\n"));
      #endif
      while (emicroa == 0)
      {
        voltControl();
        readSensors();
      }
      digitalWrite(llave,0); //off
      #if debug
        Serial.println(F("Abierto\n"));
      #endif
    }
  }
  else
  {
    if (emicroc == 1)
    {
      #if debug
        Serial.println(F("Cerrado\n"));
      #endif
    }
    else
    {
      digitalWrite(llave,1); //on
      #if debug
        Serial.println(F("Cerrando\n"));
      #endif
      while (emicroc == 0)
      {
        voltControl();
        readSensors();
      }
      digitalWrite(llave,0); //off
      #if debug
        Serial.println(F("Cerrado\n"));
      #endif
    }
  }
}

void voltControl()
{
  readVolts();
  for (byte i = 0; i<2; i++)
  {
    if (volt[i] <= vload[i])
    {
      digitalWrite(vRele[i],on[i]); // carga
    }
    if (volt[i] >= vmax[i])
    {
      digitalWrite(vRele[i],!on[i]); // no carga
    }
  }
}

void updateRelays()
{
  digitalWrite(bombaAr, ebombaAr);
  digitalWrite(bombaAb, ebombaAb);
}

void errorDetection()
{
  if(ciclosar>=maxciclosar)
  {
    #if debug
      Serial.print(F("Error al subir agua al cubo grande: superados el numero de ciclos permitidos.\t"));
      Serial.println(ciclosar);
    #endif
    #if registrodefallos
      bool aux;
      if(EEPROM.get(bombaarnosubeaguaaddr,aux) == 0)
      {
        EEPROM.put(bombaarnosubeaguaaddr,1);
      }
    #endif
    error = 1;
  }
  if(ciclosab>=maxciclosab)
  {
    #if debug
      Serial.print(F("Error al subir agua al cubo pequeno: superados el numero de ciclos permitidos.\t"));
      Serial.println(ciclosab);
    #endif
    #if registrodefallos
      bool aux;
      if(EEPROM.get(maxciclosabaddr,aux) == 0)
      {
        EEPROM.put(maxciclosabaddr,1);
      }
    #endif
    error = 1;
  }
  if (error)
  {
    ebombaAr = 0;
    ebombaAb = 0;
    updateRelays();
    while(true)
    {
      voltControl();
    }
  }
}

void waitForVolt(float getVolt[2])
{
  digitalWrite(vRele[0], !on[0]);
  digitalWrite(vRele[1], !on[1]);
  for (int i=0;i<2;i++)
  {
    digitalWrite(vRele[i],on[i]);
    #if debug
      Serial.print(F("Started charging process of zone "));
      Serial.println(i);
    #endif
    do
    {
      readVolts();
      #if debug
        Serial.print("Getting ");
        Serial.print(getVolt[i]);
        Serial.println("V...");
        Serial.print("Current voltage of zone ");
        Serial.print(i);
        Serial.print(": ");
        Serial.print(volt[i]);
        Serial.println("V\n");
        delay(2000);
      #endif
    }
    while (volt[i] < getVolt[i]);
    
    digitalWrite(vRele[i],!on[i]);
    #if debug
        Serial.print(F("Finished charging process of zone "));
      Serial.println(i);
    #endif
  }
}

void readVolts()
{
  for (int i = 0;i<2;i++)
  {
    volt[i] = floatmap(analogRead(voltPin[i]), 0, 1023, 0.0, 25.0);
  }
}

void readSensors()
{
  eboyaSeg = digitalRead(boyaSeg);
  eboyaCuPe = digitalRead(boyaCuPe);
  eboyaAr = digitalRead(boyaAr);
  eboyaAb = !digitalRead(boyaAb); // ! // INVERTIDO
  emicroa = digitalRead(microa);
  emicroc = digitalRead(microc);
}

float floatmap(float x, float in_min, float in_max, float out_min, float out_max)
{
   return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

#if debug

void printModeStr()
{
  switch(mode)
  {
    case 0:
    Serial.print(F("OFF"));
    break;
    case 1:
    Serial.print(F("SUBIR CUPE"));
    break;
    case 2:
    Serial.print(F("PAUSA BOMBA ABAJO"));
    break;
    case 3:
    Serial.print(F("PAUSA CUPE --> ARR"));
    break;
    case 4:
    Serial.print(F("SUBIR ARR"));
    break;
    case 5:
    Serial.print(F("PAUSA ARR --> CUPE"));
    break;
    case 6:
    Serial.print(F("VACIAR"));
    break;
  }
}

void printData()
{
  Serial.print(F("MODE: "));
  Serial.print(mode);
  Serial.print(F(" ("));
  printModeStr();
  Serial.println(F(")"));
  Serial.print(F("Current timer: "));
  Serial.println(millis()-prevmillis);
  Serial.println();

  for (int i= 0; i<2;i++)
  {
    Serial.print("Voltaje de ");
    Serial.print(i);
    Serial.print(": ");
    Serial.print(volt[i]);
    Serial.println(" V");
  }
  Serial.println();
  
  Serial.print(F("Boya de seguridad: "));
  Serial.println(eboyaSeg);
  Serial.print(F("Boya del cubo pequeno: "));
  Serial.println(eboyaCuPe);
  Serial.print(F("Boya de arriba: "));
  Serial.println(eboyaAr);
  Serial.print(F("Boya de abajo: "));
  Serial.println(eboyaAb);
  Serial.print(F("Micro abierto: "));
  Serial.println(emicroa);
  Serial.print(F("Micro cerrado: "));
  Serial.println(emicroc);
  Serial.println();

  Serial.print(F("Bomba Abajo: "));
  Serial.println(ebombaAb);
  Serial.print(F("Bomba Arriba: "));
  Serial.println(ebombaAr);
  Serial.println();

  Serial.print(F("Ciclos Bomba Abajo: "));
  Serial.println(ciclosab);
  Serial.print(F("Ciclos Bomba Arriba: "));
  Serial.println(ciclosar);
  Serial.println();
}
#endif

//FUNCIONES
