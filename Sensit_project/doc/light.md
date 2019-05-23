# Light firmware

This sample code is a reproduction of the Sens'it Discovery's Light mode.


## What it does

With the Light firmware your Sens'it will make a brightness measurement every hour and send values in a Sigfox message using the Sens'it Discovery payload format.  
You can also force a new measurement and message with a double press on the button.

This firmware is compatible with the [sensit.io](https://www.sensit.io) application.

## How to Build

To build this firmware, use the `make light` command.

If you want to change the measurement and message sending period, you can edit the `MEASUREMENT_PERIOD` define.

## Sources

- [main_LIGHT.c](../sdk/src/main_LIGHT.c)
- [ltr329.c](../sdk/src/sensors/ltr329.c)
- [discovery.h](../sdk/resources/discovery.h)

## Authors

- Sens'it Team

## Copyright

- Copyright (c) 2018 Sigfox, All Rights Reserved.
