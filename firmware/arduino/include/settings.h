#ifndef SETTINGS_H
#define SETTINGS_H

#include <stdint.h> // because uint16_t type declarations on platform.io

#include "credentials.h"

#define TRIGGER_PIN D3
#define BUTTON_PIN D2 // D4 is Pin 2 = LED
#define DHT_PIN D1

const char myHOSTNAME[] = "glade"; // http://esp8266.local - max value 63 characters


const char *NTP_SERVER = "pt.pool.ntp.org";                            // See https://github.com/nayarsystems/posix_tz_db/blob/master/zones.csv for Timezone codes for your region
const char *TZ_INFO = "WET-0WEST-1,M3.5.0/01:00:00,M10.5.0/02:00:00";  // enter your time zone (https://remotemonitoringsystems.ca/time-zone-abbreviations.php)
const int UPDATE_NTP_INTERVAL_MS = 600000;


struct settings_t {
    bool active;
    bool days[7];
    bool hours[24];
    uint16_t interval;
} settings;


#endif /* SETTINGS_H */