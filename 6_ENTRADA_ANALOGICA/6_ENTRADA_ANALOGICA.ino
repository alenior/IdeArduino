void setup() 
{
pinMode(4,INPUT);
Serial.begin(9600);
}

void loop() 
{
int leituraADC=analogRead(4);
Serial.print("A leitura ADC eh ");
Serial.println(leituraADC);
delay(500);
} 
