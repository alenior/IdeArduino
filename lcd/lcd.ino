#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// Defina o endereço I2C do seu LCD. Normalmente é 0x3F ou 0x27, mas pode variar.
LiquidCrystal_I2C lcd(0x3F, 16, 2);  // Endereço I2C, largura do display (16 colunas) e altura (2 linhas)

void setup() {
  // Inicializa a comunicação com o LCD
  lcd.begin(16, 2);  
  lcd.backlight();  // Liga o backlight do display
  
  // Exibe a mensagem "Flamengo Campeão!" na primeira linha
  lcd.setCursor(0, 0);  // Posiciona o cursor na primeira linha e primeira coluna
  lcd.print("Flamengo Campeao!");
}

void loop() {
  // Não é necessário fazer nada no loop para manter a mensagem exibida
}
