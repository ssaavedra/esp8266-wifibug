#ifndef __CONFIG_H__
#define __CONFIG_H__


// Enable only when GPIO16 is connected to RST
#ifndef USE_ESP8266_DEEPSLEEP
#define USE_ESP8266_DEEPSLEEP 0
#endif

#ifndef CONFIG_WIFI_SSID
#define CONFIG_WIFI_SSID "ssid"
#endif
#ifndef CONFIG_WIFI_PASS
#define CONFIG_WIFI_PASS "password"
#endif
#ifndef CONFIG_UPLOAD_SERVER_HOST
#define CONFIG_UPLOAD_SERVER_HOST "espbug.labs.ssaavedra.eu"
#endif
#ifndef CONFIG_UPLOAD_SERVER_PORT
#define CONFIG_UPLOAD_SERVER_PORT 443
#endif
#ifndef CONFIG_UPLOAD_SERVER_SSL
#define CONFIG_UPLOAD_SERVER_SSL 0
#endif

// #define CONFIG_UPLOAD_SSL_SERVER_FINGERPRINT ""

#ifndef WIFI_CONNECTION_WAIT_THRESOLD_TICKS
#define WIFI_CONNECTION_WAIT_THRESOLD_TICKS 40
#endif

#ifndef MAX_FOUND_NETWORKS
#define MAX_FOUND_NETWORKS 32
#endif

#ifndef SSID_RECORD
#define SSID_RECORD 32
#endif



#endif
