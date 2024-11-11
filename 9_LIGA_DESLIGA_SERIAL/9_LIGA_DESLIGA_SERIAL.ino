//---Mapeamento de Hardware---//
#define led 2

//---Declaração de variável---// 
char acionaLed=0;

//---Função para configurações iniciais do programa---//
void setup() 
{
Serial.begin(9600);
pinMode(led,OUTPUT);
}

//---Loop Infinito---//
void loop() 
{
  
  if (Serial.available() > 0)
  {
  acionaLed=Serial.read();
  }
    
    if(acionaLed=='l')
    {
      digitalWrite(led,HIGH);
      Serial.print(acionaLed);
      Serial.println(" LED on ");
    }
      
      if(acionaLed=='d')
      {
        digitalWrite(led,LOW);
        Serial.print(acionaLed);
        Serial.println(" LED off");
      }
 
}
