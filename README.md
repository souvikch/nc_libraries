# nc_libraries

A shared NimbleController library project for portable NC-BUS core components, transport adapters, and device drivers. The `nc-lib-i2c-framework` branch is the current development branch for the I²C driver foundation and the NC-BUS architecture definition; it is not yet a complete or production-validated NC-BUS implementation.

## Design goals

- **NC-BUS is the system-wide logical bus** connecting sensors, devices, and nodes across local and network transports, including Ethernet.
- Keep NC-BUS core portable; ESP-IDF, Arduino, socket APIs, and hardware-specific code belong in adapters.
- Keep logical device identity independent from physical addresses and transport endpoints.
- Use typed properties and bounded messages/resources; avoid dynamic string-based dispatch in hot paths.
- Keep device register logic in device drivers and transport mechanics in transport components.
- Enforce finite timeouts, explicit error propagation, and synchronization for shared hardware.
- Test with fake transports and hardware smoke tests.

## Architecture contract

See **[docs/nc-bus-specification.md](docs/nc-bus-specification.md)** for the initial NC-BUS core contract, transport boundaries, identity, properties, message operations, errors, lifecycle, resource limits, and implementation sequence.

## Current driver foundation

The branch currently contains early components for:

1. **TCA9548A** — validate address, select one of eight channels, disable all channels.
2. **DS1307** — read/set date and time, check oscillator halt state.
3. **ADS1115** — single-shot conversion and millivolt reads for four single-ended channels, configurable PGA range and data rate.
4. **Generic I²C API** — validated write and combined write/read calls.
5. **ESP-IDF I²C adapter** — controller initialization, cached device handles, timeout and transport error mapping.
6. **Smoke-test application** — validates the API and attempts device operations when hardware is connected.

**Planned next device drivers:** PCF8575 GPIO expander (before ADS1115 integration where the application requires its GPIO state), followed by the selected SHT-series sensor.

## Target component boundaries

- `nc_common`: common types and error codes.
- `nc_property`: typed property descriptors and validation.
- `nc_bus`: portable NC-BUS logical device, property, routing, lifecycle, and message contract.
- `nc_i2c`: transport-level I²C transaction contract.
- `nc_i2c_espidf`: ESP-IDF I²C adapter.
- `nc_tca9548a`, `nc_ds1307`, `nc_pcf8575`, `nc_ads1115`, `nc_sht`: device-specific drivers.
- Future transport adapters: Ethernet/IP and other transports, added only after their interfaces and security requirements are defined.

The target layout is architectural intent; not every listed component is implemented yet. NC-BUS routing, remote-node messaging, Ethernet transport, and end-to-end driver registration remain unfinished.

## Build and test

Use the ESP-IDF 5.x environment configured for this project:

    idf.py set-target esp32
    idf.py build
    idf.py flash monitor

The smoke-test app defaults to SDA GPIO 21 and SCL GPIO 22 at 100 kHz. Confirm the actual board pinout and override `NC_TEST_I2C_SDA_GPIO` / `NC_TEST_I2C_SCL_GPIO` if needed. Internal pull-ups are enabled by default; suitable external pull-ups may be required.

A missing-device result is expected when a device is not connected. A successful compile is not proof of electrical reliability. Validate disconnected devices, bus faults, concurrent access, mux routing, and recovery on hardware.

## Integration boundary and readiness

The current I²C adapter targets the ESP-IDF I²C master driver and is not yet integrated with the actual NCCore I²C Bus Manager public API. NC-BUS is currently specified, not fully implemented. Atomic mux-routed transactions, synchronization, failure recovery, complete logical device registration, Ethernet transport, and hardware validation remain acceptance gates. Do not treat this branch as production-ready until these are completed and tested.
