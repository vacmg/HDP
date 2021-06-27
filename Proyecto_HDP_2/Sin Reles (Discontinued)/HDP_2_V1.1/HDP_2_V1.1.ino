/////////////////////////////////////////////SETTINGS

#define debug 0
const float vmax = 15.5;
const float vmin = 15;
const float vllave = 14;
const float vllenarmin = 12;
const float vllenarmax = 15;
const float vstart = 15;
unsigned const long llaveTime = 10000; // en ms
unsigned const long riegoTime = 2400000; // en ms (40 min) 2400000
#if debug
  unsigned int printTime = 500; // en ms
#endif
/////////////////////////////////////////////SETTINGS

/////////////////////////////////////////////DECLARACION

const int led = 13;
const int bomba = 12;
const int llaveIz = 10;
const int llaveDer = 11;

const int boya = 2;
const int microADer = 9;
const int microCDer = 5;
const int microAIz = 7;
const int microCIz = 6;
const int progDer = 4;
const int progIz = 3;

const int voltPin = A0;
const int voltRelay = 8;

float volts;
bool eMicroADer;
bool eMicroAIz;
bool eMicroCDer;
bool eMicroCIz;
bool eBoya;
bool eProgDer;
bool eProgIz;
bool llenarst;
#if debug
  unsigned long antes;
  unsigned long ahora;
  unsigned long tiempo;
#endif

/////////////////////////////////////////////DECLARACION

/////////////////////////////////////////////SETUP

void setup()
{
  #if debug
    Serial.begin(115200);
    delay(100);
    Serial.println(F("BOOTING..."));
    delay(1000);
  #endif
  digitalWrite(bomba,1);
  digitalWrite(llaveDer,1);
  digitalWrite(llaveIz,1);
  digitalWrite(voltRelay,1);
  
  pinMode(boya,INPUT);
  pinMode(bomba,OUTPUT);
  pinMode(llaveDer,OUTPUT);
  pinMode(microADer,INPUT);
  pinMode(microCDer,INPUT);
  pinMode(llaveIz,OUTPUT);
  pinMode(microAIz,INPUT);
  pinMode(microCIz,INPUT);
  pinMode(progDer,INPUT);
  pinMode(progIz,INPUT);
  pinMode(led,OUTPUT);
  pinMode(voltRelay,OUTPUT);
  vital();
  while (volts <= vstart)
  {
    #if debug
      Serial.println("Consiguiendo voltaje...");
    #endif
    vital();
  }
  vital();
  if (!llave('D',0))
  {
    error();
  }
  if (!llave('I',0))
  {
    error();
  }
  #if debug
    Serial.println(F("READY"));
    delay(1500);
  #endif
}

/////////////////////////////////////////////SETUP

/////////////////////////////////////////////LOOP

void loop()
{
  vital();
  //error();
  if (eProgDer == 1)
  {
    delay(5000);
    regar('D');
  }
  if (eProgIz == 1)
  {
    delay(5000);
    regar('I');
  }
}

/////////////////////////////////////////////LOOP

/////////////////////////////////////////////FUNCTIONS

void error()
{
  #if debug
    Serial.println(F("ERROR"));
    delay(2500);
  #endif
  while (true)
  {
    voltsVital();
    digitalWrite(led,1);
    delay(125);
    digitalWrite(led,0);
    delay(125);
  }
}

void regar(char id) // si 0 ==> derecha; si 1 ==> izquierda
{  
  if (!llave(id,1))
  {
    #if debug
      Serial.println(F("ERROR"));
      delay(2500);
    #endif
    return;
  }
  if (id == 'D')
  {
    delay(5000);
    vital();
    while(eProgDer == 1)
    {
      vital();
    }
  }
  else if (id == 'I')
  {
    delay(5000);
    vital();
    while(eProgIz == 1)
    {
      vital();
    }
  }
  if (!llave(id,0))
  {
    error();
  }
  #if debug
    Serial.println(F("Iniciando llenado de deposito..."));
    delay(750);
  #endif
  llenarst = 0;
  unsigned long tiempoAntes = millis();
  unsigned long tiempoAhora = millis();
  while (tiempoAhora-tiempoAntes <= riegoTime)
  {
    tiempoAhora = millis();
    #if debug
      Serial.println(tiempoAhora-tiempoAntes);
      Serial.flush();
    #endif
    mantenerCuboLleno();
  }
  digitalWrite(bomba,1); // apago bomba
}

void voltsVital()
{
  int sensorValue = analogRead(voltPin);          // realizar la lectura
  volts = fmap(sensorValue, 0, 1023, 0.0, 25.0);   // cambiar escala a 0.0 - 25.0
  if (volts >= vmax)
  {
    digitalWrite(voltRelay,0); // abrir // rele normanmente abierto
  }
  else if (volts <= vmin)
  {
    digitalWrite(voltRelay,1); // cerrar // rele normanmente abierto
  }
}

void vital()
{
  voltsVital();
  eBoya = digitalRead(boya);
  eMicroADer = digitalRead(microADer);
  eMicroCDer = digitalRead(microCDer);
  eMicroAIz = digitalRead(microAIz);
  eMicroCIz = digitalRead(microCIz);
  eProgDer = digitalRead(progDer);
  eProgIz = digitalRead(progIz);
  #if debug
    ahora = millis();
    tiempo = ahora - antes;
    if (tiempo >= printTime)
    {
      antes = millis();
      com();
    }
  #endif
}
 
float fmap(float x, float in_min, float in_max, float out_min, float out_max)
{
   return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

#if debug
  void com()
  {
    Serial.print(F("Voltaje:                   "));
    Serial.println(volts);
    Serial.println();
    Serial.print(F("Estado boya:               "));
    Serial.println(eBoya);
    Serial.println();
    Serial.print(F("Estado Micro Abierto Der:  "));
    Serial.println(eMicroADer);
    Serial.println();
    Serial.print(F("Estado Micro Cerrado Der:  "));
    Serial.println(eMicroCDer);
    Serial.println();
    Serial.print(F("Estado Micro Abierto Iz:   "));
    Serial.println(eMicroAIz);
    Serial.println();
    Serial.print(F("Estado Micro Cerrado Iz:   "));
    Serial.println(eMicroCIz);
    Serial.println();
    Serial.print(F("Estado Programador Der:    "));
    Serial.println(eProgDer);
    Serial.println();
    Serial.print(F("Estado Programador Iz:     "));
    Serial.println(eProgIz);
    Serial.println();
    Serial.println("\n\n\n\n");
    Serial.flush();
  }
#endif

void llaveVital( int microa, int microc, bool *emicroa, bool *emicroc)
{
  voltsVital();
  *emicroa = digitalRead(microa);
  *emicroc = digitalRead(microc);
  #if debug
    Serial.print(F("microa: "));
    if (microa == microAIz)
    {
      Serial.print(F("izquierda"));
    }
    else if (microa == microADer)
    {
      Serial.print(F("derecha"));
    }
    else
    {
      Serial.print(F("unknown"));
    }
    Serial.print(F("; emicroa: "));
    Serial.println(*emicroa);
    Serial.print(F("microc: "));
    if (microc == microCIz)
    {
      Serial.print(F("izquierda"));
    }
    else if (microc == microCDer)
    {
      Serial.print(F("derecha"));
    }
    else
    {
      Serial.print(F("unknown"));
    }
    Serial.print(F("; emicroc: "));
    Serial.println(*emicroc);
    Serial.flush();
  #endif
}

bool llave(char id, bool state) // resoult: 0 ==> fail; 1 ==> ok //state: 0 ==> cerrar; 1 ==> abrir
{
  vital();
  while (volts <= vllave)
  {
    #if debug
      Serial.println("Consiguiendo voltaje...");
    #endif
    vital();
  }
  bool resoult = 1;
  byte llave;
  int microC;
  int microA;
  bool eMicroA;
  bool eMicroC;
  
  unsigned long before,current,elapsed;
  if (id == 'D')
  {
   llave = llaveDer; 
   microC = microCDer;
   microA = microADer;
   #if debug
     Serial.println(F("LLave derecha seleccionada"));
   #endif
  }
  else if (id == 'I')
  {
    llave = llaveIz;
    microC = microCIz;
    microA = microAIz;
    #if debug
      Serial.println(F("LLave izquierda seleccionada"));
    #endif
  }
  llaveVital(microA,microC,&eMicroA,&eMicroC);
  before = millis();
  if (state == 0)
  {
    if (eMicroC == 0)
    {
      digitalWrite(llave,0); // encender
      while (eMicroC == 0 && resoult == 1)
      {
        current = millis();
        elapsed = current-before;
        if (elapsed >= llaveTime)
        {
          resoult = 0;
        }
        #if debug
          Serial.println(F("Cerrando llave..."));
          Serial.flush();
        #endif
        llaveVital(microA,microC,&eMicroA,&eMicroC);
      }
      digitalWrite(llave,1); // apagar
      #if debug
        if (resoult)
        {
          Serial.println(F("Llave cerrada"));
          delay(750);
        }
      #endif
    }
  }
  else
  {
    if (eMicroA == 0)
    {
      digitalWrite(llave,0); // encender
      while (eMicroA == 0 && resoult == 1)
      {
        current = millis();
        elapsed = current-before;
        if (elapsed >= llaveTime)
        {
          resoult = 0;
        }
        #if debug
          Serial.println(F("Abriendo llave..."));
          Serial.flush();
        #endif
        llaveVital(microA,microC,&eMicroA,&eMicroC);
      }
      digitalWrite(llave,1); // apagar
      #if debug
        if (resoult)
        {
          Serial.println(F("Llave abierta"));
          delay(750);
        }
      #endif
    }
  }
  return resoult;
}

void mantenerCuboLleno()
{
  vital();
  if (volts >= vllenarmax && llenarst == 0)
  {
    llenarst = 1;
  }
  if (volts <= vllenarmin && llenarst == 1)
  {
    llenarst = 0;
  }

  if (llenarst == 1)
  {
    if (eBoya == 1) // se ha llenado
    {
      digitalWrite(bomba,1); // apagado
    }
    else
    {
      digitalWrite(bomba,0); // encendido
    }
  }
  else
  {
    digitalWrite(bomba,1); // apagado
  }
}

/////////////////////////////////////////////FUNCTIONS
