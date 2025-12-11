#include "Globals.h"

// ======================================================
// CONSTANT DEFINITIONS
// ======================================================
const byte correctUID[4] = {0x57, 0x0C, 0x73, 0x06};

const int GATE_OPEN_DIST_CM = 4;
const unsigned long GATE_OPEN_MS = 3000;

const int PARK_TARGET_CM = 3;
const int PARK_TOLERANCE_CM = 1;

const int LDR_DARK_THRESHOLD = 1500;

const unsigned long RFID_DOOR_OPEN_MS = 3000;
const unsigned long RFID_BAD_ALARM_MS = 10000;
const int RFID_MAX_BAD = 5;

const int FIRE_THRESHOLD = 2000;
const unsigned long CLEAR_DEBOUNCE_MS = 2000;

const unsigned long ENV_SEND_INTERVAL = 3000;
const unsigned long SENSOR_PRINT_INTERVAL = 1000;
const unsigned long LCD_UPDATE_MS = 2000;

const int RAIN_THRESHOLD = 2000;

// ======================================================
// OBJECTS
// ======================================================
Servo servoDoor;
Servo servoGate;

LiquidCrystal_I2C lcd(0x27, 16, 2);
MFRC522 rfid(SS_PIN, RST_PIN);
DHT dht(DHTPIN, DHTTYPE);

// ======================================================
// RUNTIME VARIABLES
// ======================================================
bool gateOpen = false;
unsigned long gateOpenedAt = 0;

bool doorOpen = false;
unsigned long doorOpenedAt = 0;

int badRFIDCount = 0;
bool badRFIDAlarm = false;
unsigned long badRFIDAlarmStarted = 0;

unsigned long lastBlinkToggle = 0;
bool blinkState = false;

bool fireAlarm = false;
unsigned long fireClearCandidateSince = 0;
bool fireClearCandidate = false;

bool parkingBeepDone = false;

unsigned long lastEnvSent = 0;
unsigned long lastSensorPrint = 0;
unsigned long lastLCDUpdate = 0;

LCDMode lcdMode = LCD_ENV;
unsigned long lcdMessageUntil = 0;
