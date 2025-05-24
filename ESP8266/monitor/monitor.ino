// Version 1.1
// Author: Remko Kleinjan
// Date: 251122

#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266Ping.h>

// Global
int retryDelay = 5000;

// WiFi
const char* ssid = "<SSID>";
const char* password = "<PASS>";
int waitDelay = 1000;

// External IP
const char* url = "http://checkip.dyndns.org";
String payload = "";
String old_payload = "";
int check = 0;

// Internal ping
int dhcp_range_begin = 2;
int dhcp_range_end = 100;
String gateway = "192.168.1.1";
String ping = "";

// Webserver
ESP8266WebServer server(80);

// Pushover
const char* p_url = "https://api.pushover.net/1/messages.json";
const char* token = "<TOKEN>";
const char* user = "<USER>";
const char* sound = "pianobar";
int retryCount = 0;
int retry = 3;

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

void setupWeb() {
  server.on("/", handleRoot);
  server.on("/monitor", handleMonitor);
  server.onNotFound(handleNotFound);

  server.begin();
}

void setup() {
  //Serial.begin(115200);
  Serial.begin(9600);

  Serial.println();
  Serial.println("Starting:");
  Serial.println("Monitor v1.1");
  Serial.println();

  pinMode(BUILTIN_LED, OUTPUT);

  setupWifi();
  setupWeb();
  getExtIP();
}

void getExtIP() {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    WiFiClient wifiClient;
   
    Serial.println("Getting external IP...");

    http.begin(wifiClient, url);
    http.setTimeout(retryDelay);

    int httpResponseCode = http.GET();
    
    if (httpResponseCode > 0) {
      Serial.print("HTTP Response code: ");
      Serial.println(httpResponseCode);
      if (httpResponseCode == 502) {
        Serial.println("Bad Gateway");
      } else {
        old_payload = payload;
        payload = http.getString();
        Serial.print("HTTP Response: ");
        Serial.println(payload);
      }
      wifiClient.stop();
      http.end();
      checkIP();
    } else {
      Serial.print("Error code: ");
      Serial.println(httpResponseCode);
      wifiClient.stop();
      http.end();

      delay(retryDelay);
      if (retryCount < retry) {
        retryCount++;
        getExtIP();
      } else {
        retryCount = 0;
      }
    }
  } else {
    Serial.println("WiFi not connected ¯\\_(ツ)_/¯");
  }
}

void checkIP() {
  if (payload != old_payload) {
    Serial.println("IP change detected!");
    pushover();
  } else {
    Serial.println("No IP change detected");
    Serial.println();
  }
}

void intPing() {
  if (WiFi.status() == WL_CONNECTED) {
    // Reset
    int dhcp_begin = dhcp_range_begin;
    int dhcp_end = dhcp_range_end;
    ping = "";

    String status = "";

    Serial.println("Scanning LAN:");

    String lan = WiFi.localIP().toString();
    int index = lan.lastIndexOf(".");
    String sub_lan = lan.substring(0, index);
  
    for (dhcp_begin; dhcp_begin <= dhcp_end; dhcp_begin++) {
      String ip = sub_lan + "." + dhcp_begin;
      Serial.print(ip + ": ");
      bool success = Ping.ping(ip.c_str(), 1);
      Serial.println(success);

      if (success) {
        status = "[X]";
      } else {
        status = "[&nbsp;&nbsp;&nbsp;]";
      }
      ping += ip + ": " + status + "<br>";

      if (dhcp_begin == 199) {
        dhcp_begin = 252;
        dhcp_end = 254;
      }
    }

    Serial.print(gateway + ": ");
    bool success = Ping.ping(gateway.c_str(), 1);
    Serial.println(success);

    if (success) {
      status = "[X]";
    } else {
      status = "[&nbsp;&nbsp;&nbsp;]";
    }
    ping += gateway + ": " + status + "<br>";

    Serial.println();
  } else {
    Serial.println("WiFi not connected ¯\\_(ツ)_/¯");
  }
}

void pushover() {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    WiFiClientSecure wifiClient;
    wifiClient.setInsecure();

    Serial.println("Sending notification...");

    http.begin(wifiClient, p_url);
    http.setTimeout(retryDelay);

    http.addHeader("Content-Type", "application/x-www-form-urlencoded");
    String httpRequestData = String("token=") + token + "&user=" + user + "&message=" + payload + "&sound=" + sound + "&html=1";
    Serial.println(String("POST: ") + url + "?" + httpRequestData);
    
    int httpResponseCode = http.POST(httpRequestData);

    if (httpResponseCode > 0) {
      Serial.print("HTTP Response code: ");
      Serial.println(httpResponseCode);
      String payload = http.getString();
      Serial.println(payload);
      Serial.println();
      wifiClient.stop();
      http.end();
    } else {
      Serial.print("Error code: ");
      Serial.println(httpResponseCode);
      Serial.println();
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

void handleRoot() {
  Serial.println("HTTP: /");
  server.send(200, "text/plain", "Welcome.");
}

void handleMonitor() {
  Serial.println("HTTP: /monitor");
  getExtIP();
  intPing();
  server.send(200, "text/html", payload + "<br><br>" + ping);
}

void handleNotFound(){
  Serial.println("HTTP: 404");
  server.send(404, "text/plain", "404: Not found");
}

void loop() {
  // Blink if connected to wifi
  digitalWrite(BUILTIN_LED, HIGH);
  delay(2500);

  if (WiFi.isConnected()) {
    digitalWrite(BUILTIN_LED, LOW);
    delay(500);
  }

  // 3(000)s * 1200 = 3600s = 1h
  if (check == 1200) {
    getExtIP();
    check = 0;
  }
  check++;

  // Listen for HTTP requests
  server.handleClient();
}
