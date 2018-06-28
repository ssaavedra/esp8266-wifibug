#ifndef WifiRecordList_h
#define WifiRecordList_h

#include <Arduino.h>

#ifndef SSID_RECORD
#define SSID_RECORD 32
#endif

#pragma pack(push)  /* push current alignment to stack */
#pragma pack(1)     /* set alignment to 1 byte boundary */
typedef struct wifi_strength {
  time_t timestamp;
  uint8_t bssid[6];
  char ssid[SSID_RECORD];
  int32_t rssi;
} wifi_strength_t;
#pragma pack(pop)   /* restore original alignment from stack */


class WifiRecordList {

public:
    WifiRecordList();
    WifiRecordList(size_t _max_records);
    WifiRecordList(struct wifi_strength *buffer, size_t _max_records);


    void reset();
    size_t len();

    // Returns the number of records appended
    int record_current_wifis(bool append = true);
    void print_all();
    void json_output_all(std::function<void(const char *)> printer);
    unsigned long json_length();

private:
    struct wifi_strength *recorded_strengths;
    int next_record = 0;
    size_t max_records;
};


#endif
