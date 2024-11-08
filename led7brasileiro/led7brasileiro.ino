// Definição dos pinos
const int pins[] = {18, 25, 4, 22, 23, 19, 21, 13, 27};
enum {a, b, c, d, e, f, g, p, z};

// Funções para exibir os números e o ponto
void displayDigit(bool aVal, bool bVal, bool cVal, bool dVal, bool eVal, bool fVal, bool gVal) {
  digitalWrite(pins[a], aVal);
  digitalWrite(pins[b], bVal);
  digitalWrite(pins[c], cVal);
  digitalWrite(pins[d], dVal);
  digitalWrite(pins[e], eVal);
  digitalWrite(pins[f], fVal);
  digitalWrite(pins[g], gVal);
}

void displayPoint(bool pState) {
  digitalWrite(pins[p], pState);
}

void beepBuzzer() {
  digitalWrite(pins[z], HIGH); // Ativa o buzzer
  delay(100);                   // Duração do "bip"
  digitalWrite(pins[z], LOW);   // Desativa o buzzer
}

void setup() {
  // Configuração dos pinos
  for (int i = 0; i < 9; i++) {
    pinMode(pins[i], OUTPUT);
  }
}

void loop() {
  // Array de números para exibir
  const bool digits[10][7] = {
    {LOW, LOW, LOW, LOW, LOW, LOW, HIGH},  // 0
    {HIGH, LOW, LOW, HIGH, HIGH, HIGH, HIGH},  // 1
    {LOW, LOW, HIGH, LOW, LOW, HIGH, LOW},  // 2
    {LOW, LOW, LOW, LOW, HIGH, HIGH, LOW},  // 3
    {HIGH, LOW, LOW, HIGH, HIGH, LOW, LOW},  // 4
    {LOW, HIGH, LOW, LOW, HIGH, LOW, LOW},  // 5
    {LOW, HIGH, LOW, LOW, LOW, LOW, LOW},  // 6
    {LOW, LOW, LOW, HIGH, HIGH, HIGH, HIGH},  // 7
    {LOW, LOW, LOW, LOW, LOW, LOW, LOW},  // 8
    {LOW, LOW, LOW, LOW, HIGH, LOW, LOW}   // 9
  };

  // Loop para exibir números de 0 a 9 com ponto e 'bip' do buzzer
  for (int i = 0; i < 10; i++) {
    displayDigit(digits[i][0], digits[i][1], digits[i][2], digits[i][3], digits[i][4], digits[i][5], digits[i][6]);
    displayPoint(LOW);  // Exibe o ponto
    beepBuzzer();       // Emite o 'bip'
    delay(500);
  }
}
