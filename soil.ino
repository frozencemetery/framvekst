/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include <Adafruit_seesaw.h>
#include <LiquidCrystal.h>

#define BAUD 9600

typedef enum {
    SERIAL_TX = 0,
    SERIAL_RX = 1,

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

    /* RELAY = 21, */
} pin;

Adafruit_seesaw soil;
LiquidCrystal lcd(RS, EN, DB4, DB5, DB6, DB7);

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
    /* pinMode(RELAY, OUTPUT); */

    /* pinMode(RED, OUTPUT); */
    /* pinMode(GREEN, OUTPUT); */
    /* pinMode(BLUE, OUTPUT); */

    lcd.begin(16, 2);
    lcd.print("DO YOU LIKE");
    lcd.setCursor(0, 1);
    lcd.print("BEETS???");

    if (!soil.begin(0x36)) {
        lcd.clear();
        lcd.print("Couldn't find soil sensor!");
        while (1) {
            blink();
            sdelay(300);
        }
    }

    blink();
}

void loop(void) {
    int delay_secs = 5;
    float temp_celsius;
    uint16_t capacitance;

    lcd.clear();

    temp_celsius = soil.getTemp();
    capacitance = soil.touchRead(0);

    lcd.print("Current: ");
    lcd.print(capacitance);

    lcd.setCursor(0, 1);
    /* lcd.print("Temp: "); */
    lcd.print(temp_celsius);
    lcd.print("C (+/-2)");
    
    sdelay(delay_secs);
}

/* Local variables: */
/* c-basic-offset: 4 */
/* indent-tabs-mode: nil */
/* End: */
