/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include <Adafruit_seesaw.h>
#include <LiquidCrystal.h>

#define BAUD 9600
#define DELAY_SECS 5

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
    RED_VCC = 14,
    RED_SIG = 15,

    SDA_PIN = 18,
    SCL_PIN = 19,

} pin;

Adafruit_seesaw soil;
LiquidCrystal lcd(RS, EN, DB4, DB5, DB6, DB7);

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

    pinMode(RED_VCC, OUTPUT);
    digitalWrite(RED_VCC, LOW);

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
    float temp_celsius;
    uint16_t g_cap;
    int r_cap;

    lcd.clear();

    temp_celsius = soil.getTemp();
    g_cap = soil.touchRead(0);

    /* Powering this on and off saves on corrosion. */
    digitalWrite(RED_VCC, HIGH);
    delay(10); /* Example code uses 10ms for stabilization. */
    r_cap = analogRead(RED_SIG);
    digitalWrite(RED_VCC, LOW);

    lcd.print("G: ");
    lcd.print(g_cap);
    lcd.print(", ");
    lcd.print(fofc(temp_celsius));
    lcd.print("F");
    /* lcd.print(temp_celsius); */
    /* lcd.print("C"); */
    /* lcd.print("C (+/-2)"); */

    lcd.setCursor(0, 1);
    lcd.print("R: ");
    lcd.print(r_cap);
    sdelay(DELAY_SECS);
}

/* Local variables: */
/* c-basic-offset: 4 */
/* indent-tabs-mode: nil */
/* End: */
