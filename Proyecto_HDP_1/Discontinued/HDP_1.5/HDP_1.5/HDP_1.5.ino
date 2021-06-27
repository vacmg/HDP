//LIBRARIES

//LIBRARIES

//SETTINGS

const bool serial = 0; // usar para comunicar con pc
const float vmax[2] = {17,13.5}; // voltaje al que deja de cargar
const float vload[2] = {15.5,12}; // voltaje al que empieza a cargar
const float vmin[2] = {8,8}; // voltaje al que deja de funcionar el sistema
const int ciclos = 301; // tiempo de subir al cubo grande (seg) * 10 + 1

unsigned const int led = 13;

//SETTINGS

//I/O

unsigned const int vRele[2] = {12, 7};
unsigned const int servo = 5;
unsigned const int bombaAr = 6;
unsigned const int bombaAb = 4;

unsigned const int boyaSeg = 2;
unsigned const int boyaCuPe = 3;
unsigned const int boyaAr = 8;
unsigned const int boyaAb = 9;
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

bool trigger = 0;

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
    if (eboyaCuPe == 0)
    {
      if (eboyaSeg == 1)
      {
        bombear(0);
        if (serial == 1)
        {
          Serial.println("bombear a cubo pequeno");
          Serial.println("");
        }
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
    if (eboyaCuPe == 1 && eboyaAr == 0)
    {
      bombear(1);
      if (serial == 1)
      {
          Serial.println("bombear a cubo grande");
          Serial.println("");
      }
      for (int i=0;i<ciclos;i++)
      {
        vital();
        if (eboyaAr == 1)
        {
          i = i + 301;
        }
        if (serial == 1)
        {
          Serial.println(i);
        }
        delay(100);
      }
      apagado();
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
}

//LOOP

//FUCTIONS

void vital()
{
  leer();
  voltControl();
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
  bool trig;
  bool ger;
  eboyaSeg = digitalRead(boyaSeg);
  eboyaCuPe = digitalRead(boyaCuPe);
  eboyaAr = digitalRead(boyaAr);
  eboyaAb = digitalRead(boyaAb);
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
    if ((volt[i] < vmin[i]) || (eboyaSeg == 0))
    {
      if (i == 0){trig = 0;}
      else {ger = 0;}
    }
  }

  if (!trig || !ger)
  {
    trigger = 0;
  }
  else if (trig && ger)
  {
    trigger = 1;
  }
  
  if (serial == 1)
  {
    Serial.print("Boya de seguridad: ");
    Serial.println(eboyaSeg);
    delay(200);
    Serial.print("Boya del cubo pequeno: ");
    Serial.println(eboyaCuPe);
    delay(200);
    Serial.print("Boya de arriba: ");
    Serial.println(eboyaAr);
    delay(200);
    Serial.print("Boya de abajo: ");
    Serial.println(eboyaAb);
    delay(200);
    Serial.print("Micro abierto: ");
    Serial.println(emicroa);
    delay(200);
    Serial.print("Micro cerrado: ");
    Serial.println(emicroc);
    delay(200);

    for (int i= 0; i<2;i++)
    {
      Serial.print("Voltaje de ");
      Serial.print(i);
      Serial.print(": ");
      Serial.print(volt[i]);
      Serial.println(" V");
      delay(200);
    }
    
    Serial.println("");
    delay(200);
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
