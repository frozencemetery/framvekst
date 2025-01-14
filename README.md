# framvekst

Backing code for the fermentation chamber I'm building.  Work in progress;
more detailed documentation and setup explanation to follow at some point.
Probably.

Right now the system is set up to auto-detect sensors (DS18B20, SHT30, and/or
AM2315).  I'm dissatisfied with the AM2315 and may remove that code if it gets
in the way.  It also likes having a button.

The system starts up in "debug" configuration - logging sensors to serial and
not controlling temperature/humidity.  The button can be pressed to go to the
next mode (wait for double flash, then it will flash the current value of
mode).  Modes wrap around, and the system begins in debug.  See beginning of
source for more information.

# humid

Humidistat backed by a different arduino.  There's a fair amount of overlap in
the control logic between the two.

# heater

Like humid, but for heat.

# soil

Soil moisture experimentation.
