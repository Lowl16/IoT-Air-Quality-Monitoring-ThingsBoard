// Nama: Sandhi Karunia Sugihartana
// NIM: 2107411007
// Kelas: TI-6A
// Mata Kuliah: Internet of Things
// Praktik Pilihan 'C'

#include <WiFi.h>
#include <ThingsBoard.h>
#include <PubSubClient.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Keypad.h>
#include <DHT.h>
#include <Ticker.h>

// WiFi credentials
char ssid[] = "POCO M3 Pro 5G";
char pass[] = "61616161";

// ThingsBoard credentials
#define THINGSBOARD_SERVER "thingsboard.cloud"
#define TOKEN ""

// DHT11
#define DHTPIN 19
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// MQ2
#define MQ2PIN 34

// LCD I2C
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Keypad
const byte ROWS = 4;
const byte COLS = 4;
char keys[ROWS][COLS] = {
  { '1', '2', '3', 'A' },
  { '4', '5', '6', 'B' },
  { '7', '8', '9', 'C' },
  { '*', '0', '#', 'D' }
};
byte rowPins[ROWS] = { 15, 2, 0, 4 };
byte colPins[COLS] = { 16, 17, 5, 18 };
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

// Variables
int gas;
int unit = 10;
float humidity;
String gasInformation;
String airInformation;

// Initialize ThingsBoard client
WiFiClient espClient;
ThingsBoard tb(espClient);

// Initialize Ticker
Ticker ticker;

void setup() {
  // Initialize serial monitor
  Serial.begin(115200);

  // Initialize DHT sensor
  dht.begin();

  // Initialize LCD
  lcd.init();
  lcd.backlight();

  // Initialize WiFi
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("WiFi connected");

  // Display initial message
  lcd.setCursor(0, 0);
  lcd.print("Initializing...");
  delay(2000);
  lcd.clear();
  lcd.setCursor(0, 1);
  lcd.print("Press A/B");

  ticker.attach(1, interval);
}

void loop() {
  // Reconnect to ThingsBoard if needed
  if (!tb.connected()) {
    // Connect to the ThingsBoard server
    if (!tb.connect(THINGSBOARD_SERVER, TOKEN)) {
      Serial.println("Failed to connect to ThingsBoard");
      return;
    }
  }

  // Read sensor data
  humidity = dht.readHumidity();

  if (isnan(humidity)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  gas = analogRead(MQ2PIN);

  // Check keypad input
  char key = keypad.getKey();
  if (key) {
    switch (key) {
      case 'A': unit = 0; break;
      case 'B': unit = 1; break;
    }

    // Update the display immediately after a key press
    interval();
  }
}

void interval() {
  if (gas >= 2600) {
    gasInformation = "Detected";
  } else {
    gasInformation = "Not Detected";
  }

  if (humidity > 80 || gas >= 2600) {
    airInformation = "Bad";
  } else {
    airInformation = "Good";
  }

  // Update LCD
  lcd.setCursor(0, 0);
  switch (unit) {
    case 0:
      lcd.clear();
      lcd.print("Gas:");
      if (gas >= 2600) {
        lcd.print("Detected");
      } else {
        lcd.print("Not Detected");
      }
      break;
    case 1:
      lcd.clear();
      lcd.print("Humidity:");
      lcd.print(humidity);
      lcd.print("%");
      break;
  }

  lcd.setCursor(0, 1);
  lcd.print("Air Quality:");
  if (humidity > 80 || gas >= 2600) {
    lcd.print("Bad");
  } else {
    lcd.print("Good");
  }

  // Update ThingsBoard
  tb.sendTelemetryFloat("humidity", humidity);
  tb.sendTelemetryString("gasInformation", gasInformation.c_str());
  tb.sendTelemetryString("airInformation", airInformation.c_str());
}