#include "FireModule.h"
#include "Globals.h"
#include "LcdModule.h"

void handleFire(unsigned long now, long dPark) {
    int fire1 = analogRead(FIRE1_PIN);
    int fire2 = analogRead(FIRE2_PIN);

    if (!fireAlarm) {
        if (fire1 < FIRE_THRESHOLD || fire2 < FIRE_THRESHOLD) {
            fireAlarm = true;
            tone(BUZZER, 2000);
            lcdShowMessage("CHAY!!!", "EVACUATE!", 0);
            fireClearCandidate = false;
        }
        return;
    } else {
        // Blink 3 LEDs
        if ((now/200)%2==0) {
            digitalWrite(LED_GATE, HIGH);
            digitalWrite(LED_INDOOR, HIGH);
            digitalWrite(LED_PARK, HIGH);
        } else {
            digitalWrite(LED_GATE, LOW);
            digitalWrite(LED_INDOOR, LOW);
            digitalWrite(LED_PARK, LOW);
        }

        if (fire1 >= FIRE_THRESHOLD && fire2 >= FIRE_THRESHOLD) {
            if (!fireClearCandidate) {
                fireClearCandidate = true;
                fireClearCandidateSince = now;
            } else {
                if (now - fireClearCandidateSince >= CLEAR_DEBOUNCE_MS) {
                    fireAlarm = false;
                    fireClearCandidate = false;
                    noTone(BUZZER);

                    int ldrValAfter = analogRead(LDR_PIN);
                    bool isDarkAfter = (ldrValAfter >= LDR_DARK_THRESHOLD);

                    // restore LEDs theo logic cũ
                    bool oldIndoor = digitalRead(LED_INDOOR);
                    bool oldPark   = digitalRead(LED_PARK);

                    digitalWrite(LED_INDOOR, doorOpen ? HIGH : LOW);
                    digitalWrite(LED_PARK, (dPark!=999 && abs((int)dPark-PARK_TARGET_CM)<=PARK_TOLERANCE_CM)?HIGH:LOW);
                    // LED_GATE restore handled centrally via LedModule.applyGateLedState

                    // publish trạng thái thực sau cháy
                    publishLedState("indoor", LED_INDOOR);
                    publishLedState("park",   LED_PARK);
                    // Gate sẽ được publish trong applyGateLedState (được gọi ở loop)

                    lcdShowMessage("Fire cleared","",1500);
                }
            }
        } else {
            fireClearCandidate = false;
        }
    }
}