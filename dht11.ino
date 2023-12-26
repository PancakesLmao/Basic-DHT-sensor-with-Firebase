#include <Arduino.h>
#include "DHT.h"

#include <ESP8266WiFi.h>
#include <FirebaseESP8266.h>

#include <Wire.h> 
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27,16,2);  // set the LCD address to 0x27 for a 16 chars and 2 line display

#include <addons/TokenHelper.h>
#include <addons/RTDBHelper.h>

// SETUP FIREBASE
/* 1. Define the WiFi credentials */
#define WIFI_SSID "WIFI_SSID"
#define WIFI_PASSWORD "WIFI_PASSWORD"
/* 2. Define the API Key */
#define API_KEY "API_KEY"
/* 3. Define the RTDB URL */
#define DATABASE_URL "DATABASE_URL" 
/* database secret used in Firebase.setQueryIndex function */
#define DATABASE_SECRET "DATABASE_SECRET"
/* 4. Define the user Email and password that alreadey registerd or added in your project */
#define USER_EMAIL "EMAIL"
#define USER_PASSWORD "EMAIL_PASSWORD"

// Define the Firebase Data object
FirebaseData fbdo;
// Define the FirebaseAuth data for authentication data
FirebaseAuth auth;
// Define the FirebaseConfig data for config data
FirebaseConfig config;

unsigned long deleteDataMillis = 0, pushDataMillis = 0;

int count = 0;
bool indexing = false;

// SETUP DHT22
#define DHTPIN 2 // Digital pin connected to the DHT sensor, i used D4
#define DHTTYPE DHT11   // DHT 11
// #define DHTTYPE DHT22 // DHT 22  (AM2302), AM2321
// #define DHTTYPE DHT21   // DHT 21 (AM2301)
DHT dht(DHTPIN, DHTTYPE);
// define pin data
const int ledPin = 12; //LED connect to D6 of ESP8266

int alert = 0;

void setup(void)
{
  Serial.begin(115200);
  Serial.println();
  Serial.println();

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  Serial.begin(9600);
  Serial.println(F("DHT11 test!"));

  // LED SETUP
  pinMode(ledPin, OUTPUT);
  dht.begin();

  // DHT11 SETUP
  Serial.begin(115200); // 1bit=10µs
  Serial.println("\ntest capture DTH11");

  // FIREBASE SETUP
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  Serial.printf("Firebase Client v%s\n\n", FIREBASE_CLIENT_VERSION);

  /* Assign the api key (required) */
  config.api_key = API_KEY;

  /* Assign the user sign in credentials */
  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;

  /* Assign the RTDB URL (required) */
  config.database_url = DATABASE_URL;

  config.token_status_callback = tokenStatusCallback; 
  Firebase.reconnectNetwork(true);

  // Since v4.4.x, BearSSL engine was used, the SSL buffer need to be set.
  // Large data transmission may require larger RX buffer, otherwise connection issue or data read time out can be occurred.
  fbdo.setBSSLBufferSize(4096 /* Rx buffer size in bytes from 512 - 16384 */, 1024 /* Tx buffer size in bytes from 512 - 16384 */);

  Firebase.begin(&config, &auth);

  // NTP time sync
  configTime(3 * 3600, 0, "pool.ntp.org", "time.nist.gov");

  //LCD setup
  Wire.begin(4,5); // SDA connected to pin D2 and SCL connected to pin D1
  lcd.begin(16,2);
  lcd.init();
  lcd.clear();
  lcd.backlight();
}

void loop(void)
{
  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();
  // Readings every 2 seconds
  static unsigned long previousMillis = 0;
  if (millis() - previousMillis >= 2000)
  {
    previousMillis = millis();
    if (!isnan(humidity) && !isnan(temperature))
    {
      if (humidity > 60 || temperature > 40)
      {
        // Turn on LED
        digitalWrite(ledPin, HIGH);
        alert = 1;
      }
      else
      {
        // Turn off LED
        digitalWrite(ledPin, LOW);
        alert = 0;
      }
      //LCD DISPLAY
      lcd.setCursor(0,0);
      lcd.print("Temp: ");
      lcd.print(temperature);
      lcd.print("'C");
      lcd.setCursor(0,1);
      lcd.print("Humidity:");
      lcd.print(humidity);
      lcd.print("%");
      lcd.display();
      
      delay(1000);
      // FIREBASE
      if (Firebase.ready())
      {
        Serial.print("Humidity: ");
        Serial.print(humidity);
        Serial.print("%  Temperature: ");
        Serial.print(temperature);
        Serial.println("°C");
      }

    }
    else
    {
      Serial.println("Failed to read from DHT sensor!");
    }
  }

  if (Firebase.ready())
  {
    if (!indexing)
    {
      indexing = true;
      Serial.print("Set query index in database rules... ");
      if (Firebase.setQueryIndex(fbdo, "sensor/log", "ts", DATABASE_SECRET))
        Serial.println("ok");
      else
        Serial.println(fbdo.errorReason());
    }

    // push data every 30s
    if (millis() - pushDataMillis > 30 * 1000)
    {
      pushDataMillis = millis();

      count++;

      FirebaseJson json;

      json.add("ts", (uint32_t)time(nullptr));
      json.add("humidity", humidity);
      json.add("temperature", temperature);

      Serial.print("Pushing data... ");

      if (Firebase.push(fbdo, "sensor/log", json))
        Serial.println("ok");
      else
        Serial.println(fbdo.errorReason());
    }

    // delete old data every 1 min
    if (time(nullptr) > 1618971013 && millis() - deleteDataMillis > 60 * 1000)
    {
      deleteDataMillis = millis();

      Serial.print("Delete history data older than 10 minutes... ");

      if (Firebase.deleteNodesByTimestamp(fbdo, "sensor/log", "ts", 10 /* delete 10 nodes at once */, 10 * 60 /* retain data within 10 minutes */))
        Serial.println("ok");
      else
        Serial.println(fbdo.errorReason());
    }
  }
}
