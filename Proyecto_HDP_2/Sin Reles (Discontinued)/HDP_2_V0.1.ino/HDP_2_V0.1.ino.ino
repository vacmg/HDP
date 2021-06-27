//LIBRARIES

//LIBRARIES

//DEBUG

const bool serial = 0;

unsigned const int led = 13;

//DEBUG

//I/O

unsigned const int vRele = 8;
unsigned const int servo = 9;
unsigned const int bombaAr = 10;
unsigned const int bombaAb = 11;

unsigned const int boyaSeg = 2;
unsigned const int boyaCuPe = 3;
unsigned const int boyaAr = 4;
unsigned const int boyaAb = 5;
unsigned const int microa = 6;
unsigned const int microc = 7;

unsigned const int voltPin = A0;

//I/O

//VARIABLES

bool eboyaSeg = 2;
bool eboyaCuPe = 3;
bool eboyaAr = 4;
bool eboyaAb = 5;
bool emicroa = 6;
bool emicroc = 7;
float volt;

bool trigger;

//VARIABLES

//SETUP

void setup()
{
  if (serial == 1)
  {
    Serial.begin(9600);
    delay(500);
    Serial.println("Booting...");
  }
  
  pinMode(vRele,OUTPUT);
  pinMode(servo,OUTPUT);
  pinMode(bombaAr,OUTPUT);
  pinMode(bombaAb,OUTPUT);

  pinMode(boyaSeg,INPUT);
  pinMode(boyaCuPe,INPUT);
  pinMode(boyaAr,INPUT);
  pinMode(boyaAb,INPUT);
  pinMode(microa,INPUT);
  pinMode(microc,INPUT);

  apagado();

  vital();
  while (volt < 13.5)
  {
    vital();
    if (serial == 1)
  {
    Serial.println("Getting 12V...");
    Serial.println("");
    delay(1000);
  }
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
      bombear(0);
      if (serial == 1)
      {
        Serial.println("bombear a cubo pequeno");
        Serial.println("");
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
      for (int i=0;i<301;i++)
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
      while (eboyaAb == 0){vital();}
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
  if (volt <= 13.5)
  {
    digitalWrite(vRele,1); // carga
  }
  if (volt >= 14)
  {
    digitalWrite(vRele,0); // no carga
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
  eboyaAb = digitalRead(boyaAb);
  emicroa = digitalRead(microa);
  emicroc = digitalRead(microc);
  volt = floatmap(analogRead(voltPin), 0, 1023, 0.0, 25.0);
  
  if (volt <= 11 || eboyaSeg == 0)
  {
    trigger = 0;
  }
  if (volt >= 13.5)
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
    Serial.print("Voltaje: ");
    Serial.print(volt);
    Serial.println(" V");
    delay(200);
    Serial.println("");
    delay(200);
  }
}

void bombear(bool y)
{
  if (y == 0)
  {
    digitalWrite(bombaAb,0); // enciende
    digitalWrite(bombaAr,1); // apaga
  }
  else
  {
    digitalWrite(bombaAb,1); // apaga
    digitalWrite(bombaAr,0); // enciende
  }
}

void apagado()
{
  digitalWrite(bombaAb,1); // apaga
  digitalWrite(bombaAr,1); // apaga
  digitalWrite(servo,1); // apaga
}

void regar(bool x)
{
  vital();
  apagado();
  while (volt <= 13.5){vital();if (serial == 1){Serial.println("PREPARANDO...");delay(1000);}}
  if (x == 1)
  {
    if (emicroa == 0){if (serial == 1)
  {
    Serial.println("abierto");
    Serial.println("");
  }}
    else
    {
      digitalWrite(servo,0); //on
      while (emicroa == 1){vital();if (serial == 1)
  {
    Serial.println("abriendo");
    Serial.println("");
  }}
      digitalWrite(servo,1); //off
      if (serial == 1)
  {
    Serial.println("abierto");
    Serial.println("");
  }
    }
  }
  else
  {
    if (emicroc == 0){if (serial == 1)
  {
    Serial.println("cerrado");
    Serial.println("");
  }}
    else
    {
      digitalWrite(servo,0); //on
      while (emicroc == 1){vital();if (serial == 1)
  {
    Serial.println("cerrando");
    Serial.println("");
  }}
      digitalWrite(servo,1); //off
      if (serial == 1)
  {
    Serial.println("cerrado");
    Serial.println("");
  }
    }
  }
}

//FUNCTIONS
