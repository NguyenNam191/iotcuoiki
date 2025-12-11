#include "LcdModule.h"
#include "Globals.h"
#include "DHT.h"
#include "UtilModule.h"

void initLcd() {
    Wire.begin(21, 22);
    lcd.init();
    lcd.backlight();
}

void lcdShowMessage(const char* l1, const char* l2, unsigned long ms) {
    lcd.clear();
    lcd.setCursor(0,0); lcd.print(l1);
    lcd.setCursor(0,1); lcd.print(l2);
    lcdMode = LCD_MESSAGE;
    if (ms == 0) lcdMessageUntil = 0;
    else lcdMessageUntil = millis() + ms;
}

void handleLCD(unsigned long now, bool isRaining, int water) {
    if (lcdMode == LCD_MESSAGE) {
        if (lcdMessageUntil != 0 && now >= lcdMessageUntil) {
            lcdMode = LCD_ENV;
            lcd.clear();
        }
    }

    if (lcdMode == LCD_ENV && now - lastLCDUpdate >= LCD_UPDATE_MS) {
        lastLCDUpdate = now;
        float hum = dht.readHumidity();
        float temp = dht.readTemperature();

        lcd.clear();

        if (isRaining) {
            lcd.setCursor(0,0); lcd.print("Troi dang mua");
            lcd.setCursor(0,1); lcd.print("Water: "); lcd.print(water);
        }
        else if (!isnan(hum) && !isnan(temp)) {
            lcd.setCursor(0,0);
            lcd.print("Temp: ");
            lcd.print(temp,1);
            lcd.print((char)223);
            lcd.print("C");

            lcd.setCursor(0,1);
            lcd.print("Hum: ");
            lcd.print(hum,1);
            lcd.print(" %");
        } else {
            lcd.setCursor(0,0); lcd.print("DHT22 error");
            lcd.setCursor(0,1); lcd.print("Check sensor");
        }
    }
}
