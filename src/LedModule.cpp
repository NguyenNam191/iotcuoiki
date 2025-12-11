#include "LedModule.h"
#include "Globals.h"

void applyGateLedState(bool isDark) {
    if (fireAlarm) return;
    if (isDark || gateOpen) digitalWrite(LED_GATE, HIGH);
    else digitalWrite(LED_GATE, LOW);
}
