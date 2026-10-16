# nc_libraries

Reusable NimbleController controller-side libraries for NC-BUS contracts, transport adapters, and device drivers.

**Current development branch:** `nc-lib-i2c-framework`  
**Status:** Architecture and I2C driver foundations; NC-BUS is not yet fully implemented or production-validated.

## Architectural intent

NC-BUS is the system-wide logical bus/data contract for sensors, devices, and controller nodes across multiple transports, including Ethernet. Its portable core defines logical identities, operations, typed values/status, dispatch interfaces and bounded behavior without depending on ESP-IDF or a specific transport.

This library does not replace the NimbleController Runtime. The Runtime owns the authoritative Digital Twin, topology, policy, provisioning, constraints and Virtual Resource allocation. The controller executes the validated Runtime Model through its Virtual Resource Engine and driver stack. Applications must not access physical hardware directly.

See:
- [NC-BUS Core Architecture and Contract](docs/nc-bus-specification.md)
- [Architecture overview](docs/architecture.md)
- [Implementation and validation checklist](docs/next-steps.md)

## Existing implementation foundation

Components currently present include `nc_common`, `nc_property`, `nc_device`, `nc_i2c`, `nc_i2c_espidf`, `nc_tca9548a`, `nc_ds1307`, and `nc_ads1115`, plus a hardware smoke-test application.

They are foundational and have not yet been verified by a target ESP-IDF build or complete hardware validation. The current branch should not be treated as production-ready.

## Target boundaries

- `nc_common`: shared portable types and errors.
- `nc_property`: typed property descriptors and values.
- `nc_bus`: portable NC-BUS logical operation/data contract (to implement).
- `nc_device`: current device-driver interface; overlap with logical descriptors requires reconciliation.
- `nc_i2c`: local physical I2C transaction interface.
- `nc_i2c_espidf`: ESP-IDF I2C adapter.
- Device drivers: `nc_tca9548a`, `nc_ds1307`, `nc_pcf8575` (to add), `nc_ads1115`, and the confirmed SHT-series model (to add).
- Network adapters: designed only after existing Runtime/node contracts and security requirements are reviewed.

## Build and validation

Follow the project-specific ESP-IDF setup and [validation checklist](docs/next-steps.md). A successful compile does not establish electrical reliability or production readiness. Test absent devices, invalid endpoints, concurrency, mux routing, timeout/recovery, memory use, and configured PCF8575-to-ADS1115 sequencing on hardware.