#include "UtilModule.h"
#include "Globals.h"

long readDistance(int trig, int echo) {
    digitalWrite(trig, LOW);
    delayMicroseconds(3);
    digitalWrite(trig, HIGH);
    delayMicroseconds(10);
    digitalWrite(trig, LOW);

    long duration = pulseIn(echo, HIGH, 30000);
    long distance = duration * 0.034 / 2;
    if (duration == 0 || distance > 400) return 999;
    return distance;
}

void sendEnvToSerial(float temp, float hum, int water) {
    Serial.printf("{\"temp\":%.1f,\"hum\":%.1f,\"water\":%d}\n", temp, hum, water);
}

void debugPrintSensors(long dGate, long dPark, int ldrVal, int water, int fire1, int fire2, unsigned long now) {
    Serial.println("===== SENSOR VALUES =====");
    Serial.printf("GateDist: %ld\n", dGate);
    Serial.printf("ParkDist: %ld\n", dPark);
    Serial.printf("LDR: %d\n", ldrVal);
    Serial.printf("Water: %d\n", water);
    Serial.printf("Fire1: %d\n", fire1);
    Serial.printf("Fire2: %d\n", fire2);
    Serial.println("=========================");
}
