const int ledPin1 = 26;
const int ledPin2 = 32;
const int ledPin3 = 33;

const int freq = 5000;
const int canal = 0;
const int resolution = 10;

//---Função Principal---//
void setup()
{
  ledcAttach(ledPin1, freq, resolution);
  ledcAttach(ledPin2, freq, resolution);
  ledcAttach(ledPin3, freq, resolution);
}

//---Loop Infinito---//
void loop()
{
  //Incrementado o brilho dos leds
  for(int i= 0; i<= 1024; i++)
  {   
    //Mudando o brilho dos leds com PWM
    ledcWrite(ledPin1, i);
    ledcWrite(ledPin2, i);
    ledcWrite(ledPin3, i);
    delay(5);
  }

  //Decrementado o brilho dos leds
  for(int i= 1024; i>= 0; i--)
  {
    //Mudando o brilho dos leds com PWM
    ledcWrite(ledPin1, i);
    ledcWrite(ledPin2, i);
    ledcWrite(ledPin3, i);  
    delay(5);
  }
}
