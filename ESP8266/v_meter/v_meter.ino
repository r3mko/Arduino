// Version 1.2
// Author: Remko Kleinjan
// Date: 030223

#include <ZMPT101B.h>
#include <CircularBuffer.h>
#include <LiquidCrystal_I2C.h>
#include <ESP8266WiFi.h>
#include <InfluxDbClient.h>
#include <InfluxDbCloud.h>
#include <ESP8266WebServer.h>

// We have the ZMPT101B sensor connected to A0 pin
ZMPT101B voltageSensor(A0);
float V = 0;
float V_mean = 0;
float V_write = 0;
CircularBuffer<float, 60> buffer;
float Cut_off = 5; // V

// We have the LCD display connected to D1/GPIO5 (SCL) and D2/GPIO4 (SDA)
// Set the LCD address to 0x27 for a 16 chars and 2 line display
LiquidCrystal_I2C lcd(0x27, 16, 2);

// InfluxDB
#define INFLUXDB_URL "<URL>"
#define INFLUXDB_TOKEN "<TOKEN>"
#define INFLUXDB_ORG "<ORG>"
#define INFLUXDB_BUCKET "<BUCKET>"

// Set location
#define LOCATION "<LOCATION>"

// Declare InfluxDB client instance with preconfigured InfluxCloud certificate
InfluxDBClient client(INFLUXDB_URL, INFLUXDB_ORG, INFLUXDB_BUCKET, INFLUXDB_TOKEN, InfluxDbCloud2CACert);
// Declare Data point
Point sensor("power_status");
// Time zone info
#define TZ_INFO "<TZ>"

// Webserver
ESP8266WebServer server(80);

int waitDelay = 1000;
int count = 0;

// WiFi
const char* ssid = "<SSID>";
const char* password = "<PASS>";

/* Calibrate hardware: */
//
//void setup() {
//  Serial.begin(9600);
//  pinMode(A0,INPUT);
//}

//void loop() {
//  Serial.println(analogRead(A0));
//  delay(50);
//}

/* Calibrate software: (No power input) */
//
//void setup() {
//  Serial.begin(9600);
//}

//void loop() {
//  int Z = voltageSensor.calibrate();
//  Serial.println(String("Zero: ") + Z);
//  delay(1000);
//}

void setupSensor() {
  //Set Vref
  voltageSensor.setVref(3.3);

  // Number from software calibration
  voltageSensor.setZeroPoint(123);

  // Match to current voltage
  voltageSensor.setSensitivity(0.001234);
}

void setupDisplay() {
  lcd.init();         
  lcd.clear();
  //lcd.backlight();

  lcd.setCursor(1, 0);
  lcd.print("V Monitor v1.2");
}

void setupWifi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  Serial.print("Connecting");
  while (WiFi.status() != WL_CONNECTED) {
    delay(waitDelay);
    Serial.print(".");
  }
  Serial.println();

  Serial.print("Connected to: ");
  Serial.println(WiFi.SSID());
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  WiFi.setAutoReconnect(true);
  //WiFi.persistent(true);
}

void setupInfluxDB() {
  // Set time
  timeSync(TZ_INFO, "pool.ntp.org", "time.nist.gov");

  // Check server connection
  if (client.validateConnection()) {
    Serial.print("Connected to InfluxDB: ");
    Serial.println(client.getServerUrl());
  } else {
    Serial.print("InfluxDB connection failed: ");
    Serial.println(client.getLastErrorMessage());
  }

  // Add tags to the data point
  sensor.addTag("location", LOCATION);
}

void setupWeb() {
  server.on("/", handleRoot);
  server.onNotFound(handleNotFound);

  server.begin();
}

void setup() {
  Serial.begin(9600);
  pinMode(BUILTIN_LED, OUTPUT);
  digitalWrite(BUILTIN_LED, HIGH);

  setupSensor();
  setupDisplay();
  setupWifi();
  setupInfluxDB();
  setupWeb();

  lcd.clear();
}

void writeInfluxDB() {
  if (WiFi.status() == WL_CONNECTED) {
    // Clear fields for reusing the point. Tags will remain the same as set above.
    sensor.clearFields();
  
    // Store measured value into point
    // Report RSSI of currently connected network
    sensor.addField("voltage", V_write);
  
    // Print what are we exactly writing
    Serial.print("Writing: ");
    Serial.print(sensor.toLineProtocol());

    // Write point
    if (client.writePoint(sensor)) {
      Serial.println(" [OK]");
    } else {
      Serial.println(" [FAIL]");
      Serial.println(client.getLastErrorMessage());
    }
  } else {
    Serial.println("WiFi not connected ¯\\_(ツ)_/¯");
  }
}

void handleRoot() {
  float s = 0;

  if (server.hasArg("s")) {
    s = server.arg("s").toFloat();
    if (s > 0) {
      Serial.println("Set sensor sensitivity: " + String(s, 6));
      voltageSensor.setSensitivity(s);
    }
  }

  //Serial.println("HTTP: /");
  server.send(200, "text/plain", String(s, 6));
}

void handleNotFound(){
  Serial.println("HTTP: 404");
  server.send(404, "text/plain", "404: Not found");
}

float filter(float input) {
//  if (input >= Cut_off) {
//    input -= Cut_off;
//  } else {
//    input = 0;
//  }

  if (input <= Cut_off) {
    input = 0;
  }

  return input;
}

void loop() {
  // Zero point live
  voltageSensor.calibrateLive();

  V = voltageSensor.getVoltageAC();
  //Serial.println(String(V) + " V");
  buffer.push(V);

  // Calculate moving average:
  // Set correct indeex type
	using index_t = decltype(buffer)::index_t;
  // Reset average
  V_mean = 0;
  // Calculate according to items in buffer
	for (index_t i = 0; i < buffer.size(); i++) {
	  V_mean += buffer[i] / (float)buffer.size();
	}

  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print(String("cur: ") + filter(V)  + " V");
  lcd.setCursor(0, 1);
  lcd.print(String("avg: ") + filter(V_mean) + " V");

  //lcd.setCursor(4, 0);
  //lcd.print(String(filter(V_mean)) + " V");

  count++;
  if (count == 60) {
    digitalWrite(BUILTIN_LED, LOW);

    V_write = filter(V_mean);
    writeInfluxDB();
    count = 0;

    digitalWrite(BUILTIN_LED, HIGH);
  }

  // Listen for HTTP requests
  server.handleClient();

  delay(waitDelay);
}
