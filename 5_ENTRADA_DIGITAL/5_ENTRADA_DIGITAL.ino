//---Mapeamento de Hardware---//
#define btn 0
#define led 15

//---Função Principal---//
void setup() 
{
pinMode(btn,INPUT);
pinMode(led,OUTPUT);
digitalWrite(led,LOW);
}

//---Loop Infinito---//
void loop() 
{
  if(digitalRead(btn)==LOW)
  {
  digitalWrite(led,HIGH);  
  }
    else
    digitalWrite(led,LOW);
}
