#include "DistanceModule.h"
#include "Globals.h"
#include "UtilModule.h"
#include "LcdModule.h"
#include "BuzzerModule.h"

void handleDistances(unsigned long now) {
    long dGate = readDistance(TRIG2, ECHO2);
    long dPark = readDistance(TRIG1, ECHO1);

    // Gate control
    if (!gateOpen && dGate < GATE_OPEN_DIST_CM) {
        servoGate.write(0);
        gateOpen = true;
        gateOpenedAt = now;
        lcdShowMessage("Xe toi cong", "Mo cong", 1500);
    }

    if (gateOpen && now - gateOpenedAt >= GATE_OPEN_MS) {
        servoGate.write(90);
        gateOpen = false;
    }

    // Parking LED + beep
    bool shouldOn = (dPark != 999 && abs((int)dPark - PARK_TARGET_CM) <= PARK_TOLERANCE_CM);
    int oldState = digitalRead(LED_PARK);
    int newState = shouldOn ? HIGH : LOW;

    if (newState == HIGH) {
        if (oldState != HIGH) {
            digitalWrite(LED_PARK, HIGH);
            publishLedState("park", LED_PARK);
        } else {
            digitalWrite(LED_PARK, HIGH);
        }

        if (!parkingBeepDone) {
            toneBeep(1500, 120);
            parkingBeepDone = true;
            lcdShowMessage("Xe da do", "Dung vi tri", 1500);
        }
    } else {
        if (oldState != LOW) {
            digitalWrite(LED_PARK, LOW);
            publishLedState("park", LED_PARK);
        } else {
            digitalWrite(LED_PARK, LOW);
        }
        parkingBeepDone = false;
    }
}

