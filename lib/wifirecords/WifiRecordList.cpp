#include "WifiRecordList.h"
#include <time.h>
#include <ESP8266WiFi.h>

#ifndef MIN
#define MIN(a, b) ((a < b) ? a : b)
#endif
#ifndef MAX
#define MAX(a, b) ((a > b) ? a : b)
#endif

#ifdef WRL_DEBUG
#define WRL_DEBUG_PRINT(string) (Serial.print(string))
#define WRL_DEBUG_PRINTF(string, ...) (Serial.printf(string, __VA_ARGS__))
#endif

#ifndef WRL_DEBUG
#define WRL_DEBUG_PRINT(string)
#define WRL_DEBUG_PRINTF(string, ...)
#endif

WifiRecordList::WifiRecordList(struct wifi_strength *buffer, size_t _max_records) {
  next_record = 0;
  recorded_strengths = buffer;
  max_records = _max_records;
}

void WifiRecordList::reset() {
  next_record = 0;
}

size_t WifiRecordList::len() {
  return next_record;
}

int WifiRecordList::record_current_wifis(bool append) {
  if(!append) next_record = 0;

  int8_t found_networks = WiFi.scanNetworks();
  WRL_DEBUG_PRINT("Networks found: ");
  WRL_DEBUG_PRINT(found_networks);
  WRL_DEBUG_PRINTF("\nWe have space for %d networks, which is MIN(%d, %d - %d)\n", MIN(found_networks, max_records - next_record),
found_networks, max_records, next_record);

  time_t now = time(nullptr);
  for(int i = 0; i < MIN(found_networks, ((int32_t) max_records) - next_record); i++) {
    recorded_strengths[next_record].timestamp = now;
    strncpy(recorded_strengths[next_record].ssid, WiFi.SSID(i).c_str(), SSID_RECORD);
    for(int j = 0; j < 6; j++) {
      uint8_t *bssid = WiFi.BSSID(i);
      recorded_strengths[next_record].bssid[j] = bssid[j];
    }
    recorded_strengths[next_record].rssi = WiFi.RSSI(i);
    next_record++;
  }

  WiFi.scanDelete();
  return MAX(0, found_networks - (max_records - next_record));
}


void WifiRecordList::print_all() {
  struct tm *timeinfo;
  Serial.print(("WiFi networks found: "));
  Serial.println(next_record);
  for(int i = 0; i < next_record; i++) {
    Serial.printf("-- WIFI #%02i\n", i);
    Serial.print("Timestamp: ");
    timeinfo = gmtime(&recorded_strengths[i].timestamp);
    Serial.print(asctime(timeinfo));
    Serial.print("BSSID: ");
    Serial.printf(
      "%02x:%02x:%02x:%02x:%02x:%02x\n",
      recorded_strengths[i].bssid[0],
      recorded_strengths[i].bssid[1],
      recorded_strengths[i].bssid[2],
      recorded_strengths[i].bssid[3],
      recorded_strengths[i].bssid[4],
      recorded_strengths[i].bssid[5]
    );
    Serial.printf("SSID: %s\n", recorded_strengths[i].ssid);
    Serial.printf("RSSI: %d dBm", recorded_strengths[i].rssi);
    Serial.printf("\n\n");
  }
}


static const char *bssid_to_str(const uint8_t *bssid) {
  static char buffer[24];
  snprintf(buffer, 24, "%02x:%02x:%02x:%02x:%02x:%02x", bssid[0], bssid[1], bssid[2], bssid[3], bssid[4], bssid[5]);
  return buffer;
}

void record_to_json(std::function<void(const char *)> printer, struct wifi_strength record) {
  static char buffer[100]; // Don't consume stack space
  sprintf_P(
    buffer,
    PSTR("{\"ts\": %d, \"bssid\": \"%s\", \"ssid\": \"%s\", \"rssi\": %d}"),
    record.timestamp,
    bssid_to_str(record.bssid),
    record.ssid,
    record.rssi
  );
  printer(buffer);
}

void record_to_json(char *dest, struct wifi_strength record) {
  sprintf(
    dest,
    ("{\"ts\": %ld, \"bssid\": \"%s\", \"ssid\": \"%s\", \"rssi\": %d}"),
    record.timestamp,
    bssid_to_str(record.bssid),
    record.ssid,
    record.rssi
  );
}

void WifiRecordList::json_output_all(char *dest) {
  strcat(dest, "[");
  for(int i = 0; i < next_record; i++) {
    record_to_json(dest, recorded_strengths[i]);
    if(i + 1 < next_record) { strcat(dest, ",\n"); };
  }
  strcat(dest, "]\n");
}

unsigned long WifiRecordList::json_length() {
  size_t length = 3;
  char buffer[100];
  buffer[0] = '\0';

  for(int i = 0; i < next_record; i++) {
    record_to_json(buffer, recorded_strengths[i]);
    length += strlen(buffer);
    if(i + 1 < next_record) { i++; };
  }
  return length;
}
