#ifndef SOUNDS_H_
#define SOUNDS_H

void playTone(int frequency, int duration, int BuzzerPin);

void keySuccess(int BuzzerPin);
void keyFailure(int BuzzerPin);
void konamiCodeSuccess(int BuzzerPin);
void easterEggSuccess(int BuzzerPin);

#endif