#include "BuzzerModule.h"
#include "Globals.h"

void toneBeep(int freq, unsigned long ms) {
    tone(BUZZER, freq);
    delay(ms);
    noTone(BUZZER);
}

void playHappyMelody() {
    int notes[] = { 600, 750, 900, 750, 600 };
    int duration[] = { 120, 120, 120, 120, 200 };

    for (int i = 0; i < 5; i++) {
        tone(BUZZER, notes[i]);
        delay(duration[i]);
    }
    noTone(BUZZER);
}
