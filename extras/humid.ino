/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include <Adafruit_SHT31.h>

#define BAUD 9600

typedef enum {
    SERIAL_0 = 0,
    SERIAL_1 = 1,
    
    /* LED_BUILTIN = 13, */

    SDA_PIN = 18,
    SCL_PIN = 19,

    RELAY = 21,
} pin;

Adafruit_SHT31 sht30;
bool relay_on;

inline void blink(void) {
    digitalWrite(LED_BUILTIN, HIGH);
    delay(2000);
    digitalWrite(LED_BUILTIN, LOW);
}

/* Small ints - they overflow. */
inline void sdelay(int seconds) {
    while (seconds > 30) {
        delay(30 * 1000);
        seconds -= 30;
    }
    delay(seconds * 1000);
}

void setup(void) {
    pinMode(LED_BUILTIN, OUTPUT);
    pinMode(RELAY, OUTPUT);

    blink();

    /* Serial.begin(BAUD); */
    /* while (!Serial) { */
    /*     delay(100); */
    /* } */
    /* Serial.println("Console connected"); */

    if (!sht30.begin()) {
        Serial.println("Couldn't find SHT30!");
        while (1) {
            blink();
            delay(2000);
        }
    }
}

void loop(void) {
    float humidity;

    humidity = sht30.readHumidity();
    if (isnan(humidity)) {
        blink();
    }

    /* Serial.print("Read: "); */
    /* Serial.print(humidity); */
    /* Serial.println("%RH"); */

    if (humidity < 48 && !relay_on) {
        digitalWrite(RELAY, HIGH);
        relay_on = true;
        sdelay(60);
    } else if (humidity > 53 && relay_on) {
        digitalWrite(RELAY, LOW);
        relay_on = false;
        sdelay(120);
    }

    delay(2000); /* Allow time between sensor readings. */
}

/* Local variables: */
/* c-basic-offset: 4 */
/* indent-tabs-mode: nil */
/* End: */
