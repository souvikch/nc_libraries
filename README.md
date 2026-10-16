# nc_libraries

A single ESP-IDF project for NimbleController shared libraries and hardware-device components.

## Goals

- Separate bus management from device-specific logic.
- Expose hardware through generic device/property interfaces.
- Keep application logic independent of GPIO and device register details.
- Use one shared I2C layer for all I2C device components.

## Implementation order

1. **TCA9548A** I2C multiplexer — establish channel selection and bus routing.
2. **DS1307** RTC — implement clock access over the shared I2C layer.
3. **ADS1115** ADC — implement analog measurements over the shared I2C layer.

## Structure

- `main/`: application entry point and project integration.
- `components/nc_common/`: shared status/error definitions.
- `components/nc_property/`: typed property descriptors and validation.
- `components/nc_device/`: generic device contract.
- `components/nc_i2c/`: provisional I2C transport boundary.
- `components/nc_tca9548a/`: TCA9548A multiplexer scaffold.
- `components/nc_ds1307/`: DS1307 RTC scaffold.
- `components/nc_ads1115/`: ADS1115 ADC scaffold.
- `docs/`: architecture and next steps.
- `test/`: test planning.

## Build

In an ESP-IDF terminal:

```sh
idf.py set-target esp32
idf.py build
```

This branch establishes the project and initial component interfaces. Hardware transactions are not implemented yet. The I2C adapter must first be integrated with the actual NCCore I2C Bus Manager API.
