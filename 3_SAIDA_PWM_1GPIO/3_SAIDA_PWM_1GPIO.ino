//---Mapeamento de Hardware---//
#define led 15

//---Variáveis Globais---//
int frequencia = 5000;
int canal      =    0;
int resolucao  =    8;

//---Função Principal---//
void setup()
{
  //Configuração para o funcionamento do PWM no LED
  ledcSetup(canal, frequencia, resolucao);
  
  //Associando a GPIO ao canal escolhido do PWM
  ledcAttachPin(led, canal);
}

//---Loop Infinito---//
void loop()
{
  //Incrementando o brilho do LED
  for(int i=0; i<= 255; i++)
  {   
    //Mudando a intensidade do brilho do LED com uso do PWM
    ledcWrite(canal, i);
    delay(15);
  }

  //Decrementando o brilho do LED
  for(int i=255; i>=0; i--)
  {
    //Mudando a intensidade do brilho do LED com uso do PWM
    ledcWrite(canal, i);   
    delay(15);
  }
}
