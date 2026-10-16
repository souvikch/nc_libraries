# NC-BUS and Device Library Architecture

**Status:** Draft for implementation  
**Aligned with:** `souvikch/nimble-runtime` architecture on `main`

## Architectural authority

The NimbleController Runtime documentation is authoritative for Runtime responsibilities. This repository defines reusable controller-side contracts, transport adapters, and device drivers; it must not introduce a competing Digital Twin, resource allocator, Virtual Resource Registry, provisioning system, or automation policy engine.

The Runtime owns topology, policy, constraints, resource allocation, provisioning, and the Digital Twin. The controller executes validated resources and reports operational state and telemetry.

## Layering

```text
NimbleController Runtime
  Digital Twin / provisioning / resource allocation / policy
                         |
                existing Runtime contract
                         |
Controller Runtime Integration
  applies validated model / commands / telemetry
                         |
Virtual Resource Engine and controller registry
  application-facing logical resources and capabilities
                         |
NC-BUS (portable logical device/data contract)
                         |
Transport and Bus Managers
  I2C / SPI / UART / GPIO / Ethernet-IP / future
                         |
Device Drivers
  TCA9548A / DS1307 / PCF8575 / ADS1115 / SHT
                         |
                    Hardware
```

The diagram expresses responsibilities, not a requirement that every local register operation traverse every layer. Local drivers can make bounded synchronous calls through a bus manager; cross-node communication uses a defined network/runtime contract.

## Component ownership

- `nc_common`: portable primitive types and normalized error foundation.
- `nc_property`: typed property descriptors and values; avoid string-based hot-path dispatch.
- `nc_bus`: portable logical operation, endpoint, identity, dispatch and status contract. It is not the Runtime's resource registry.
- `nc_device`: existing driver-facing device API. Its overlap with logical device descriptors must be resolved before NC-BUS ABI is frozen.
- `nc_i2c`: low-level local I2C transaction contract.
- `nc_i2c_espidf`: ESP-IDF implementation of the I2C contract, including physical bus ownership, serialization and transport error mapping.
- Device components: one driver per physical device type; no direct peer-driver calls.
- Future Ethernet adapter: only after the existing Runtime MQTT/node contracts, framing, identity, and security are reviewed.

## Device implementation sequence

1. TCA9548A mux and routing safety.
2. DS1307 RTC.
3. PCF8575 GPIO expander.
4. ADS1115 ADC, including explicit configured sequencing if PCF8575-controlled signals are prerequisites.
5. Selected SHT-series device after exact part number is confirmed.

PCF8575 belongs before ADS1115 integration in the project sequence. The driver itself must not hard-code the dependency; express it in the hardware profile/configuration or a coordination layer after the actual signal relationship is confirmed.

## Rules

- Application logic uses Virtual Resources, never GPIO numbers, I2C addresses, or socket details.
- Platform Profile defines fixed controller hardware and bus topology; Device Profile defines provisioned devices and logical/resource mappings.
- Runtime resource allocation/policy remains out of this library.
- A logical ID is not a transport address.
- All blocking operations have finite deadlines and bounded inputs.
- Mux channel selection and downstream transaction are one synchronized critical section.
- Preserve last valid property values on read failure; model availability and measurement separately.
- Avoid unbounded allocation in steady-state and one task per device by default.
- A build/hardware-ready status requires actual compilation, concurrency/fault tests, and hardware validation.

## Canonical specification

See [NC-BUS Core Architecture and Contract](nc-bus-specification.md).