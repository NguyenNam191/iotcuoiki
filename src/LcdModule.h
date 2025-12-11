#ifndef LCD_MODULE_H
#define LCD_MODULE_H

void initLcd();
void lcdShowMessage(const char* l1, const char* l2, unsigned long ms);
void handleLCD(unsigned long now, bool isRaining, int water);

#endif
