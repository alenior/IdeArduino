//---Mapeamento de Hardware---//
#include <HardwareSerial.h>
#define ledS1 2
#define btnS1 0
#define ledS2 15
#define btnS2 36

#define RX1_PIN 4  // Novo pino RX para UART1
#define TX1_PIN 2  // Novo pino TX para UART1

//---Declaração de variáveis---//
char dadoS1=0,dadoS2;
bool flagBS1=0, flagBS2=0, flagLS1=0, flagLS2=0;

//---Protótipo das funções auxiliares---//
void configuracao();
void leSerial();
void acionaLed();
void leBotoes();

//---Função Principal---//
void  setup () 
{ 
configuracao();  
}

//---Loop Infinito---//
void  loop () 
{
  leSerial(); 
  acionaLed();
  leBotoes();     
}

//---Funções Auxiliares---//
void configuracao()
{
  Serial.begin  ( 9600 );
  Serial1.begin ( 9600 );
  Serial2.begin ( 9600 );
  pinMode(ledS1,OUTPUT);
  pinMode(ledS2,OUTPUT);
  pinMode(btnS1,INPUT);
  pinMode(btnS2,INPUT);
  digitalWrite(ledS1,LOW); 
  digitalWrite(ledS2,LOW); 
}
//===========================================//
void leSerial()
{
 if(Serial1.available ()) 
 {
 dadoS1=Serial1.read();
 } 

     if(Serial2.available ()) 
     {
     dadoS2=Serial2.read();
     }
}
//===========================================//
void acionaLed()
{
  if (dadoS1=='l') 
  {
  digitalWrite(ledS1,HIGH);
  }
  else
  if (dadoS1=='d') 
  {
  digitalWrite(ledS1,LOW);
  } 

    if (dadoS2=='l') 
    {
    digitalWrite(ledS2,HIGH);
    }
    else
    if (dadoS2=='d') 
    {
    digitalWrite(ledS2,LOW);
    } 
}
//===========================================//
void leBotoes()
{
  if((digitalRead(btnS1)==LOW)&&(flagBS1==0))
  {
  flagBS1=1;
  flagLS1=!flagLS1;
    if(flagLS1==0)
    {
    Serial1.write('l');
    Serial.println("Serial 1 aciona LED da GPIO27");
    }
    if(flagLS1==1)
    {
    Serial1.write('d');
    Serial.println("Serial 1 desaciona LED da GPIO27");
    }
  }

  if((digitalRead(btnS1)==HIGH)&&(flagBS1==1))
  {
  flagBS1=0;
  } 

      if((digitalRead(btnS2)==LOW)&&(flagBS2==0))
      {
      flagBS2=1;
      flagLS2=!flagLS2;
        if(flagLS2==0)
        {
        Serial2.write('l');
        Serial.println("Serial 2 aciona LED da GPIO12");
        }
        if(flagLS2==1)
        {
        Serial2.write('d');
        Serial.println("Serial 2 desaciona LED da GPIO12");
        }
      }
    
      if((digitalRead(btnS2)==HIGH)&&(flagBS2==1))
      {
      flagBS2=0;
      } 
}
