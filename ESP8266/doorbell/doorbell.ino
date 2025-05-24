// Version 1.1
// Author: Remko Kleinjan
// Date: 191122

#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>

#define INPUTPIN D3

// WiFi
const char* ssid = "<SSID>";
const char* password = "<PASS>";
int waitDelay = 1000;

// Pushover
const char* url = "https://api.pushover.net/1/messages.json";
const char* token = "<TOKEN>";
const char* user = "<USER>";
const char* message = "Ding%20Dong!";
const char* sound = "echo";
int retryDelay = 5000;
int retryCount = 0;
int retry = 3;

// Doorbell
int readDelay = 50;
int buttonPressedDelay = 3000;
int inputValue = 0;

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

  WiFi.setAutoReconnect(true);
  //WiFi.persistent(true);
}

void pushover() {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    WiFiClientSecure wifiClient;
    wifiClient.setInsecure();

    Serial.println("Sending notification...");

    http.begin(wifiClient, url);
    http.setTimeout(retryDelay);

    http.addHeader("Content-Type", "application/x-www-form-urlencoded");
    String httpRequestData = String("token=") + token + "&user=" + user + "&message=" + message + "&sound=" + sound;
    Serial.println(String("POST: ") + url + "?" + httpRequestData);
    
    int httpResponseCode = http.POST(httpRequestData);

    if (httpResponseCode > 0) {
      Serial.print("HTTP Response code: ");
      Serial.println(httpResponseCode);
      String payload = http.getString();
      Serial.println(payload);
      wifiClient.stop();
      http.end();
    } else {
      Serial.print("Error code: ");
      Serial.println(httpResponseCode);
      wifiClient.stop();
      http.end();

      delay(retryDelay);
      if (retryCount < retry) {
        retryCount++;
        pushover();
      } else {
        retryCount = 0;
      }
    }
  } else {
    Serial.println("WiFi not connected ¯\\_(ツ)_/¯");
  }
}

void doorbell() {
  inputValue = digitalRead(INPUTPIN);
  digitalWrite(BUILTIN_LED, inputValue);

  if (inputValue == 0) {
    Serial.println("Doorbell pressed!");
    pushover();
    delay(buttonPressedDelay);
  } else {
    delay(readDelay);
  }
}

void setup() {
  Serial.begin(9600);

  Serial.println();
  Serial.println("Starting:");
  Serial.println("Doorbell v1.1");
  Serial.println();

  pinMode(INPUTPIN, INPUT_PULLUP);
  pinMode(BUILTIN_LED, OUTPUT);

  setupWifi();
}

void loop() {
  doorbell();
}
