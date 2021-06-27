/////////////////////////////////////////////SETTINGS

#define debug 1 // 10% vs 21% ROM
const float vmax = 15.5; // voltaje maximo de carga / flotacion
const float vmin = 15; // voltaje minimo de flotacion / inicio de carga 
const float vllave = 14; // voltaje minimo para accionar la llave (se mantiene en carga hasta conseguirlo) (debe ser MENOR que vmin)
const float vllenarmin = 12; // por debajo de este voltaje no llena el cubo
const float vllenarmax = 15; // por encima de este voltaje empieza a llenar el cubo (el sistema llena el cubo hasta llegar a vllenarmin)
const float vstart = 15; // voltaje necesario para iniciar el sistema (garantiza que hay suficiente luz para operar)
unsigned const long llaveTime = 10000; // en ms // tiempo maximo antes de dar error por atasco de la llave

#if debug
  unsigned const long riegoTime = 10000; // en ms (10 seg) 10000 // tiempo maximo de llenado de deposito (en modo debug)
#else
  unsigned const long riegoTime = 2400000; // en ms (40 min) 2400000 // // tiempo maximo de llenado de deposito
#endif

#if debug
  unsigned int printTime = 500; // en ms tiempo entre actualizaciones de info
#endif
/////////////////////////////////////////////SETTINGS

/////////////////////////////////////////////DECLARACION

const int led = 13;
const int bomba = 7;
const int llaveIz = 6;
const int llaveDer = 5;

const int boya = 2;
const int microADer = 3;
const int microCDer = 8;
const int microAIz = 9;
const int microCIz = 10;
const int progDer = 11;
const int progIz = 12;

const int voltPin = A0;
const int voltRelay = 4;

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
  byte state = 0; // muestra el estado de operación actual
  char lado = 'X';
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
  digitalWrite(bomba,0);
  digitalWrite(llaveDer,0);
  digitalWrite(llaveIz,0);
  digitalWrite(voltRelay,0);
  
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
  #if debug
    Serial.println("Consiguiendo voltaje...");
    state = 1; // iniciando / voltaje
    delay(750);
  #endif
  while (volts <= vstart)
  {
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
    state = 2; // lleno / en espera
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
    #if debug
      lado = 'D';
    #endif
    delay(5000);
    regar('D');
  }
  if (eProgIz == 1)
  {
    #if debug
      lado = 'I';
    #endif
    delay(5000);
    regar('I');
  }
}

/////////////////////////////////////////////LOOP

/////////////////////////////////////////////FUNCTIONS

void error()
{
  #if debug
    Serial.println(F("ERROR\n\n"));
    delay(250);
    state = 127;
    com();
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
      Serial.println(F("ERROR\n\n"));
      delay(250);
      com();
      delay(2500);
    #endif
    return;
  }
  #if debug
    state = 5; // regando
  #endif
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
  bool lleno = 0;
  while (tiempoAhora-tiempoAntes <= riegoTime && !lleno)
  {
    tiempoAhora = millis();
    lleno = mantenerCuboLleno();
  }
  #if debug
    Serial.println(F("Fin de llenado"));
    state = 2; // lleno / en espera
    lado = 'X';
    delay(750);
  #endif
  digitalWrite(bomba,0); // apago bomba
}

void voltsVital()
{
  int sensorValue = analogRead(voltPin);          // realizar la lectura
  volts = fmap(sensorValue, 0, 1023, 0.0, 25.0);   // cambiar escala a 0.0 - 25.0
  if (volts >= vmax)
  {
    digitalWrite(voltRelay,1); // abrir // rele normanmente abierto
  }
  else if (volts <= vmin)
  {
    digitalWrite(voltRelay,0); // cerrar // rele normanmente abierto
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
    Serial.print(F("Voltaje:                    "));
    Serial.println(volts);
    Serial.println();
    Serial.print(F("Estado boya:                "));
    Serial.println(eBoya);
    Serial.println();
    Serial.print(F("Estado Micro Abierto Der:   "));
    Serial.println(eMicroADer);
    Serial.println();
    Serial.print(F("Estado Micro Cerrado Der:   "));
    Serial.println(eMicroCDer);
    Serial.println();
    Serial.print(F("Estado Micro Abierto Iz:    "));
    Serial.println(eMicroAIz);
    Serial.println();
    Serial.print(F("Estado Micro Cerrado Iz:    "));
    Serial.println(eMicroCIz);
    Serial.println();
    Serial.print(F("Estado Programador Der:     "));
    Serial.println(eProgDer);
    Serial.println();
    Serial.print(F("Estado Programador Iz:      "));
    Serial.println(eProgIz);
    Serial.println();
    Serial.print(F("Modo de operacion actual:   "));
    printstate();
    Serial.print(F("Programador desencadenante: "));
    Serial.println(lado);
    Serial.println("\n\n\n");
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

bool llave(char id, bool status) // resoult: 0 ==> fail; 1 ==> ok //state: 0 ==> cerrar; 1 ==> abrir
{
  #if debug
    state = 3; // consiguiendo voltaje para mover llave
  #endif
  vital();
  while (volts <= vllave)
  {
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
  if (status == 0)
  {
    #if debug
      state = 6; // cerrando llave
    #endif
    if (eMicroC == 0)
    {
      #if debug
        Serial.println(F("Cerrando llave..."));
        Serial.flush();
      #endif
      digitalWrite(llave,1); // encender
      while (eMicroC == 0 && resoult == 1)
      {
        current = millis();
        elapsed = current-before;
        if (elapsed >= llaveTime)
        {
          resoult = 0;
        }
        llaveVital(microA,microC,&eMicroA,&eMicroC);
      }
      digitalWrite(llave,0); // apagar
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
    #if debug
      state = 4; // abriendo llave
    #endif
    if (eMicroA == 0)
    {
      #if debug
        Serial.println(F("Abriendo llave..."));
        Serial.flush();
      #endif
      digitalWrite(llave,1); // encender
      while (eMicroA == 0 && resoult == 1)
      {
        current = millis();
        elapsed = current-before;
        if (elapsed >= llaveTime)
        {
          resoult = 0;
        }
        llaveVital(microA,microC,&eMicroA,&eMicroC);
      }
      digitalWrite(llave,0); // apagar
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

bool mantenerCuboLleno()
{
  vital();
  if (volts >= vllenarmax && llenarst == 0)
  {
    llenarst = 1;
    #if debug
      state = 7; // llenando cubo
    #endif
  }
  if (volts <= vllenarmin && llenarst == 1)
  {
    llenarst = 0;
    #if debug
      state = 8; // getting vllenarmin
    #endif
  }

  if (llenarst == 1)
  {
    if (eBoya == 1) // se ha llenado
    {
      digitalWrite(bomba,0); // apagado
      #if debug
        Serial.println(F("LLeno"));
        delay(1500);
      #endif
      return 1;
    }
    else
    {
      digitalWrite(bomba,1); // encendido
    }
  }
  else
  {
    digitalWrite(bomba,0); // apagado
  }
  return 0;
}

#if debug
  void printstate()
  {
    switch(state)
    {
      case 0:
        Serial.println(F("Iniciando"));
      break;
      case 1:
        Serial.println(F("Consiguiendo vmin"));
      break;
      case 2:
        Serial.println(F("En espera / Lleno"));
      break;
      case 3:
        Serial.println(F("Consiguiendo vllave"));
      break;
      case 4:
        Serial.println(F("Abriendo llave"));
      break;
      case 5:
        Serial.println(F("Regando"));
      break;
      case 6:
        Serial.println(F("Cerrando llave"));
      break;
      case 7:
        Serial.println(F("Llenando deposito"));
      break;
      case 8:
        Serial.println(F("Esperando riegoTime"));
      break;
      default:
        Serial.println(F("ERROR"));
    }
  }
#endif

/////////////////////////////////////////////FUNCTIONS
