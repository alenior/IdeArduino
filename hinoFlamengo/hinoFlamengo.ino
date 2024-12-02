#include "pitches.h"

// Notas da melodia do Hino do Flamengo com base nos acordes fornecidos
int melody[] = {
  NOTE_D4, NOTE_B3, NOTE_G3, NOTE_GS3, NOTE_D4, NOTE_A4, NOTE_D4,  // Intro
  NOTE_E4, NOTE_A4, // Uma vez Flamengo
  NOTE_D4,  // Sempre Flamengo
  NOTE_G4, NOTE_D4, NOTE_FS4, // Flamengo sempre eu hei de ser
  NOTE_G4, NOTE_FS4, NOTE_A4, NOTE_B4, // É o meu maior prazer
  NOTE_FS4, NOTE_G4, NOTE_E4, NOTE_D4, NOTE_B3, // Vê-lo brilhar
  NOTE_D4, NOTE_A4, NOTE_D4, NOTE_B3, NOTE_B4, NOTE_A4, NOTE_B4, NOTE_A4, // Seja na terra, seja no mar
  NOTE_E4, NOTE_FS4, NOTE_D4, NOTE_A4, NOTE_B4, NOTE_D4, NOTE_B3, // Vencer, vencer, vencer
  NOTE_A4, NOTE_D4, NOTE_B3, NOTE_E4, NOTE_D4, NOTE_FS4, // Uma vez Flamengo
  NOTE_A4, NOTE_D4, NOTE_A4, NOTE_D4, NOTE_A4, NOTE_D4, NOTE_B3, // Flamengo até morrer
  NOTE_D4, NOTE_A4, NOTE_B3, NOTE_A4, NOTE_D4, NOTE_E4, NOTE_FS4, NOTE_A4, // Na regata ele me mata
  NOTE_D4, NOTE_B3, NOTE_G4, NOTE_FS4, NOTE_A4, NOTE_D4, NOTE_B3, // Me maltrata, me arrebata
  NOTE_E4, NOTE_D4, NOTE_E4, NOTE_B4, NOTE_D4, NOTE_A4, // Que emoção no coração
  NOTE_B4, NOTE_D4, NOTE_B3, NOTE_E4, NOTE_A4, NOTE_B3, NOTE_A4, NOTE_D4, // Consagrado no gramado
  NOTE_E4, NOTE_A4, NOTE_A7, NOTE_A4, NOTE_D4, NOTE_E4, NOTE_D4, NOTE_FS4, // Sempre amado, o mais cotado
  NOTE_A4, NOTE_B4, NOTE_A4, NOTE_A4, NOTE_B3, NOTE_D4, NOTE_E4, NOTE_A4, // Nos "Fla-Flus é o Ai, Jesus
  NOTE_D4, NOTE_E4, NOTE_B3, NOTE_D4, NOTE_FS4, NOTE_D4, NOTE_B3, NOTE_D4, // Eu teria um desgosto profundo
  NOTE_A4, NOTE_E4, NOTE_A4, NOTE_D4, NOTE_FS4, NOTE_D4, NOTE_E4 // Se faltasse o Flamengo no mundo
};

int noteDurations[] = {
  4, 4, 4, 4, 4, 4, 4,  // Intro
  4, 4, // Uma vez Flamengo
  4,  // Sempre Flamengo
  4, 4, 4, 4, // Flamengo sempre eu hei de ser
  4, 4, 4, 4, // É o meu maior prazer
  4, 4, 4, 4, // Vê-lo brilhar
  4, 4, 4, 4, 4, // Seja na terra
  4, 4, 4, 4, 4,  // Seja no mar
  4, 4, 4, 4, 4,  // Vencer, vencer, vencer
  4, 4, 4, 4, 4, // Uma vez Flamengo
  4, 4, 4, 4, // Flamengo até morrer
  4, 4, 4, 4, 4, 4, // Na regata ele me mata
  4, 4, 4, 4, 4, // Me maltrata, me arrebata
  4, 4, 4, 4, // Que emoção no coração
  4, 4, 4, 4, 4, // Consagrado no gramado
  4, 4, 4, 4, // Sempre amado, o mais cotado
  4, 4, 4, 4, // Nos "Fla-Flus é o Ai, Jesus
  4, 4, 4, 4, // Eu teria um desgosto profundo
  4, 4, 4, 4 // Se faltasse o Flamengo no mundo
};

void setup() {
  // Iterar sobre as notas da melodia
  for (int thisNote = 0; thisNote < 72; thisNote++) {

    // Calcular a duração da nota
    int noteDuration = 1000 / noteDurations[thisNote];
    tone(25, melody[thisNote], noteDuration);

    // Intervalo entre as notas (ajustado para 30% de pausa)
    int pauseBetweenNotes = noteDuration * 1.30;
    delay(pauseBetweenNotes);

    // Parar o som após cada nota
    noTone(25);
  }
}

void loop() {
  // Sem necessidade de repetir a melodia
}
