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
    SERIAL_TX = 0,
    SERIAL_RX = 1,

    ONEWIRE_PIN = 2,
    TEMP_RELAY = 3,
    HUMID_RELAY = 4,
    MODE_BUTTON = 5,

    /* LED_BUILTIN = 13, */

    SDA_PIN = 18,
    SCL_PIN = 19,
} pin;

#define COFF(a) (((float)(a) - 32) * 5) / 9

/* Careful with these.  There needs to be defined band for the system to coast
 * between.  While this is all floating point, sensor precision is a concern:
 * the DS18B20 claims ±0.5C, the SHT30 ±0.2C, and the AM2315 ±0.1C. */
struct {
    float low;
    float high;
    bool debug;
} modes[] = {
    /* Be verbose - Arduino toolchain lacks full designated initializers. */
    { /* start in debug */
        .low = NAN,
        .high = NAN,
        .debug = true,
    },
    { /* sourdough bread */
        .low = COFF(72),
        .high = COFF(75),
        .debug = false,
    },
    { /* vegan yogurt */
        .low = COFF(107),
        .high = COFF(110),
        .debug = false,
    },
};
uint8_t mode = 0; /* incremented by button push */
float low;
float high;
bool debug;

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

bool temp_relay_off = true;
bool humid_relay_off = true;

static inline float fofc(float celsius) {
    return (9 * celsius) / 5 + 32;
}

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
        digitalWrite(pin, LOW);
        *relay_off = true;
    }
}

static inline void relay_on(pin pin, bool *relay_off) {
    if (*relay_off) {
        digitalWrite(pin, HIGH);
        *relay_off = false;
    }
}

void setup(void) {
    pinMode(TEMP_RELAY, OUTPUT);
    pinMode(HUMID_RELAY, OUTPUT);

    pinMode(MODE_BUTTON, INPUT_PULLUP);

    pinMode(LED_BUILTIN, OUTPUT);

    Serial.begin(BAUD);
    while (!Serial) {
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

    if (!sht30_found && !am2315_found) {
        Serial.println("No humidistats found - disabling humidity control");

        if (!ds18b20_found) {
            Serial.println("No themostats either?  Life is meaningless");
            while (1) {
                delay(1000);
            }
        }
    }

    low = modes[mode].low;
    high = modes[mode].high;
    debug = modes[mode].debug;
}

/* Get the best possible readings out of the system, but don't be too picky. */
static inline void take_readings(float *t_out, float *h_out, float *t_probe_out) {
    *t_out = *h_out = *t_probe_out = NAN;

    if (sht30_found) {
        *t_out = sht30.readTemperature();
        *h_out = sht30.readHumidity();
    }
    if (am2315_found && (isnan(*t_out) || isnan(*h_out))) {
        if (!am2315.readTemperatureAndHumidity(t_out, h_out)) {
            /* Set them to known-failed values, and allow for the ds18b20. */
            *t_out = *h_out = NAN;
        }
    }

    if (ds18b20_found) {
        *t_probe_out = ds18b20_get_temp();
        if (isnan(*t_out)) {
            *t_out = *t_probe_out;
        }
    }
}

static inline void log_readings(float t, float h, float t_probe) {
    if (!debug) {
        return;
    }

    Serial.print("Read: ");
    Serial.print(fofc(t));
    Serial.print("°F (probe ");
    Serial.print(fofc(t_probe));
    Serial.print("°F), ");
    Serial.print(h);
    Serial.println("%RH");
}

static inline void flash(void) {
    digitalWrite(LED_BUILTIN, LOW);
    delay(100);
    digitalWrite(LED_BUILTIN, HIGH);
    delay(100);
    digitalWrite(LED_BUILTIN, LOW);
}

void loop(void) {
    float t, h, t_probe;

    if (digitalRead(MODE_BUTTON) == LOW) {
        flash();
        flash();
        mode++;
        if (mode >= sizeof(modes) / sizeof(*modes)) {
            mode = 0;
        }
        Serial.print("mode: ");
        Serial.println(mode);
        low = modes[mode].low;
        high = modes[mode].high;
        debug = modes[mode].debug;
        delay(500);
        for (unsigned char i = 0; i < mode; i++) {
            flash();
        }
        return; /* continue */
    }

    digitalWrite(LED_BUILTIN, LOW);
    delay(500);

    take_readings(&t, &h, &t_probe);
    log_readings(t, h, t_probe);
    if (!isnan(t)) {
        if (t < low && temp_relay_off) {
            relay_on(TEMP_RELAY, &temp_relay_off);
        } else if (t > high && !temp_relay_off) {
            relay_off(TEMP_RELAY, &temp_relay_off);
        }
    } else {
        digitalWrite(LED_BUILTIN, HIGH);
    }

    delay(2000); /* Need >2s between loop iterations for sensors. */
}

/* Local variables: */
/* c-basic-offset: 4 */
/* indent-tabs-mode: nil */
/* End: */
