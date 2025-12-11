// =========================
// main.cpp - Full with MQTT integration (PubSubClient TLS)
// Cleaned & fixed version (matches your project structure)
// =========================

#include <SPI.h>
#include <Arduino.h>
#include <ESP32Servo.h>
#include <LiquidCrystal_I2C.h>
#include <MFRC522.h>
#include <Wire.h>
#include "DHT.h"

// MQTT & WiFi
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>

// Secrets & CA
#include "secrets/wifi.h"     // WiFiSecrets::ssid, ::pass
#include "secrets/mqtt.h"     // EMQX::broker, ::port, ::username, ::password
#include "ca_cert_emqx.h"     // const char* ca_cert
#include "wifi_connect.h"

// Modules and globals
#include "Globals.h"
#include "UtilModule.h"
#include "BuzzerModule.h"
#include "LcdModule.h"
#include "EnvironmentModule.h"
#include "DistanceModule.h"
#include "FireModule.h"
#include "RfidModule.h"
#include "LedModule.h"

// =========================
// MQTT / WiFi globals
// =========================
static WiFiClientSecure tlsClient;
static PubSubClient mqttClient(tlsClient);

// Topics (as you provided)
static const char* temperature_topic = "esp32/sensors/temperature";
static const char* humidity_topic    = "esp32/sensors/humidity";
static const char* water_topic       = "esp32/sensors/water";
static const char* fire_topic        = "esp32/status/fire";

static const char* cmd_door_topic    = "esp32/cmd/door";
static const char* cmd_gate_topic    = "esp32/cmd/gate";
static const char* cmd_led_gate      = "esp32/cmd/led/gate";
static const char* cmd_led_park      = "esp32/cmd/led/park";
static const char* cmd_led_indoor    = "esp32/cmd/led/indoor";

static const char* status_door_topic = "esp32/status/door";
static const char* status_gate_topic = "esp32/status/gate";
static const char* status_led_base   = "esp32/status/led/";

// reconnect timing
unsigned long lastMqttReconnectAttempt = 0;
const unsigned long MQTT_RECONNECT_INTERVAL_MS = 3000;

// =========================
// Helper: publish helper
// =========================
static void publishStringTopic(const char* topic, const String &payload) {
    if (mqttClient.connected()) {
        mqttClient.publish(topic, payload.c_str(), false);
    }
}

// publishLedState is declared in Globals.h as extern; define it here (non-static)
void publishLedState(const char* name, int pin) {
    String topic = String(status_led_base) + name;
    String payload = digitalRead(pin) ? "on" : "off";
    publishStringTopic(topic.c_str(), payload);
}

// Forward declarations
void mqttReconnect();
void mqttCallback(char* topic, byte* payload, unsigned int length);

// =========================
// MQTT callback (incoming commands)
// =========================
void mqttCallback(char* topic, byte* payload, unsigned int length) {
    String msg;
    msg.reserve(length + 1);
    for (unsigned int i = 0; i < length; ++i) msg += (char)payload[i];

    Serial.print("[MQTT] recv topic=");
    Serial.print(topic);
    Serial.print(" msg=");
    Serial.println(msg);

    String t(topic);

    // Door control
    if (t == cmd_door_topic) {
        if (msg.equalsIgnoreCase("open") || msg.equalsIgnoreCase("1") || msg.equalsIgnoreCase("open\r") ) {
            servoDoor.write(0);
            doorOpen = true;
            doorOpenedAt = millis();

            // turn indoor LED ON (this is allowed as part of door open behavior)
            digitalWrite(LED_INDOOR, HIGH);
            publishStringTopic(status_door_topic, "open");
            publishLedState("indoor", LED_INDOOR);
        } else if (msg.equalsIgnoreCase("close") || msg.equalsIgnoreCase("0") || msg.equalsIgnoreCase("close\r")) {
            servoDoor.write(90);
            doorOpen = false;
            digitalWrite(LED_INDOOR, LOW);
            publishStringTopic(status_door_topic, "close");
            publishLedState("indoor", LED_INDOOR);
        }
    }

    // Gate control
    else if (t == cmd_gate_topic) {
        if (msg.equalsIgnoreCase("open") || msg.equalsIgnoreCase("1")) {
            servoGate.write(0);
            gateOpen = true;
            gateOpenedAt = millis();
            publishStringTopic(status_gate_topic, "open");
        } else if (msg.equalsIgnoreCase("close") || msg.equalsIgnoreCase("0")) {
            servoGate.write(90);
            gateOpen = false;
            publishStringTopic(status_gate_topic, "close");
        }
    }

    // LED controls (explicitly from Node-RED)
    else if (t == cmd_led_gate) {
        if (msg.equalsIgnoreCase("on") || msg.equalsIgnoreCase("1")) digitalWrite(LED_GATE, HIGH);
        else digitalWrite(LED_GATE, LOW);
        publishLedState("gate", LED_GATE);
    }

    else if (t == cmd_led_park) {
        if (msg.equalsIgnoreCase("on") || msg.equalsIgnoreCase("1")) digitalWrite(LED_PARK, HIGH);
        else digitalWrite(LED_PARK, LOW);
        publishLedState("park", LED_PARK);
    }

    else if (t == cmd_led_indoor) {
        if (msg.equalsIgnoreCase("on") || msg.equalsIgnoreCase("1")) digitalWrite(LED_INDOOR, HIGH);
        else digitalWrite(LED_INDOOR, LOW);
        publishLedState("indoor", LED_INDOOR);
    }
}

// =========================
// MQTT reconnect
// =========================
void mqttReconnect() {
    if (mqttClient.connected()) return;

    unsigned long now = millis();
    if (now - lastMqttReconnectAttempt < MQTT_RECONNECT_INTERVAL_MS) return;
    lastMqttReconnectAttempt = now;

    Serial.print("[MQTT] Attempting connection... ");
    String clientId = "esp32-" + WiFi.macAddress();
    if (mqttClient.connect(clientId.c_str(), EMQX::username, EMQX::password)) {
        Serial.println("connected");

        mqttClient.subscribe(cmd_door_topic);
        mqttClient.subscribe(cmd_gate_topic);
        mqttClient.subscribe(cmd_led_gate);
        mqttClient.subscribe(cmd_led_park);
        mqttClient.subscribe(cmd_led_indoor);

        // publish initial status
        publishStringTopic(status_door_topic, doorOpen ? "open" : "close");
        publishStringTopic(status_gate_topic, gateOpen ? "open" : "close");
        publishLedState("gate",   LED_GATE);
        publishLedState("park",   LED_PARK);
        publishLedState("indoor", LED_INDOOR);

    } else {
        Serial.print("failed, rc=");
        Serial.println(mqttClient.state());
    }
}

// =========================
// SETUP
// =========================
void setup() {
    Serial.begin(115200);

    pinMode(TRIG1, OUTPUT); pinMode(ECHO1, INPUT);
    pinMode(TRIG2, OUTPUT); pinMode(ECHO2, INPUT);
    pinMode(BUZZER, OUTPUT);
    pinMode(LED_GATE, OUTPUT);
    pinMode(LED_PARK, OUTPUT);
    pinMode(LED_INDOOR, OUTPUT);

    // default LED off
    digitalWrite(LED_GATE, LOW);
    digitalWrite(LED_PARK, LOW);
    digitalWrite(LED_INDOOR, LOW);

    servoDoor.attach(SERVO_DOOR_PIN);
    servoGate.attach(SERVO_GATE_PIN);
    servoDoor.write(90);
    servoGate.write(90);

    Wire.begin(21, 22);
    lcd.init();
    lcd.backlight();

    SPI.begin(SCK_PIN, MISO_PIN, MOSI_PIN);
    pinMode(SS_PIN, OUTPUT);
    digitalWrite(SS_PIN, HIGH);
    rfid.PCD_Init();
    delay(50);
    Serial.println("RFID READY");

    dht.begin();

    lcdShowMessage("System Ready", "", 1200);
    delay(900);
    lcdMode = LCD_ENV;
    lcd.clear();

    // WiFi
    Serial.println("[WIFI] connecting...");
    WiFi.begin(WiFiSecrets::ssid, WiFiSecrets::pass);
    while (WiFi.status() != WL_CONNECTED) {
        delay(250);
        Serial.print(".");
    }
    Serial.println();
    Serial.print("[WIFI] connected. IP=");
    Serial.println(WiFi.localIP());

    // MQTT
    tlsClient.setCACert(ca_cert);
    mqttClient.setServer(EMQX::broker, EMQX::port);
    mqttClient.setCallback(mqttCallback);
}

// =========================
// MAIN LOOP
// =========================
void loop() {
    unsigned long now = millis();

    long dGate = readDistance(TRIG2, ECHO2);
    long dPark = readDistance(TRIG1, ECHO1);
    int ldrVal = analogRead(LDR_PIN);
    int water = analogRead(WATER_PIN);

    bool isRaining = water > RAIN_THRESHOLD;
    bool isDark    = ldrVal >= LDR_DARK_THRESHOLD;

    handleRFID(now);
    handleRFIDAlarmBlink(now);
    handleDistances(now);
    handleFire(now, dPark);
    handleEnv(now, isRaining);
    handleLCD(now, isRaining, water);

    if (now - lastSensorPrint >= SENSOR_PRINT_INTERVAL) {
        lastSensorPrint = now;
        debugPrintSensors(dGate, dPark, ldrVal, water,
                          analogRead(FIRE1_PIN), analogRead(FIRE2_PIN), now);
    }

    // applyGateLedState may be part of LedModule — keep it if you still want auto gate-led behaviour.
    applyGateLedState(isDark);

    if (!mqttClient.connected()) {
        mqttReconnect();
    } else {
        mqttClient.loop();
    }

    if (now - lastEnvSent >= ENV_SEND_INTERVAL) {
        float hum = dht.readHumidity();
        float temp = dht.readTemperature();
        int waterVal = analogRead(WATER_PIN);

        if (!isnan(hum) && !isnan(temp)) {
            char buf[64];
            snprintf(buf, sizeof(buf), "%.1f", temp);
            publishStringTopic(temperature_topic, String(buf));
            snprintf(buf, sizeof(buf), "%.1f", hum);
            publishStringTopic(humidity_topic, String(buf));
            snprintf(buf, sizeof(buf), "%d", waterVal);
            publishStringTopic(water_topic, String(buf));
        } else {
            char buf[16];
            snprintf(buf, sizeof(buf), "%d", waterVal);
            publishStringTopic(water_topic, String(buf));
        }

        int f1 = analogRead(FIRE1_PIN);
        int f2 = analogRead(FIRE2_PIN);
        bool fireNow = (f1 < FIRE_THRESHOLD || f2 < FIRE_THRESHOLD);
        publishStringTopic(fire_topic, fireNow ? "1" : "0");

        lastEnvSent = now;
    }

    // If door was opened earlier in the code (by RFID or MQTT), auto-turn-off after configured ms
    if (doorOpen && (now - doorOpenedAt >= RFID_DOOR_OPEN_MS)) {
        servoDoor.write(90);
        doorOpen = false;
        digitalWrite(LED_INDOOR, LOW);
        publishLedState("indoor", LED_INDOOR);
    }

    delay(5);
}









