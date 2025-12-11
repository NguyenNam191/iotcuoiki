#include <SPI.h>
#include "RfidModule.h"
#include "Globals.h"
#include "BuzzerModule.h"
#include "LcdModule.h"

void handleRFID(unsigned long now) {
    if (rfid.PICC_IsNewCardPresent() && rfid.PICC_ReadCardSerial()) {

        bool valid = true;
        for (byte i = 0; i < 4; i++)
            if (rfid.uid.uidByte[i] != correctUID[i]) valid = false;

        if (valid) {
            badRFIDCount = 0;
            badRFIDAlarm = false;
            noTone(BUZZER);

            playHappyMelody();
            lcdShowMessage("Chao mung", "Ve nha!", 2000);

            servoDoor.write(0);
            doorOpen = true;
            doorOpenedAt = now;

            int oldIndoor = digitalRead(LED_INDOOR);
            digitalWrite(LED_INDOOR, HIGH);
            if (oldIndoor != HIGH) {
                publishLedState("indoor", LED_INDOOR);
            }
        }
        else {
            badRFIDCount++;
            toneBeep(900, 120);
            lcdShowMessage("Sai the!", "", 1200);

            if (badRFIDCount >= RFID_MAX_BAD && !badRFIDAlarm) {
                badRFIDAlarm = true;
                badRFIDAlarmStarted = now;

                tone(BUZZER, 1000);
                lcdShowMessage("CO TROM!!!", "RFID FAILED", 0);
            }
        }

        rfid.PICC_HaltA();
        rfid.PCD_StopCrypto1();
    }

    if (doorOpen && now - doorOpenedAt >= RFID_DOOR_OPEN_MS) {
        servoDoor.write(90);
        doorOpen = false;
        int oldIndoor = digitalRead(LED_INDOOR);
        digitalWrite(LED_INDOOR, LOW);
        if (oldIndoor != LOW) {
            publishLedState("indoor", LED_INDOOR);
        }
    }

    if (badRFIDAlarm && now - badRFIDAlarmStarted >= RFID_BAD_ALARM_MS) {
        badRFIDAlarm = false;
        badRFIDCount = 0;
        noTone(BUZZER);
        lcdShowMessage("Reset RFID", "", 1500);
    }
}

void handleRFIDAlarmBlink(unsigned long now) {

    if (!badRFIDAlarm) {
        if (!doorOpen) {
            int oldIndoor = digitalRead(LED_INDOOR);
            if (oldIndoor != LOW) {
                digitalWrite(LED_INDOOR, LOW);
                publishLedState("indoor", LED_INDOOR);
            } else {
                digitalWrite(LED_INDOOR, LOW);
            }
        }
        return;
    }

    if (now - lastBlinkToggle >= 200) {
        lastBlinkToggle = now;
        blinkState = !blinkState;
        int newState = blinkState ? HIGH : LOW;
        int oldState = digitalRead(LED_INDOOR);
        digitalWrite(LED_INDOOR, newState);
        if (oldState != newState) {
            publishLedState("indoor", LED_INDOOR);
        }
    }
}


