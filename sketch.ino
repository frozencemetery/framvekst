/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include <Adafruit_SHT31.h>
#include <Adafruit_AM2315.h>
#include <OneWire.h>

/* This could be bigger, but it's the IDE default, and doing anything at all
 * in the IDE is unpleasant, so... eh. */
#define BAUD 9600

/* Customize your wiring here if needed. */
typedef enum {
    /* Pins 0 and 1 mirror serial with USB. */
    ONEWIRE_PIN = 2,
    RELAY_ONE,
    RELAY_TWO,
} pin;

/* If you're choosing between the AM2315 and the SHT30, my preference was for
 * the SHT30, since the AM2315's humidity readings were consistently 10%
 * higher than all of: the SHT30, my humidifer, and my standalone
 * hygrometer.  Spec on all of these is ±2%, as it happens. */
Adafruit_AM2315 am2315;
bool am2315_found = false;

/* If you experience difficulty with the SHT30, note that its data and clock
 * wire colors are the reverse of the AM2315.  That is to say, WHITE is
 * data, while YELLOW is clock. */
Adafruit_SHT31 sht30; /* Yes, this is correct. */
bool sht30_found = false;

/* Why bother with this one if it doesn't have humidity?  Well, it has a
 * food-safe probe. */
#define DS18B20_MEASURE 0x44
#define DS18B20_READ 0xBE
uint8_t ds18b20_addr[8];
OneWire onewire(ONEWIRE_PIN);
bool ds18b20_found = false;

/* Sainsmart multi-channel relay board. */
bool relay1_off = false;
bool relay2_off = false;

/* Assumes temp sensor is already selected and ready.  I don't really see
 * much purpose in the checksuming, so I don't bother. */
static inline float ds18b20_get_temp() {
    uint8_t buf[9];
    int16_t temp_raw;

    /* Request measurement. */
    onewire.reset();
    onewire.select(ds18b20_addr);
    onewire.write(DS18B20_MEASURE); /* No parasite because why. */

    /* Read scratchpad. */
    onewire.reset();
    onewire.select(ds18b20_addr);
    onewire.write(DS18B20_READ);

    onewire.read_bytes(buf, sizeof(buf));

    /* High five bits of buf[1] are all sign... supposedly. */
    temp_raw = ((buf[1] & 0x87) << 8) | buf[0];

    return ((float)temp_raw) / 16;
}

/* "On" and "off" are wiring conventions.  "Off" is the state the relay is in
 * when the control pin is disconnected. */
static inline void relay_off(pin pin, bool *relay_off) {
    if (!*relay_off) {
        pinMode(pin, LOW);
        *relay_off = true;
    }
}

static inline void relay_on(pin pin, bool *relay_off) {
    if (*relay_off) {
        pinMode(pin, HIGH);
        *relay_off = false;
    }
}

void setup(void) {
    /* Relays start in ~unknown state - turn them off. */
    pinMode(RELAY_ONE, OUTPUT);
    relay_off(RELAY_ONE, &relay1_off);
    pinMode(RELAY_TWO, OUTPUT);
    relay_off(RELAY_TWO, &relay2_off);

    pinMode(LED_BUILTIN, OUTPUT);

    Serial.begin(BAUD);
    while (!Serial) { /* C++ is weeeeeeeird */
        delay(100);
    }

    Serial.println("Console connected");

    if (!onewire.search(ds18b20_addr)) {
        Serial.println("No 1wire devices found.");
    } else {
        ds18b20_found = true;
        Serial.println("Found DS18B20.");

        /* First temperature reading can be garbage. */
        (void)ds18b20_get_temp();
    }

    if (!am2315.begin()) {
        Serial.println("Did not find AM2315!");
    } else {
        am2315_found = true;
        Serial.println("Found AM2315.");
    }

    if (!sht30.begin()) {
        Serial.println("Couldn't find SHT30!");
    } else {
        sht30_found = true;
        Serial.println("Found SHT30.");
    }
}

void loop(void) {
    float t, h;

    /* I'm alive! */
    digitalWrite(LED_BUILTIN, HIGH);
    delay(2000);
    digitalWrite(LED_BUILTIN, LOW);
    delay(500);

    if (ds18b20_found) {
        t = ds18b20_get_temp();
        Serial.print("DS18B20 says: T: ");
        Serial.print(t);
        Serial.println("°C");
    }

    if (am2315_found) {
        if (!am2315.readTemperatureAndHumidity(&t, &h)) {
            Serial.print("AM2315 read failure?");
        } else {
            Serial.print("AM2315 says: ");
            Serial.print("T: ");
            Serial.print(t);
            Serial.print("°C, H: ");
            Serial.print(h);
            Serial.println("%");
        }
    }

    if (sht30_found) {
        t = sht30.readTemperature();
        h = sht30.readHumidity();
        if (t == NAN || h == NAN) {
            Serial.print("SHT30 read failure?");
        } else {
            Serial.print("SHT30 says: ");
            Serial.print("T: ");
            Serial.print(t);
            Serial.print("°C, H: ");
            Serial.print(h);
            Serial.println("%");
        }
    }
}

/* Local variables: */
/* c-basic-offset: 4 */
/* indent-tabs-mode: nil */
/* End: */
