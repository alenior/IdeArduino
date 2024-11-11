//---Mapeamento de Hardware---//
#define touchPin  12 
#define ledPin    26
#define limite   20

//---Variáveis Globais---//
int  touchValue;

//---Função Principal---//
void setup()
{
  Serial.begin(115200);
  delay(1000);
  pinMode (ledPin, OUTPUT);
}

//---Loop Infinito---//
void loop()
{
  for(int i=0;i<100;i++)
  {
  touchValue += touchRead(touchPin); 
  }
  touchValue=touchValue/100;
  Serial.print(touchValue);
    if(touchValue < limite)
    {
    digitalWrite(ledPin, HIGH);
    Serial.println(" - LED Aceso");
    }
      else
      {
      digitalWrite(ledPin, LOW);
      Serial.println(" - LED Apagado");
      }
  delay(500);
}
