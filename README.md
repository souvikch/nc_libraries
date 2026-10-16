# nc_libraries

A single ESP-IDF project for NimbleController shared embedded libraries and device drivers. This branch implements the first hardware-driver slice; it is not yet the complete NC virtualized sensor/data bus.

## Design goals

- Small, bounded memory footprint; no per-device worker tasks or dynamic property maps on the transfer hot path.
- One generic, typed I2C transaction contract with a replaceable transport adapter.
- Device-specific register logic stays outside the bus driver.
- NC applications consume logical resources through the future NC virtual sensor/data bus, not GPIOs or physical addresses.
- Every driver exposes bounded timeouts and propagates transport errors.
- A hardware smoke-test application exercises devices when they are connected.

## Implemented components

1. **TCA9548A** — validate address, select one of eight channels, disable all channels.
2. **DS1307** — read/set date and time, check oscillator halt state.
3. **ADS1115** — single-shot conversion and millivolt read for each of four single-ended channels, configurable PGA range and data rate.
4. **Generic I2C API** — validated write and combined write/read calls.
5. **ESP-IDF 5.x adapter** — controller initialization, cached device handles, timeout and transport error mapping.
6. **Smoke-test app** — main/app_main.c checks bus validation and attempts real device operations.

## Architecture

- components/nc_common/: common error codes.
- components/nc_property/: typed property descriptors and validation.
- components/nc_device/: generic device contract.
- components/nc_i2c/: transport-neutral I2C contract.
- components/nc_i2c_espidf/: ESP-IDF transport adapter.
- components/nc_tca9548a/, nc_ds1307/, nc_ads1115/: device drivers.
- docs/architecture.md: layering and reliability requirements.

## Build and test

Use an ESP-IDF 5.x environment:

    idf.py set-target esp32
    idf.py build
    idf.py flash monitor

The smoke-test app defaults to SDA GPIO 21 and SCL GPIO 22 at 100 kHz. These are only common ESP32 defaults: confirm the actual board pinout and override NC_TEST_I2C_SDA_GPIO / NC_TEST_I2C_SCL_GPIO if needed. Internal pull-ups are enabled by default, but suitable external pull-ups may be required for reliable hardware.

A missing-device result is expected when a device is not connected. A successful compile is not proof of electrical reliability; test disconnected devices, bus faults, concurrent access, mux routing and recovery on hardware.

## Important integration boundary

The adapter currently targets ESP-IDF's I2C master driver. It is not yet bound to the actual NCCore I2C Bus Manager public API. The NC virtual sensor/data bus, resource registry, and atomic mux-routed target transactions must be integrated against the real NCCore runtime contract before this branch is considered production-ready. The exact SHT sensor model is also still to be selected.