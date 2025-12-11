#include "EnvironmentModule.h"
#include "Globals.h"
#include "UtilModule.h"

void handleEnv(unsigned long now, bool isRaining) {
    static float cachedHum = NAN;
    static float cachedTemp = NAN;

    if (now - lastEnvSent >= ENV_SEND_INTERVAL) {
        lastEnvSent = now;
        cachedHum = dht.readHumidity();
        cachedTemp = dht.readTemperature();
        int water = analogRead(WATER_PIN);
        if (!isnan(cachedHum) && !isnan(cachedTemp)) {
            sendEnvToSerial(cachedTemp, cachedHum, water);
        }
    }
}