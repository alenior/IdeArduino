const int interruptPin1 = 2;
const int interruptPin2 = 3;
const int horario = 7;
const int antihorario = 8;
const int pwm = 9;
volatile int speed = 0;
volatile int turn = 0;


void setup() {
  pinMode(horario,OUTPUT);
  pinMode(antihorario, OUTPUT);
  pinMode(pwm, OUTPUT);
  attachInterrupt(digitalPinToInterrupt(interruptPin1), rotacao, FALLING);
  attachInterrupt(digitalPinToInterrupt(interruptPin2), velocidade, FALLING);
  analogWrite(9,255);
  digitalWrite(horario, HIGH);
  digitalWrite(antihorario, LOW);
}

void loop() {
  if (turn == 0){
    digitalWrite(horario, HIGH);
    digitalWrite(antihorario, LOW);
  }
  else{
    digitalWrite(horario, LOW);
    digitalWrite(antihorario, HIGH);
  }
  if (speed == 0){
    analogWrite(9, 255);
  
  }
  else{
    analogWrite(9, 127);
  }  
}


//Tratamento de interrupção da via 1. Dependendo da flag pode multar ou contar.
void rotacao(){
  digitalWrite(horario, LOW);
  digitalWrite(antihorario, LOW);
  delay(5000);
  turn = !turn;  
}

//Tratamento de interrupção da via 2. Dependendo da flag pode multar ou contar.
void velocidade(){
  speed = !speed;
}