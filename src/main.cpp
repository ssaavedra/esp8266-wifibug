#include <Arduino.h>
#include <time.h>
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <RestClient.h>
#include "config.h"

#define SLEEP_TIME 60

#ifndef MIN
#define MIN(a, b) ((a < b) ? a : b)
#endif


#include <WifiRecordList.h>

static const char ssid[] PROGMEM = CONFIG_WIFI_SSID;
static const char wifi_password[] PROGMEM = CONFIG_WIFI_PASS;

long global_iteration = 0;
char mac_header[32];
char global_buffer[500];
String response;
struct wifi_strength _wifi_records_buffer[MAX_FOUND_NETWORKS];
WifiRecordList wifi_records = WifiRecordList(_wifi_records_buffer, MAX_FOUND_NETWORKS);

char server_url_path[28];


void sleep(int seconds) {
  #if USE_ESP8266_DEEPSLEEP
  ESP.deepSleep(seconds * 1e6);
  #else
  delay(seconds * 1e3);
  #endif
}


void setup_time() {
    // Synchronize time useing SNTP. This is necessary to verify that
    // the TLS certificates offered by the server are currently valid.
    Serial.print(F("Setting time using SNTP"));
    // UTC, please
    configTime(0, 0, "pool.ntp.org", "time.nist.gov");
    time_t now = time(nullptr);
    while (now < 8 * 3600 * 2) {
      delay(500);
      Serial.print(".");
      now = time(nullptr);
    }
    Serial.println("");
    struct tm timeinfo;
    gmtime_r(&now, &timeinfo);
    Serial.print(F("Current time: "));
    Serial.print(asctime_r(&timeinfo, global_buffer));
}

bool wifi_connect(const char *ssid, const char *password) {
  String ssid_p = String((FPSTR(ssid)));
  String pw_p = String((FPSTR(password)));
  int wait_ticks = 0;
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid_p.c_str(), pw_p.c_str());
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");

    if(wait_ticks++ > WIFI_CONNECTION_WAIT_THRESOLD_TICKS) {
      Serial.print(F("Connection data was not useful to connect. Current status is: "));
      Serial.println(WiFi.status());
      sleep(SLEEP_TIME); // NORETURN
      return false;
    }
  }
  Serial.println("");
  Serial.print(F("WiFi connected to "));
  Serial.print(WiFi.BSSIDstr());
  Serial.print(F(" SSID: "));
  Serial.println(WiFi.SSID());
  Serial.println(F("IP address: "));
  Serial.println(WiFi.localIP());
  Serial.println(F("Using DNS Server"));
  Serial.println(WiFi.dnsIP());
  return true;
}


void print_known_wifis() {
  // wifi_records.print_all();

  wifi_records.json_output_all([](const char *p) {
    Serial.print(p);
  });
}

void record_wifis() {
  wifi_records.record_current_wifis(true);
}

/*
#ifndef CONFIG_UPLOAD_SERVER_SSL_FINGERPRINT
RestClient client = RestClient(CONFIG_UPLOAD_SERVER_HOST, CONFIG_UPLOAD_SERVER_PORT, CONFIG_UPLOAD_SERVER_SSL);
#else
RestClient client = RestClient(CONFIG_UPLOAD_SERVER_HOST, CONFIG_UPLOAD_SERVER_PORT, (char*)CONFIG_UPLOAD_SERVER_SSL_FINGERPRINT);
#endif
*/


void setup() {
  RestClient client = RestClient(CONFIG_UPLOAD_SERVER_HOST, CONFIG_UPLOAD_SERVER_PORT, (int) CONFIG_UPLOAD_SERVER_SSL);
  Serial.begin(115200);
  Serial.println();
  Serial.print(F("My MAC address is "));
  Serial.println(WiFi.macAddress());
  Serial.print(F("connecting to "));
  Serial.println(FPSTR(ssid));
  Serial.print("Using SSL: ");
  Serial.println(CONFIG_UPLOAD_SERVER_SSL);

  if(!wifi_connect(ssid, wifi_password)) { sleep(SLEEP_TIME * 2); return;}
  setup_time();
  // WiFi.disconnect();

  strcpy_P(mac_header, PSTR("X-DEVICE-MAC: "));
  strcat(mac_header, WiFi.macAddress().c_str());

  strcpy_P(server_url_path, PSTR("/data/"));
  strcat(server_url_path, WiFi.macAddress().c_str());

  client.setContentType("application/json");
  client.setHeader(mac_header);


  response = "";
  Serial.println("Begin loop");
  record_wifis();
  Serial.println("Wifi networks recorded");
  print_known_wifis();

  int statusCode = client.request(
    "POST", server_url_path,
    [](std::function<void(const char*)> printer) {
      String *len = new String(wifi_records.json_length());
      printer("Content-Length: ");
      printer(len->c_str());
      delete len;
      printer("\r\n\r\n");
      wifi_records.json_output_all(printer);
      printer("\r\n");
    },
    &response
  );

  if(statusCode == 200) {
    wifi_records.reset();
    Serial.print(F("Status code from server: "));
    Serial.println(statusCode);
    Serial.print(F("Response body from server: "));
    Serial.println(response);

    // Avoid wifi disconnection when not deep-sleeping
    #if USE_ESP8266_DEEPSLEEP
      WiFi.disconnect(true);
      Serial.println(F("Deep-Sleeping"));
    #else
      Serial.println(F("Sleeping"));
    #endif
    sleep(SLEEP_TIME);
  } else if(statusCode <= 100) {
    // Could not connect to the server!
    Serial.println(F("Could not connect to the server. Light-sleeping and retrying."));
    delay(1000);
  }
  #if !USE_ESP8266_DEEPSLEEP
    if(WiFi.status() != WL_CONNECTED && !wifi_connect(ssid, wifi_password)) {return;}
  #endif
}

// For the case where deepSleep is not used
void loop() { setup(); delay(1000); }
