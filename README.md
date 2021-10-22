# framvekst

Backing code for the fermentation chamber I'm building.  Work in progress;
more detailed documentation and setup explanation to follow at some point.
Probably.

Right now the system is set up to auto-detect sensors (DS18B20, SHT30, and/or
AM2315).  I'm dissatisfied with the AM2315 and may remove that code if it gets
in the way.  It's configured in "nut milk yogurt" configuration - humidity
falls where it will, while temperature is kept in a specific range.  The
humidity stuff is all wired up, just not doing anything.

## Setup

This is an Arduino project.  The Arduino IDE has never worked well for me, so
I try to use it as little as possible.

It depends on the
[OneWire](https://github.com/PaulStoffregen/OneWire/releases),
[Adafruit_SHT31](https://github.com/adafruit/Adafruit_SHT31/releases), and
[Adafruit_AM2315](https://github.com/adafruit/Adafruit_AM2315/releases)
libraries.  Those last two in turn depend on
[Adafruit_BusIO](https://github.com/adafruit/Adafruit_BusIO/releases).
Download the archives and unpack them into their own directories in
~/Arduino/libraries.

Symlink the sketch into ~/Arduino/sketch (or rename and symlink elsewhere).
Then open it in the Arduino IDE, and if everything goes right, you can build,
upload, and inspect serial spew.  And if it doesn't, then neither of us has
any idea what's gone wrong.  Isn't embedded fun?
