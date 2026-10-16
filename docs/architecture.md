# Architecture and design decisions

## Implementation order
1. TCA9548A I2C multiplexer.
2. DS1307 real-time clock.
3. ADS1115 ADC.
4. Add the selected SHT-series temperature/humidity sensor once the exact model is confirmed.

## Layering
- Application and NC virtual sensor/data bus: logical resources, units, timestamps, quality and status. It must not expose GPIO pins or physical I2C addresses to application logic.
- Device/property layer: typed configuration and values, capabilities, lifecycle, validation and error reporting.
- Generic I2C API: address-based read/write and combined write/read transactions with bounded timeout.
- Transport adapter: ESP-IDF I2C controller setup, driver handles, transaction serialization and error translation.
- Device drivers: register semantics and sensor-specific conversion only.

## Generic bus plus typed properties
The generic bus is the correct shared abstraction. Keep its hot path small and typed; use property descriptors for introspection/configuration, not a dynamic string map for every transfer. Bus properties include clock speed and timeout policy. Device properties include device address, conversion rate, range, calibration, and readings. Separate read-only measured values from writable configuration.

## Reliability rules
- One shared bus instance per physical controller.
- Never initialize the same physical bus from individual device drivers.
- Bound all waits and propagate transport errors.
- Validate addresses, enum values and data ranges before I/O.
- Serialize each physical transaction in the adapter.
- For a muxed device, mux channel selection and the following device transaction must be atomic as a group. Do not permit another task to change the mux between those operations.
- No unbounded allocation or per-device worker task by default.
- Hardware tests must cover disconnected devices, bad addresses, concurrent access and recovery behavior.

## Current implementation boundary
The generic I2C contract, ESP-IDF master adapter, TCA9548A control, DS1307 RTC operations, ADS1115 single-shot conversion, and smoke-test app are in this branch. Build and hardware validation are still required. A unified NC virtual sensor bus/registry is an integration layer and must be connected to the actual NCCore runtime contract rather than invented independently in this component repository.
