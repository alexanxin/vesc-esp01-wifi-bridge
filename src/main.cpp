// vesc-esp01-wifi-bridge
// Transparent WiFi <-> UART bridge so VESC Tool (phone or desktop) can talk to a VESC
// through a ~1 EUR ESP-01 module.
//
// The ESP-01 runs its own WiFi access point. Join it, then in VESC Tool use
// Connection > TCP, address 192.168.4.1, port 65102 (or TCP_PORT).
// Bytes are passed unchanged between the TCP connection and the VESC UART.
//
// Use it for monitoring and settings. Do NOT upload VESC firmware over it.
//
// Configuration lives in platformio.ini (WIFI_SSID, WIFI_PASS, WIFI_TX_DBM, VESC_BAUD, TCP_PORT).
// SPDX-License-Identifier: MIT

#include <ESP8266WiFi.h>

#ifndef WIFI_SSID
#define WIFI_SSID "VESC-Bridge"
#endif
#ifndef WIFI_PASS
#define WIFI_PASS "CHANGE_ME"
#endif
#ifndef WIFI_TX_DBM
#define WIFI_TX_DBM 10.0
#endif
#ifndef VESC_BAUD
#define VESC_BAUD 115200
#endif
#ifndef TCP_PORT
#define TCP_PORT 65102
#endif

// ---- refuse to build with the placeholder or a too-short password ----
constexpr bool strEq(const char* a, const char* b) { return *a == *b && (*a == '\0' || strEq(a + 1, b + 1)); }
constexpr unsigned strLen(const char* s) { return *s ? 1 + strLen(s + 1) : 0; }
static_assert(!strEq(WIFI_PASS, "CHANGE_ME"), "Set your own WIFI_PASS in platformio.ini before building.");
static_assert(strLen(WIFI_PASS) >= 8 && strLen(WIFI_PASS) <= 63, "WIFI_PASS must be 8-63 characters (WPA2).");

WiFiServer server(TCP_PORT);
WiFiClient client;

void setup() {
  Serial.begin(VESC_BAUD);           // UART0 on the ESP-01 TX/RX pins -> VESC
  Serial.setDebugOutput(false);
  WiFi.mode(WIFI_AP);
  WiFi.setOutputPower(WIFI_TX_DBM);
  WiFi.softAP(WIFI_SSID, WIFI_PASS); // the ESP-01 is 192.168.4.1
  server.begin();
  server.setNoDelay(true);
}

void loop() {
  // accept a new client, replacing a stale one
  if (server.hasClient()) {
    if (client && client.connected()) client.stop();
    client = server.available();
    client.setNoDelay(true);
    while (Serial.available()) Serial.read();   // drop anything half-received
  }

  if (client && client.connected()) {
    uint8_t buf[256];
    // phone -> VESC
    int n = client.available();
    if (n > 0) {
      n = client.read(buf, n > (int)sizeof buf ? sizeof buf : n);
      Serial.write(buf, n);
    }
    // VESC -> phone
    n = Serial.available();
    if (n > 0) {
      n = Serial.readBytes(buf, n > (int)sizeof buf ? sizeof buf : n);
      client.write(buf, n);
    }
  } else {
    while (Serial.available()) Serial.read();     // nobody listening: discard
  }
  yield();
}
