/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include <Adafruit_SHT31.h>
#include <LiquidCrystal.h>

/* Adapted from humid.ino to drive a heater instead of a humidifier. */

/* Farenheit. */
#define TEMP_MIN 35
#define TEMP_MAX 39

#define BAUD 9600
#define BASE_DELAY 5

typedef enum {
    /* SERIAL_TX = 0, */
    /* SERIAL_RX = 1, */

    RS = 7,
    EN = 8,
    DB4 = 9,
    DB5 = 10,
    DB6 = 11,
    DB7 = 12,
    /* LED_BUILTIN = 13, */

    RELAY = 17,
    SDA_PIN = 18,
    SCL_PIN = 19,
} pin;

LiquidCrystal lcd(RS, EN, DB4, DB5, DB6, DB7);
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

inline float fofc(float celsius) {
    return (9 * celsius) / 5 + 32;
}

void setup(void) {
    Serial.begin(BAUD);
    while (!Serial) {
        delay(100);
    }

    pinMode(LED_BUILTIN, OUTPUT);
    pinMode(RELAY, OUTPUT);

    lcd.begin(16, 2);
    lcd.print("Hi farmer!");

    blink();

    if (!sht30.begin()) {
        Serial.println("Sensor not found!");
        while (1) {
            blink();
            delay(2000);
        }
    }
}

void loop(void) {
    float humidity, temperature;
    int delay_secs = BASE_DELAY;

    if (!sht30.readBoth(&temperature, &humidity)) {
        Serial.println("Sensor read failure!");
        goto done;
    }

    temperature = fofc(temperature);

    if (relay_on && temperature > TEMP_MAX) {
        digitalWrite(RELAY, LOW);
        relay_on = false;
        delay_secs += 10;
    } else if (!relay_on && temperature < TEMP_MIN) {
        digitalWrite(RELAY, HIGH);
        relay_on = true;
        delay_secs += 10;
    }

    lcd.clear();
    lcd.print(temperature);
    lcd.print("F, heat ");
    if (relay_on) {
        lcd.print("on");
    } else {
        lcd.print("off");
    }

    lcd.setCursor(0, 1);
    lcd.print("Humidity: ");
    lcd.print(humidity);
    lcd.print("%");

done:
    sdelay(delay_secs);
}

/* Local variables: */
/* c-basic-offset: 4 */
/* indent-tabs-mode: nil */
/* End: */
