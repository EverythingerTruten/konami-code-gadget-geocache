#include <Arduino.h>
#include "sounds.h"
// Definicja nut

const int NOTE_A4 = 440;
const int NOTE_B4 = 494;
const int NOTE_C4 = 262;
const int NOTE_D4 = 294;
const int NOTE_F5 = 698;
const int NOTE_E5 = 659;
const int NOTE_D5 = 587;
const int NOTE_C5 = 523;
const int NOTE_B5 = 987;
const int NOTE_E6 = 1319;
const int NOTE_Fs6 = 1480;
const int NOTE_B6 = 1976;
const int NOTE_Gs5 = 831;
const int NOTE_As5 = 932;
const int REST = 0;   // cisza

//Kontrola tempa, pozwala na zmianę BPM
int standardBPM = 240;
int helldiversBPM = 220;
int gradiusBPM = 500;

int quarter(int bpm){
    return 60000 / bpm;
}

int half(int bpm){
    return 120000 / bpm;
}

//Funkcja helper

void playTone(int frequency, int duration, int BuzzerPin) {
    if (frequency == REST) {
        delay(duration); //Cisza przez długość nuty
    }   else {
        tone(BuzzerPin, frequency, duration);
        delay(duration * 1.3); // odstęp między nutami
    }
}



//Funkcje audio

void keySuccess(int BuzzerPin){
    playTone(NOTE_B4, quarter(standardBPM), BuzzerPin);
}

void keyFailure(int BuzzerPin){
    playTone(NOTE_D4, half(standardBPM), BuzzerPin);
    playTone(NOTE_C4, half(standardBPM), BuzzerPin);
}

void konamiCodeSuccess(int BuzzerPin){
    playTone(NOTE_B5, quarter(gradiusBPM), BuzzerPin);
    playTone(NOTE_E6, quarter(gradiusBPM), BuzzerPin);
    playTone(NOTE_Fs6, quarter(gradiusBPM), BuzzerPin);
    playTone(REST, quarter(gradiusBPM), BuzzerPin);
    playTone(NOTE_B6, quarter(gradiusBPM), BuzzerPin);
    playTone(REST, quarter(gradiusBPM), BuzzerPin);
    playTone(NOTE_Fs6, quarter(gradiusBPM), BuzzerPin);
    playTone(REST, quarter(gradiusBPM), BuzzerPin);
    playTone(NOTE_E6, quarter(gradiusBPM), BuzzerPin);
    playTone(REST, quarter(gradiusBPM), BuzzerPin);
    playTone(NOTE_B5, quarter(gradiusBPM), BuzzerPin);
    playTone(NOTE_E6, quarter(gradiusBPM), BuzzerPin);
    playTone(NOTE_B5, quarter(gradiusBPM), BuzzerPin);
    playTone(NOTE_E6, quarter(gradiusBPM), BuzzerPin);
    playTone(NOTE_Fs6, quarter(gradiusBPM), BuzzerPin);
    playTone(NOTE_Fs6, quarter(gradiusBPM), BuzzerPin);
    playTone(NOTE_Gs5, quarter(gradiusBPM), BuzzerPin);
    playTone(REST, quarter(gradiusBPM), BuzzerPin);
    playTone(NOTE_As5, quarter(gradiusBPM), BuzzerPin);
    playTone(REST, quarter(gradiusBPM), BuzzerPin);
    playTone(NOTE_B5, quarter(gradiusBPM), BuzzerPin);
};

void easterEggSuccess(int BuzzerPin){
    playTone(NOTE_F5, quarter(helldiversBPM), BuzzerPin);
    playTone(NOTE_E5, quarter(helldiversBPM), BuzzerPin);
    playTone(NOTE_D5, quarter(helldiversBPM), BuzzerPin);
    playTone(NOTE_A4, half(helldiversBPM) * 1.75, BuzzerPin);
    playTone(NOTE_C5, half(helldiversBPM), BuzzerPin);
    playTone(NOTE_D5, half(helldiversBPM) * 1.5, BuzzerPin);
};