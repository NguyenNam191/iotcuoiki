#ifndef UTIL_MODULE_H
#define UTIL_MODULE_H

long readDistance(int trig, int echo);
void sendEnvToSerial(float temp, float hum, int water);
void debugPrintSensors(long dGate, long dPark, int ldrVal, int water, int fire1, int fire2, unsigned long now);

#endif
