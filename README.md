ESP Wifi Quality Tracker
========================

This repository is part of the "Cookifying the Real World" project
that was prototyped at SummerLab2018 in Hirikilabs (Tabakalera,
Donostia, Euskadi, Spain, Earth).

This Arduino project holds the source code needed to program an
ESP8266 with a program that will do the following:

- Connect to a specific wifi network
- Get the current UTC time via SNTP
- Scan all wifi connections in range
- Push the list of `{timestamp, bssid, ssid, rssi} as a POST to a specific URL
- Sleep for a certain time

The data pushed includes:
- ts: the time (Unix Epoch format) of the data retrieval
- bssid: the binary ssid (the AP MAC address) advertised by the AP
- ssid: the human-friendly SSID name (up to a certain number of
  characters)
- rssi: the power received by the antenna, measured in dBm (according
  to the ESP8266/Arduino docs)

If your ESP8266 has deep-sleep mode enabled (i.e., `GPIO16` is
connected to `EXT_RSTB`; in the case of WeMos D1 Mini, that accounts
to `D0` being connected to `RST`), you can enable the flag
`USE_ESP8266_DEEPSLEEP` in config.h or platformio.ini to enable
deep-sleep.

Otherwise, keep it disabled in order to use the standard arduino
`sleep()` call.


Build and usage
---------------

We built the project using PlatformIO.

You may want to edit the provided platformio.ini file with the following build_flags:

```
build_flags =
    -DUSE_ESP8266_DEEPSLEEP=0
    -DCONFIG_WIFI_SSID=\"YOUR_SSID_NAME\"
    -DCONFIG_WIFI_PASS=\"YOUR_WPA_PASSWORD\"
    -DCONFIG_UPLOAD_SERVER_HOST=\"the.host.of.your.backend.server.example\"
    -DCONFIG_UPLOAD_SERVER_PORT=4444
    -DCONFIG_UPLOAD_SERVER_SSL=0
```

Flash using PlatformIO, with `pio run --target upload`, as usual.


Debugging
---------

In order to debug, connect the ESP8266 to a serial connection and open
the serial port at 115200 baud/s.

Use the output to open an issue (you should redact the information
that may be personally identifiable, if any).


Issues
------

For the moment, we are using GitHub issues as the main point of
contact.


Contributing
------------

Contributions to the project are very welcome. In order to keep the
development of this project as free software, please take note of the
license below.

When you provide contributions to be included under this project, you
agree to provide your code under the same license of the project.


License
-------

This project is released under the GNU General Public License version
3.0 published by the Free Software Foundation.
