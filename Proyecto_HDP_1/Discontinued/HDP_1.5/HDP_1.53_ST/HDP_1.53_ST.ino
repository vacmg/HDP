//LIBRARIES

//LIBRARIES

//SETTINGS

const bool serial = 1; // usar para comunicar con pc
bool off = 1; // apagado por software

const float vmax[2] = {15.5,15.5}; // voltaje al que deja de cargar
const float vload[2] = {15,15}; // voltaje al que empieza a cargar
const float vmin[2] = {12,12}; // voltaje al que deja de funcionar el sistema
unsigned const long subirartime = 40000; // tiempo de subir al cubo grande // en ms (seg * 1000)
unsigned const long boyaAbTime = 30000; // tiempo de subir a cubo pequeño antes de parar para conseguir vmax // en ms (seg * 1000)
unsigned const long pausa1 = 10000; // tiempo de espera desde que se llena cupe hasta que sube arriba // en ms (seg * 1000)
unsigned const long pausa2 = 10000; // tiempo de espera desde que sube arriba hasta que llena cupe // en ms (seg * 1000)
unsigned const long cupetime = 50000; // tiempo maximo que sube a cupe antes de hacer pausacupe // en ms (seg * 1000)
unsigned const long pausacupe = 30000; // tiempo que espera si cupe no se llena en cupetime ms // en ms (seg * 1000)

unsigned const int led = 13;
unsigned const long looptime = 500; // tiempo entre lectura de mediciones // si repiquetean los reles, aumentar valor // en ms (seg * 1000)
unsigned const long refreshtime = 500; // cada cuanto refrescar las mediciones de debug // en ms (seg * 1000)

//SETTINGS

//I/O

unsigned const int vRele[2] = {12, 7};
unsigned const int servo = 5;
unsigned const int bombaAr = 6;
unsigned const int bombaAb = 4;

unsigned const int boyaSeg = 2;
unsigned const int boyaCuPe = 3;
unsigned const int boyaAr = 8;
unsigned const int boyaAb = 9; // va invertida
unsigned const int microa = 10;
unsigned const int microc = 11;

unsigned const int voltPin[2] = {A0, A1};
//I/O

//VARIABLES

bool eboyaSeg;
bool eboyaCuPe;
bool eboyaAr;
bool eboyaAb;
bool emicroa;
bool emicroc;
float volt[2];

unsigned long prevcupe, prevpausacupe, prevpausa1, prevpausa2;
bool subircupe = 1, subirar = 0;
bool enablepausa1 = 0, enablepausa2 = 0, enablepausacupe = 0, enablecupe = 0;
bool sw = 1, cupesw = 1;

bool voltsw = 1;

bool trigger = 0;
bool trig = 0;
bool ger = 0;

//VARIABLES

//SETUP

void setup()
{
  if (serial == 1)
  {
    Serial.begin(115200);
    delay(500);
    Serial.println("Booting...");
  }

  apagado();
  digitalWrite(vRele[0],0); // vrele[0] va invertido // apagado
  digitalWrite(vRele[1],1); // apagado
  
  pinMode(vRele[0],OUTPUT);
  pinMode(vRele[1],OUTPUT);
  pinMode(servo,OUTPUT);
  pinMode(bombaAr,OUTPUT);
  pinMode(bombaAb,OUTPUT);
  pinMode(boyaSeg,INPUT);
  pinMode(boyaCuPe,INPUT);
  pinMode(boyaAr,INPUT);
  pinMode(boyaAb,INPUT);
  pinMode(microa,INPUT);
  pinMode(microc,INPUT);

  digitalWrite(vRele[0],0); // vrele[0] va invertido // apagado
  digitalWrite(vRele[1],1); // apagado
  apagado();
  vital();
  const bool on[2] = {1,0};
  for (int i=0;i<2;i++)
  {
    digitalWrite(vRele[i],on[i]);
    while (volt[i] < vmax[i])
    {
      leer();
      if (serial == 1)
      {
        Serial.print("Getting ");
        Serial.print(vmax[i]);
        Serial.println("V...");
        Serial.print("Current voltage of zone ");
        Serial.print(i);
        Serial.print(": ");
        Serial.print(volt[i]);
        Serial.println("V\n");
        delay(1000);
      }
    }
    digitalWrite(vRele[i],!on[i]);
  }
    
  regar(0);
  if (serial == 1)
  {
    Serial.println("Ready");
  }
}

//SETUP

//LOOP

void loop()
{
  vital();
  if (off) trigger = 0;
  
  if (trigger == 0)
  {
    digitalWrite(led,0);
    apagado();
    if (serial == 1)
    {
      Serial.println("OFF");
      Serial.println("");
    }
  }
  else
  {
    digitalWrite(led,1);
    if (serial == 1)
    {
      Serial.println("ON");
      Serial.println("");
    }
    if (eboyaCuPe == 0 && subircupe)
    {
      if (serial)
      {
        Serial.println("\n\nEstoy subiendo agua a cupe\n");
      }
      if (cupesw)
      {
        prevcupe = millis();
        enablecupe = 1;
        cupesw = 0;
      }
      sw = 1;
      if (eboyaSeg == 1)
      {
        bombear(0);
      }
      else
      {
        if (serial == 1)
        {
          Serial.println("SIN AGUA\n");
        }
        apagado();
      }
    }
    else if (sw)
    {
      if (serial)
      {
        Serial.println("\n\nEstoy en pausa 'cupe --> arr'\n");
      }
      subircupe = 0;
      sw = 0;
      enablepausa1 = 1;
      enablecupe = 0;
      prevpausa1 = millis();
      vital();
      delay(2000);
    }
    if (eboyaCuPe == 1 && eboyaAr == 0)
    {
      apagado();
      if (subirar)
      {
        bombear(1);
        if (serial == 1)
        {
            Serial.println("bombear a cubo grande");
            Serial.println();
        }
        unsigned long prevbombearar = millis();
        bool bombearar = 1;
        while (bombearar && eboyaAr == 0 && trigger == 1)
        {
          vital();
          if (prevbombearar+subirartime <= millis())
          {
            bombearar = 0;
          }
        }
        apagado();
        subirar = 0;
        prevpausa2 = millis();
        enablepausa2 = 1;
      }
    }
    if (eboyaAr == 1)
    {
      if (serial == 1)
      {
        Serial.println("regar");
        Serial.println("");
      }
      regar(1);
      while (eboyaAb == 1){vital();}
      regar(0);
    }
  }
  delay(looptime);
}

//LOOP

//FUCTIONS

void printData()
{
  Serial.print("Boya de seguridad: ");
  Serial.println(eboyaSeg);
  Serial.print("Boya del cubo pequeno: ");
  Serial.println(eboyaCuPe);
  Serial.print("Boya de arriba: ");
  Serial.println(eboyaAr);
  Serial.print("Boya de abajo: ");
  Serial.println(eboyaAb);
  Serial.print("Micro abierto: ");
  Serial.println(emicroa);
  Serial.print("Micro cerrado: ");
  Serial.println(emicroc);
  
  Serial.print("Pausa 'cupe --> arr': ");
  Serial.println(millis()-prevpausa1);
  Serial.print("Pausa 'arr --> cupe': ");
  Serial.println(millis()-prevpausa2);
  Serial.print("Cupe time: ");
  Serial.println(millis()-prevcupe);
  Serial.print("Pausa cupe: ");
  Serial.println(millis()-prevpausacupe);

  Serial.print("Subircupe: ");
  Serial.println(subircupe);
  Serial.print("Subirar: ");
  Serial.println(subirar);
  Serial.print("Enablepausa 'cupe --> arr': ");
  Serial.println(enablepausa1);
  Serial.print("Enablepausa 'arr --> cupe': ");
  Serial.println(enablepausa2);
  Serial.print("Enablepausacupe: ");
  Serial.println(enablepausacupe);
  Serial.print("Enablecupe: ");
  Serial.println(enablecupe);
  Serial.print("Sw: ");
  Serial.println(sw);
  Serial.print("Cupesw: ");
  Serial.println(cupesw);
  Serial.print("Trigger: ");
  Serial.println(trigger);
  Serial.print("Trig: ");
  Serial.println(trig);
  Serial.print("Ger: ");
  Serial.println(ger);    

  for (int i= 0; i<2;i++)
  {
    Serial.print("Voltaje de ");
    Serial.print(i);
    Serial.print(": ");
    Serial.print(volt[i]);
    Serial.println(" V");
  }
  
  Serial.println();
  delay(refreshtime);
}

void resetSwitches()
{
  subircupe = 1;
  subirar = 0;
  enablepausa1 = 0;
  enablepausa2 = 0;
  enablepausacupe = 0;
  enablecupe = 0;
  sw = 1;
  cupesw = 1;
}

void vital()
{
  leer();
  voltControl();
  timingControl();
}

void timingControl()
{
  if (prevpausa1+pausa1 <= millis() && enablepausa1)
  {
    subirar = 1;
    enablepausa1 = 0;
    if (serial)
    {
      printData();
      Serial.println("END PAUSA CUPE-->ARR");
      delay(2000);
    }
  }
  if (prevpausa2+pausa2 <= millis() && enablepausa2)
  {
    subircupe = 1;
    cupesw = 1;
    enablepausa2 = 0;
    if (serial)
    {
      printData();
      Serial.println("END PAUSA ARR-->CUPE");
      delay(2000);
    }
  }
  if (prevcupe+cupetime <= millis() && enablecupe)
  {
    subircupe = 0;
    sw = 0;
    prevpausacupe = millis();
    enablepausacupe = 1;
    enablecupe = 0;
    apagado();
    if (serial)
    {
      printData();
      Serial.println("Cupe STOP");
      delay(2000);
    }
  }
  if (prevpausacupe+pausacupe <=millis() && enablepausacupe)
  {
    subircupe = 1;
    cupesw = 1;
    enablepausacupe = 0;
    if (serial)
    {
      printData();
      Serial.println("Cupe PAUSE END");
      delay(2000);
    }
  }
}

void voltControl()
{
    if (volt[0] <= vload[0])
    {
      digitalWrite(vRele[0],1); // carga
    }
    if (volt[0] >= vmax[0])
    {
      digitalWrite(vRele[0],0); // no carga
    }
    if (volt[1] <= vload[1])
    {
      digitalWrite(vRele[1],0); // carga
    }
    if (volt[1] >= vmax[1])
    {
      digitalWrite(vRele[1],1); // no carga
    }
}

float floatmap(float x, float in_min, float in_max, float out_min, float out_max)
{
   return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

void leer()
{
  eboyaSeg = digitalRead(boyaSeg);
  eboyaCuPe = digitalRead(boyaCuPe);
  eboyaAr = digitalRead(boyaAr);
  eboyaAb = !digitalRead(boyaAb); // ! // INVERTIDO
  emicroa = digitalRead(microa);
  emicroc = digitalRead(microc);
    
  for (int i = 0;i<2;i++)
  {
    volt[i] = floatmap(analogRead(voltPin[i]), 0, 1023, 0.0, 25.0);
//volt[0] = 20;
//volt[1] = 15.5;
    if (volt[i] >= vmax[i])
    {
      if (i == 0){trig = 1;}
      else {ger = 1;}
    }
    if ((volt[i] < vmin[i]))
    {
      if (i == 0){trig = 0;}
      else {ger = 0;}
    }
  }

  if (trig && ger && eboyaSeg)
  {
    trigger = 1;
    if (voltsw)
    {
      if (serial)
      {
        Serial.println("Reseting switches due to the start ON MODE");
      }
      resetSwitches();
      voltsw = 0;
    }
  }
  else
  {
    trigger = 0;
    if (!voltsw)
    {
      voltsw = 1;
    }
  }
  
  if (serial)
  {
    printData();
  }
}

void bombear(bool y)
{
  if (y == 0)
  {
    digitalWrite(bombaAb,1); // enciende
    digitalWrite(bombaAr,0); // apaga
  }
  else
  {
    digitalWrite(bombaAb,0); // apaga
    digitalWrite(bombaAr,1); // enciende
  }
}

void apagado()
{
  digitalWrite(bombaAb,0); // apaga
  digitalWrite(bombaAr,0); // apaga
  digitalWrite(servo,0); // apaga
}

void regar(bool x)
{
  vital();
  apagado();
  digitalWrite(vRele[0],1); // fuerzo la carga para evitar zona muerta de voltaje (ni llega a vmax ni baja a vmin)
  while (volt[0] <= vmax[0]){vital();if (serial == 1){Serial.println("PREPARANDO...");delay(1000);}}
  if (x == 1)
  {
    if (emicroa == 1)
    {
      if (serial == 1)
      {
        Serial.println("abierto");
        Serial.println("");
      } 
    }
    else
    {
      digitalWrite(servo,1); //on
      while (emicroa == 0)
      {
        vital();
        if (serial == 1)
        {
          Serial.println("abriendo");
          Serial.println("");
        }
      }
      digitalWrite(servo,0); //off
      if (serial == 1)
      {
        Serial.println("abierto");
        Serial.println("");
      }
    }
  }
  else
  {
    if (emicroc == 1)
    {
      if (serial == 1)
      {
        Serial.println("cerrado");
        Serial.println("");
      }
    }
    else
    {
      digitalWrite(servo,1); //on
      while (emicroc == 0)
      {
        vital();
        if (serial == 1)
        {
          Serial.println("cerrando");
          Serial.println("");
        }
      }
      digitalWrite(servo,0); //off
      if (serial == 1)
      {
        Serial.println("cerrado");
        Serial.println("");
      }
    }
  }
}

//FUNCTIONS
