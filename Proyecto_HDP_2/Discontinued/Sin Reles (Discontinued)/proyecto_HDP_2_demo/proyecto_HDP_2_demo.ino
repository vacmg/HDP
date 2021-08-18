//////////////////////////////////////////////////////INCLUDE/////////////////////////////////

//////////////////////////////////////////////////////DECLARE/////////////////////////////////
const int boya = 31;
const int bomba = 53;
const int llaveDer = 51;
const int microADer = 49;
const int microCDer = 47;
const int llaveIz = 45;
const int microAIz = 43;
const int microCIz = 41;
const int progDer = 52;
const int progIz = 50;
int estadoBoya;
int estadoMicroADer;
int estadoMicroCDer;
int estadoMicroAIz;
int estadoMicroCIz;
int estadoProgDer = 0;
int estadoProgIz;
const bool debug = true;
int c;
//////////////////////////////////////////////////////SETUP///////////////////////////////////

void setup() 
{
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
  c = 1;
  if (debug == 1)
  {
   Serial.begin(9600);
   Serial.print("Booting");
   delay(200);
   Serial.print(".");
   delay(200);
  }
   digitalWrite(bomba,1);
  if(debug == 1)
  {
   Serial.print(".");
   delay(200);
   Serial.print(".");
   delay(200);
  }
   digitalWrite(llaveDer,1);
  if(debug == 1)
  {
   Serial.print(".");
   delay(200);
   Serial.println(".");
   delay(200);
  }
   digitalWrite(llaveIz,1);
  if(debug == 1)
  {
   Serial.println(" ");
   delay(200);
  }
}

//////////////////////////////////////////////////////LOOP////////////////////////////////////
void loop() 
{
  leer();
  llenar();
//if (estadoProgDer == 1)
//{
  abrirDer();
  for( int i = 0;i<4;i++) //i<301
  {
    llenar();
    delay(1000);
  }
  cerrarDer();
  
//}
    
}

//////////////////////////////////////////////////////VOID////////////////////////////////////
void leer()
{
  estadoBoya = digitalRead(boya);
  estadoMicroADer = digitalRead(microADer);
  estadoMicroCDer = digitalRead(microCDer);
  estadoMicroAIz = digitalRead(microAIz);
  estadoMicroCIz = digitalRead(microCIz);
  estadoProgDer = digitalRead(progDer);
  estadoProgIz = digitalRead(progIz);
  if (debug == 1)
  {
    if (c == 1)
    {
      com();
    }
  }
}

void com()
{
  Serial.print("Estado boya:              ");
  Serial.println(estadoBoya);
  Serial.println(" ");
  delay(250);
  Serial.print("Estado Micro Abierto Der: ");
  Serial.println(estadoMicroADer);
  Serial.println(" ");
  delay(250);
  Serial.print("Estado Micro Cerrado Der: ");
  Serial.println(estadoMicroCDer);
  Serial.println(" ");
  delay(250);
  Serial.print("Estado Micro Abierto Iz:  ");
  Serial.println(estadoMicroAIz);
  Serial.println(" ");
  delay(250);
  Serial.print("Estado Micro Cerrado Iz:  ");
  Serial.println(estadoMicroCIz);
  Serial.println(" ");
  delay(250);
  Serial.print("Estado Programador Der:   ");
  Serial.println(estadoProgDer);
  Serial.println(" ");
  delay(250);
  Serial.print("Estado Programador Iz:    ");
  Serial.println(estadoProgIz);
  Serial.println(" ");
  delay(250);
  delay(75);
  Serial.println(" ");
  delay(75);
  Serial.println(" ");
  delay(75);
  Serial.println(" ");
  delay(75);
  Serial.println(" ");
  delay(75);
  Serial.println(" ");
}

void llenar()
{
  c = 0;
  leer();
  if(estadoBoya == 0)
  {
    digitalWrite(bomba,1);
    if(debug == 1)
    {
     Serial.println("LLENO");
     Serial.println(" ");
    }
  }
  else
  {
    digitalWrite(bomba,0);
    if(debug == 1)
    {
     Serial.println("LLENANDO...");
     Serial.println(" ");
    }
  }
  c = 1;
}

void abrirDer()
{
  digitalWrite(llaveDer,0);
  c = 0;
  leer();
  while(estadoMicroADer == 0)
  {
    leer();
    if(debug == 1)
    {
     Serial.println("Abriendo...");
    }
  }
  digitalWrite(llaveDer,1);
  if(debug == 1)
    {
     Serial.println("Abierto");
    }
  c = 1;
}

void cerrarDer()
{
  digitalWrite(llaveDer,0);
  c = 0;
  leer();
  while(estadoMicroCDer == 0)
  {
    leer();
    if(debug == 1)
    {
     Serial.println("Cerrando...");
    }
  }
  digitalWrite(llaveDer,1);
  if(debug == 1)
    {
     Serial.println("Cerrado");
    }
  c = 1;
}

//////////////////////////////////////////////////////END/////////////////////////////////////
