#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <DHT.h>

LiquidCrystal_I2C lcd(0x27, 20, 4);

// Pin define
#define RST_TRIG 16
#define SOLENOID 15
#define PILOTLAMP 4
#define LEDSTRIP 5
#define OBSTACLE 13
#define DHTPIN 14
#define DHTTYPE DHT22
#define delayStart 500

DHT dht(DHTPIN, DHTTYPE);

const char* ssid = "Winnie the pooh";   // Ganti dengan SSID WiFi Anda
const char* password = "winniechan";  // Ganti dengan password WiFi Anda

// DEFINE LOCKER
String LockerNumber = "1";

bool stats;    // Variable state request code
bool state;    // Vdad
bool restart;  // Value restart from API
bool justopen;
int codeopen;

String APIHealt;
String code;
String timeleft;
String duration;
float t, h;
String temp;
String humi;
bool valueobs;
String Strobs;

void setup() {
  pinMode(SOLENOID, OUTPUT);
  digitalWrite(SOLENOID, HIGH);
  Serial.begin(115200);
  pinMode(PILOTLAMP, OUTPUT);
  pinMode(LEDSTRIP, OUTPUT);
  pinMode(OBSTACLE, INPUT_PULLUP);
  delay(100);
  dht.begin();
  lcd.init();
  lcd.backlight();  // Turn on the backlight
  lcd.home();
  delay(100);
  lcd.setCursor(4, 0);  // Move the cursor to the origin
  lcd.print("POWERING ON");
  lcd.setCursor(0, 1);
  lcd.print("Interfacing ESP32");
  lcd.setCursor(5, 2);
  lcd.print("NATSLOCK");
  lcd.setCursor(6, 3);
  lcd.print("XI TMA");
  delay(1000);
  connectWifi();
}

void loop() {
  getCode();
  readDHT();
  readObs();

  if (restart == 1) {
    return RST_DEVICE();
  }

  if (APIHealt == "No") {
    lcdIdleError();
  } else if (stats == 1 && state == 1) {
    lcdRentLocker();
  } else if (stats == 1 && state == 0) {
    lcdUnrentLocker();
  } else {
    if (state == 1) {
      if (justopen == 1) {
        solOpen();
        if (codeopen == 1) {
          lcdRentSuccess();
        } else if (codeopen == 2) {
          lcdOpenSuccess();
        } else if (codeopen == 3) {
          lcdUnrentSuccess();
        }
      } else {
        solClose();
        lcdIdleUnLock();
        pilotGreen();
      }
    } else {
      if (justopen == 1) {
        solOpen();
        if (codeopen == 1) {
          lcdRentSuccess();
        } else if (codeopen == 2) {
          lcdOpenSuccess();
        } else if (codeopen == 3) {
          lcdUnrentSuccess();
        }
      } else {
        solClose();
        lcdIdleLock();
        pilotRed();
      }
    }
  }

  // Wait before sending the next request
  delay(1000);
}

void getCode() {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;

    // Lokasi URL API yang akan diminta
    String url = "http://192.168.51.137:3000/api/lockerdb/" + LockerNumber;

    Serial.print("Sending HTTP GET request to: ");
    Serial.println(url);

    http.begin(url);
    int httpCode = http.GET();
    Serial.println(httpCode);

    if (httpCode == 502) {
      Serial.print("HTTP request failed with error code: ");
      Serial.println(httpCode);
      APIHealt = "No";
    }

    if (httpCode > 0) {
      if (httpCode == HTTP_CODE_OK) {
        String payload = http.getString();
        Serial.println("Response payload:");
        Serial.println(payload);

        // Parsing JSON
        DynamicJsonDocument doc(1024);
        deserializeJson(doc, payload);

        // Mendapatkan nilai OnGoing.stats
        stats = doc["OnGoing"]["stats"];
        state = doc["state"];
        restart = doc["NeedRestart"];
        code = doc["OnGoing"]["code"].as<String>();
        timeleft = doc["OnGoing"]["timeLeft"].as<String>();
        duration = doc["OnGoing"]["duration"].as<String>();
        justopen = doc["justOpen"];
        codeopen = doc["codeOpen"].as<int>();
        APIHealt = "Yes";
      }
    } else {
      Serial.print("HTTP request failed with error code: ");
      Serial.println(httpCode);
      APIHealt = "No";
    }

    http.end();
  }
}

// Remaining API functions (ApiPLGreen, ApiPLRed, etc.) and additional logic
// would follow similar structure as before but utilizing WiFi and HTTPClient for ESP32


void ApiPLGreen(String state) {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;

    // Lokasi URL API yang akan diminta
    String url = "http://192.168.51.137:3000/api/system/locker/pl/green/" + LockerNumber + "/" + state;

    Serial.print("Sending HTTP GET request to: ");
    Serial.println(url);

    // Menggunakan WiFiClient untuk memulai koneksi
    WiFiClient client;
    http.begin(client, url);

    int httpCode = http.GET();
    Serial.println(httpCode);
    if (httpCode == 502) {
      Serial.print("HTTP request failed with error code: ");
      Serial.println(httpCode);
      APIHealt = "No";
    }
    if (httpCode > 0) {
      // Penerimaan respon berhasil
      if (httpCode == HTTP_CODE_OK) {
        String payload = http.getString();
        Serial.println("Response payload:");
        Serial.println(payload);

        // Parsing JSON
        DynamicJsonDocument doc(1024);
        deserializeJson(doc, payload);

        // Mendapatkan nilai OnGoing.stats
        APIHealt = "Yes";
      }
    } else {
      Serial.print("HTTP request failed with error code: ");
      Serial.println(httpCode);
      APIHealt = "No";
    }

    // Menutup koneksi
    http.end();
  }
}

void ApiPLRed(String state) {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;

    // Lokasi URL API yang akan diminta
    String url = "http://192.168.51.137:3000/api/system/locker/pl/red/" + LockerNumber + "/" + state;

    Serial.print("Sending HTTP GET request to: ");
    Serial.println(url);

    // Menggunakan WiFiClient untuk memulai koneksi
    WiFiClient client;
    http.begin(client, url);

    int httpCode = http.GET();
    Serial.println(httpCode);
    if (httpCode == 502) {
      Serial.print("HTTP request failed with error code: ");
      Serial.println(httpCode);
      APIHealt = "No";
    }
    if (httpCode > 0) {
      // Penerimaan respon berhasil
      if (httpCode == HTTP_CODE_OK) {
        String payload = http.getString();
        Serial.println("Response payload:");
        Serial.println(payload);

        // Parsing JSON
        DynamicJsonDocument doc(1024);
        deserializeJson(doc, payload);

        // Mendapatkan nilai OnGoing.stats
        APIHealt = "Yes";
      }
    } else {
      Serial.print("HTTP request failed with error code: ");
      Serial.println(httpCode);
      APIHealt = "No";
    }

    // Menutup koneksi
    http.end();
  }
}

void ApiDHT(String tempe, String humid) {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;

    // Lokasi URL API yang akan diminta
    String url = "http://192.168.51.137:3000/api/system/locker/dht/" + LockerNumber + "/" + tempe + "/" + humid;

    Serial.print("Sending HTTP GET request to: ");
    Serial.println(url);

    // Menggunakan WiFiClient untuk memulai koneksi
    WiFiClient client;
    http.begin(client, url);

    int httpCode = http.GET();
    Serial.println(httpCode);
    if (httpCode == 502) {
      Serial.print("HTTP request failed with error code: ");
      Serial.println(httpCode);
      APIHealt = "No";
    }
    if (httpCode > 0) {
      // Penerimaan respon berhasil
      if (httpCode == HTTP_CODE_OK) {
        String payload = http.getString();
        Serial.println("Response payload:");
        Serial.println(payload);

        // Parsing JSON
        DynamicJsonDocument doc(1024);
        deserializeJson(doc, payload);

        // Mendapatkan nilai OnGoing.stats
        APIHealt = "Yes";
      }
    } else {
      Serial.print("HTTP request failed with error code: ");
      Serial.println(httpCode);
      APIHealt = "No";
    }

    // Menutup koneksi
    http.end();
  }
}

void ApiObs(String Strobs) {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;

    // Lokasi URL API yang akan diminta
    String url = "http://192.168.51.137:3000/api/system/locker/obs/" + LockerNumber + "/" + Strobs;

    Serial.print("Sending HTTP GET request to: ");
    Serial.println(url);

    // Menggunakan WiFiClient untuk memulai koneksi
    WiFiClient client;
    http.begin(client, url);

    int httpCode = http.GET();
    Serial.println(httpCode);
    if (httpCode == 502) {
      Serial.print("HTTP request failed with error code: ");
      Serial.println(httpCode);
      APIHealt = "No";
    }
    if (httpCode > 0) {
      // Penerimaan respon berhasil
      if (httpCode == HTTP_CODE_OK) {
        String payload = http.getString();
        Serial.println("Response payload:");
        Serial.println(payload);

        // Parsing JSON
        DynamicJsonDocument doc(1024);
        deserializeJson(doc, payload);

        // Mendapatkan nilai OnGoing.stats
        APIHealt = "Yes";
      }
    } else {
      Serial.print("HTTP request failed with error code: ");
      Serial.println(httpCode);
      APIHealt = "No";
    }

    // Menutup koneksi
    http.end();
  }
}

void connectWifi() {
  // Menghubungkan ESP8266 ke jaringan WiFi
  WiFi.begin(ssid, password);
  Serial.println("Connecting to WiFi");

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
    lcd.clear();
    lcd.setCursor(4, 0);  // Move the cursor at origin
    lcd.print("POWERING ON");
    lcd.setCursor(2, 1);
    lcd.print("Connecting to AP");
    lcd.setCursor(5, 2);
    lcd.print("NATSLOCK");
    lcd.setCursor(6, 3);
    lcd.print("XI TMA");
  }
  Serial.println("\nConnected to WiFi");
  lcd.clear();
  lcd.setCursor(4, 0);  // Move the cursor at origin
  lcd.print("POWERING ON");
  lcd.setCursor(2, 1);
  lcd.print("CONNECTED to AP");
  lcd.setCursor(5, 2);
  lcd.print("NATSLOCK");
  lcd.setCursor(6, 3);
  lcd.print("XI TMA");
  delay(1000);
  relayStartup();
  lcd.clear();
}

void lcdIdleUnLock() {
  lcd.clear();
  lcd.setCursor(6, 0);  // Move the cursor at origin
  lcd.print("NATSLOCK");
  lcd.setCursor(2, 1);
  lcd.print("Made by XI TM A");
  lcd.setCursor(0, 2);
  lcd.print("Rent This Locker At:");
  lcd.setCursor(3, 3);
  lcd.print("NatSlock.site");
  delay(2000);
  lcd.clear();
  lcd.setCursor(6, 0);  // Move the cursor at origin
  lcd.print("");
  lcd.setCursor(6, 0);
  lcd.print("LOCKER " + LockerNumber);
  lcd.setCursor(4, 1);
  lcd.print("THIS LOCKER");
  lcd.setCursor(5, 2);
  lcd.print("AVAILABLE");
  lcd.setCursor(3, 3);
  lcd.print("NatSlock.site");
  delay(1000);
  lcd.clear();
  lcd.setCursor(6, 0);  // Move the cursor at origin
  lcd.print("");
  lcd.setCursor(6, 0);
  lcd.print("LOCKER " + LockerNumber);
  lcd.setCursor(4, 1);
  lcd.print("THIS LOCKER");
  lcd.setCursor(5, 2);
  lcd.print("AVAILABLE");
  lcd.setCursor(3, 3);
  lcd.print(temp);
  lcd.setCursor(7, 3);
  lcd.print((char)223);
  lcd.setCursor(8, 3);
  lcd.print("C " + humi + " %");
}

void lcdIdleLock() {
  lcd.clear();
  lcd.setCursor(6, 0);  // Move the cursor at origin
  lcd.print("NATSLOCK");
  lcd.setCursor(3, 1);
  lcd.print("Made by XI TM A");
  lcd.setCursor(1, 2);
  lcd.print("THIS LOCKER RENTED");
  lcd.setCursor(2, 3);
  lcd.print("Use Other Locker");
  delay(1000);
  getCode();
  lcd.clear();
  lcd.setCursor(6, 0);  // Move the cursor at origin
  lcd.print("");
  lcd.setCursor(6, 0);
  lcd.print("LOCKER " + LockerNumber);
  lcd.setCursor(4, 1);
  lcd.print("THIS LOCKER");
  lcd.setCursor(3, 2);
  lcd.print("NOT AVAILABLE");
  lcd.setCursor(0, 3);
  lcd.print("Duration: " + duration);
  delay(1000);
  getCode();
  lcd.clear();
  lcd.setCursor(6, 0);  // Move the cursor at origin
  lcd.print("");
  lcd.setCursor(6, 0);
  lcd.print("LOCKER " + LockerNumber);
  lcd.setCursor(4, 1);
  lcd.print("THIS LOCKER");
  lcd.setCursor(3, 2);
  lcd.print("NOT AVAILABLE");
  lcd.setCursor(0, 3);
  lcd.print("Duration: " + duration);
  delay(1000);
  lcd.clear();
  lcd.setCursor(6, 0);  // Move the cursor at origin
  lcd.print("");
  lcd.setCursor(6, 0);
  lcd.print("LOCKER "+ LockerNumber);
  lcd.setCursor(4, 1);
  lcd.print("THIS LOCKER");
  lcd.setCursor(3, 2);
  lcd.print("NOT AVAILABLE");
  lcd.setCursor(3, 3);
  lcd.print(temp);
  lcd.setCursor(7, 3);
  lcd.print((char)223);
  lcd.setCursor(8, 3);
  lcd.print("C " + humi + " %");
}

void lcdIdleError() {
  lcd.clear();
  lcd.setCursor(6, 0);  // Move the cursor at origin
  lcd.print("NATSLOCK");
  lcd.setCursor(3, 1);
  lcd.print("Made by XI TM A");
  lcd.setCursor(1, 2);
  lcd.print("THIS LOCKER ERROR!");
  lcd.setCursor(3, 3);
  lcd.print("CONTACT ADMIN:");
  delay(1000);
  lcd.clear();
  lcd.setCursor(6, 0);  // Move the cursor at origin
  lcd.print("NATSLOCK");
  lcd.setCursor(3, 1);
  lcd.print("Made by XI TM A");
  lcd.setCursor(1, 2);
  lcd.print("THIS LOCKER ERROR!");
  lcd.setCursor(2, 3);
  lcd.print("WA:628213480805");
}

void lcdRentLocker() {
  lcd.clear();
  lcd.setCursor(4, 0);  // Move the cursor at origin
  lcd.print("RENT LOCKER");
  lcd.setCursor(2, 1);
  lcd.print("Valid: " + timeleft + " Left");
  lcd.setCursor(1, 2);
  lcd.print("Please Enter Code:");
  if (code == "CANCELED") {
    lcd.setCursor(2, 3);
    lcd.print("--  " + code + "  --");
  } else if (code.length() == 4) {
    lcd.setCursor(4, 3);
    lcd.print("--  " + code + "  --");
  } else {
    lcd.setCursor(5, 3);
    lcd.print("--  " + code + "  --");
  }
}

void lcdUnrentLocker() {
  lcd.clear();
  if (code.length() == 2) {
    lcd.setCursor(4, 0);  // Move the cursor at origin
    lcd.print("OPEN LOCKER");
  } else {
    lcd.setCursor(3, 0);  // Move the cursor at origin
    lcd.print("UNRENT LOCKER");
  }

  lcd.setCursor(2, 1);
  lcd.print("Valid: " + timeleft + " Left");
  lcd.setCursor(1, 2);
  lcd.print("Please Enter Code:");

  if (code == "CANCELED") {
    lcd.setCursor(2, 3);
    lcd.print("--  " + code + "  --");
  } else if (code.length() == 4) {
    lcd.setCursor(4, 3);
    lcd.print("--  " + code + "  --");
  } else {
    lcd.setCursor(5, 3);
    lcd.print("--  " + code + "  --");
  }
}

void pilotGreen() {
  digitalWrite(PILOTLAMP, HIGH);
  ApiPLGreen("on");
  ApiPLRed("off");
}

void pilotRed() {
  digitalWrite(PILOTLAMP, LOW);
  ApiPLGreen("off");
  ApiPLRed("on");
}

void relayStartup() {
  pilotGreen();
  delay(delayStart);
  pilotRed();
  delay(delayStart);
  pilotGreen();
  delay(delayStart);
  pilotRed();
  delay(delayStart);
  pilotGreen();
  delay(delayStart);
  pilotRed();
  delay(delayStart);
}

void RST_DEVICE() {
  lcd.clear();
  lcd.setCursor(6, 0);  // Move the cursor at origin
  lcd.print("NATSLOCK");
  lcd.setCursor(3, 1);
  lcd.print("REMOTE NEED TO");
  lcd.setCursor(1, 2);
  lcd.print("THIS LOCKER RESET");
  lcd.setCursor(2, 3);
  lcd.print("Wait For Moment...");
  pilotGreen();
  delay(delayStart);
  pilotRed();
  delay(delayStart);
  pilotGreen();
  delay(delayStart);
  pilotRed();
  delay(delayStart);
  pilotGreen();
  delay(delayStart);
  pilotRed();
  delay(delayStart);
  pinMode(RST_TRIG, OUTPUT);
  digitalWrite(RST_TRIG, LOW);
  delay(delayStart);
  digitalWrite(RST_TRIG, HIGH);
}

void lcdRentSuccess() {
  lcd.clear();
  lcd.setCursor(4, 0);
  lcd.print("RENT SUCCESS");
  lcd.setCursor(2, 1);
  lcd.print("OPEN THIS LOCKER");
  lcd.setCursor(4, 2);
  lcd.print("Don't Forget");
  lcd.setCursor(6, 3);
  lcd.print("To Close");
}
void lcdOpenSuccess() {
  lcd.clear();
  lcd.setCursor(4, 0);
  lcd.print("OPEN SUCCESS");
  lcd.setCursor(2, 1);
  lcd.print("OPEN THIS LOCKER");
  lcd.setCursor(4, 2);
  lcd.print("Don't Forget");
  lcd.setCursor(6, 3);
  lcd.print("To Close");
}

void lcdUnrentSuccess() {
  lcd.clear();
  lcd.setCursor(3, 0);
  lcd.print("UNRENT SUCCESS");
  lcd.setCursor(2, 1);
  lcd.print("OPEN THIS LOCKER");
  lcd.setCursor(4, 2);
  lcd.print("Don't Forget");
  lcd.setCursor(6, 3);
  lcd.print("To Close");
}

void readDHT() {
  h = dht.readHumidity();
  t = dht.readTemperature();
  temp = String(t);
  humi = String(h);

  if (isnan(h) || isnan(t)) {
    h = 0;
    t = 0;
    temp = String(t);
    humi = String(h);
    Serial.println("Failed to read from DHT sensor!");
  }

  ApiDHT(temp, humi);
}

void readObs() {
  valueobs = digitalRead(OBSTACLE);
  if (valueobs == 0) {
    String Strobs = "0";
    digitalWrite(LEDSTRIP, HIGH);
    ApiObs(Strobs);
  } else {
    String Strobs = "1";
    digitalWrite(LEDSTRIP, LOW);
    ApiObs(Strobs);
  }
  //Serial.println(valueobs);
}

void solOpen() {
  digitalWrite(SOLENOID, LOW);
}

void solClose() {
  digitalWrite(SOLENOID, HIGH);
}