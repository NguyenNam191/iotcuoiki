#ifndef GLOBALS_H
#define GLOBALS_H

#include <Arduino.h>
#include <ESP32Servo.h>
#include <LiquidCrystal_I2C.h>
#include <MFRC522.h>
#include <Wire.h>
#include "DHT.h"

// ======================================================
// PIN DEFINITIONS
// ======================================================
#define TRIG1 5
#define ECHO1 18

#define TRIG2 4
#define ECHO2 19

#define SERVO_DOOR_PIN 14
#define SERVO_GATE_PIN 27

#define BUZZER 32

#define SS_PIN 2
#define RST_PIN 15
#define MOSI_PIN 26
#define MISO_PIN 33
#define SCK_PIN 25

#define WATER_PIN 34
#define DHTPIN 23
#define DHTTYPE DHT22

#define LDR_PIN 35
#define LED_GATE 12
#define LED_PARK 16
#define LED_INDOOR 17

#define FIRE1_PIN 36
#define FIRE2_PIN 39

// ======================================================
// CONSTANTS
// ======================================================
extern const byte correctUID[4];

extern const int GATE_OPEN_DIST_CM;
extern const unsigned long GATE_OPEN_MS;

extern const int PARK_TARGET_CM;
extern const int PARK_TOLERANCE_CM;

extern const int LDR_DARK_THRESHOLD;

extern const unsigned long RFID_DOOR_OPEN_MS;
extern const unsigned long RFID_BAD_ALARM_MS;
extern const int RFID_MAX_BAD;

extern const int FIRE_THRESHOLD;
extern const unsigned long CLEAR_DEBOUNCE_MS;

extern const unsigned long ENV_SEND_INTERVAL;
extern const unsigned long SENSOR_PRINT_INTERVAL;
extern const unsigned long LCD_UPDATE_MS;

extern const int RAIN_THRESHOLD;

// ======================================================
// GLOBAL OBJECTS
// ======================================================
extern Servo servoDoor;
extern Servo servoGate;

extern LiquidCrystal_I2C lcd;
extern MFRC522 rfid;
extern DHT dht;

// ======================================================
// GLOBAL STATES
// ======================================================
extern bool gateOpen;
extern unsigned long gateOpenedAt;

extern bool doorOpen;
extern unsigned long doorOpenedAt;

extern int badRFIDCount;
extern bool badRFIDAlarm;
extern unsigned long badRFIDAlarmStarted;

extern unsigned long lastBlinkToggle;
extern bool blinkState;

extern bool fireAlarm;
extern unsigned long fireClearCandidateSince;
extern bool fireClearCandidate;

extern bool parkingBeepDone;

extern unsigned long lastEnvSent;
extern unsigned long lastSensorPrint;
extern unsigned long lastLCDUpdate;

// LCD MODE
enum LCDMode { LCD_ENV, LCD_MESSAGE };
extern LCDMode lcdMode;
extern unsigned long lcdMessageUntil;

// ======================================================
// MQTT LED notify helper (định nghĩa trong main.cpp)
// ======================================================
void publishLedState(const char* name, int pin);

#endif
