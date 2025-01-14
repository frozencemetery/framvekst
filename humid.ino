/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include <Adafruit_SHT31.h>
#include <LiquidCrystal.h>

#define BAUD 9600

typedef enum {
    SERIAL_TX = 0,
    SERIAL_RX = 1,

    RELAY = 2,
    /* RED = 3, */
    /* GREEN = 5, */
    /* BLUE = 6, */
    RS = 7,
    EN = 8,
    DB4 = 9,
    DB5 = 10,
    DB6 = 11,
    DB7 = 12,
    /* LED_BUILTIN = 13, */

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

    /* pinMode(RED, OUTPUT); */
    /* pinMode(GREEN, OUTPUT); */
    /* pinMode(BLUE, OUTPUT); */

    lcd.begin(16, 2);
    lcd.print("\"potatoes\"...");

    blink();

    if (!sht30.begin()) {
        while (1) {
            blink();
            delay(2000);
        }
    }
}

void loop(void) {
    float humidity, temperature;
    int delay_secs = 5;

    humidity = sht30.readHumidity();
    temperature = sht30.readTemperature();
    if (isnan(humidity) || isnan(temperature)) {
        lcd.clear();
        lcd.print("SHT30 bad read!");
        blink();
        sdelay(5);
        return;
    }

    lcd.clear();
    lcd.print(humidity);
    lcd.print("%RH ");

    if (humidity < 40 && !relay_on) {
        digitalWrite(RELAY, HIGH);
        relay_on = true;
        delay_secs += 300;
        lcd.print("*on*");
    } else if (humidity > 45 && relay_on) {
        digitalWrite(RELAY, LOW);
        relay_on = false;
        delay_secs += 300;
        lcd.print("*off*");
    } else if (relay_on) {
        lcd.print("on");
    } else {
        lcd.print("off");
    }

    lcd.setCursor(0, 1);
    lcd.print(temperature);
    lcd.print("C (");
    lcd.print(fofc(temperature));
    lcd.print("F)");

    sdelay(delay_secs);
}

/* Local variables: */
/* c-basic-offset: 4 */
/* indent-tabs-mode: nil */
/* End: */
