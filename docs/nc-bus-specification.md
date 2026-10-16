# NC-BUS Core Architecture and Contract

**Status:** Initial architecture contract  
**Scope:** Portable logical bus for NimbleController  
**Target repository:** `souvikch/nc_libraries`

## 1. Purpose

NC-BUS is the system-wide communication and data bus connecting sensors, devices, and controller nodes across local and network transports. It provides a consistent logical model without requiring every transport to have the same physical capabilities.

NC-BUS is not synonymous with I²C, Ethernet, or a sensor registry. It includes a portable core and pluggable transport adapters.

## 2. Architectural layers

1. **NC-BUS core (portable):** identities, endpoints, message envelopes, routing decisions, capabilities, lifecycle, error model, and bounded resource policies.
2. **Data/property model (portable):** typed properties, units, access rights, validity, timestamps, and optional quality/status.
3. **Transport adapters:** move messages or execute local transactions over a specific medium. Initial transport: I²C. Network transport: Ethernet/IP, introduced after message framing and security requirements are agreed.
4. **Device drivers:** implement device-specific register/protocol behavior, e.g. TCA9548A, DS1307, PCF8575, ADS1115, and SHT-series sensors.
5. **Node integration:** exposes local devices and exchanges logical device data with remote nodes.
6. **NimbleController integration:** consumes NC-BUS device/property APIs for automation, management, and application logic.

Dependencies point inward: drivers and transport adapters depend on core interfaces; the core must not depend on ESP-IDF, Arduino, sockets, or a specific hardware driver.

## 3. Core concepts

### 3.1 Identity and addressing

- Every logical node and device has a stable logical identifier that is independent of transport addresses.
- A transport endpoint is separate metadata: for example I²C controller + address + mux route, or Ethernet interface + network endpoint.
- Addresses are not globally unique device identities. An I²C address may be reused on separate mux channels.
- Routing resolves a logical destination to one or more configured endpoints. The core must not assume that every endpoint is directly reachable.

### 3.2 Device descriptor

A device descriptor should carry:
- stable device ID and owning node ID;
- device type / driver identifier and schema version;
- supported capabilities;
- configured endpoint(s);
- lifecycle/availability state;
- optional human-readable label, kept out of hot-path lookup keys.

Device registration must validate required fields and reject duplicate IDs. Registration does not itself guarantee that the physical device is reachable.

### 3.3 Properties and values

A property descriptor defines:
- stable property ID within the device;
- value type (boolean, signed/unsigned integer, float, string/bytes only where needed, or structured type with an explicit schema);
- unit and scale where applicable;
- read/write permissions;
- constraints/range where applicable;
- optional sampling/update behavior.

A property value carries the typed value, status/quality, and timestamp where meaningful. Failed reads must not silently overwrite a previously valid value with a fabricated default. String names are for configuration and diagnostics; runtime operations should use numeric/enumerated IDs or pre-resolved handles.

### 3.4 Messages and operations

The logical API must support at minimum:
- read property;
- write property (where allowed);
- invoke a device operation;
- publish value/state change;
- report device/transport status.

Operations have a destination device ID, operation/property ID, bounded payload, request/correlation ID when request-response is used, deadline/timeout, and a typed result/error. The initial implementation may use in-process calls for local devices; the logical contract must not bake in that implementation detail.

### 3.5 Capabilities

Capabilities describe supported operations and constraints, not merely device type. Consumers must check capability/permissions before writes or method invocation. Capability discovery should be bounded and cacheable.

## 4. Transport contract

A transport adapter is responsible for:
- initialization and shutdown;
- validating transport-specific endpoint configuration;
- sending/receiving bounded frames or executing bounded local operations;
- mapping transport errors to NC-BUS error categories;
- reporting link/endpoint state;
- enforcing deadlines and transport-specific limits.

A transport adapter must not own application policy or redefine logical device identity. The core must not assume reliable delivery, ordering, broadcast support, or equivalent security across transports.

### Initial I²C mapping

I²C is a local peripheral transport, not a general remote-node protocol. Its endpoint metadata includes controller/bus identity, 7-bit device address, speed/configuration, and optional routing path such as a TCA9548A channel.

For muxed I²C, mux selection and the dependent downstream transaction must be serialized as one critical section. Separate calls that permit another task to change the mux between selection and transfer are unsafe. Timeouts and transfer lengths must be bounded.

The PCF8575 is an I²C GPIO-expander driver. If its GPIO state must be set before ADS1115 sampling, that dependency belongs in an explicit device configuration or coordination/sequence layer; the generic I²C transport and NC-BUS core must not hard-code it.

### Ethernet mapping

Ethernet/IP is a network transport and may connect nodes. Before implementation, define framing/versioning, maximum frame size, node authentication, authorization, replay handling where relevant, connection lifecycle, and compatibility behavior. Do not expose an unauthenticated network write path to device operations.

## 5. Error model

Use stable typed error categories rather than parsing strings:
- invalid argument/configuration;
- not initialized / invalid lifecycle state;
- not found / duplicate identity;
- unsupported operation / permission denied;
- timeout / deadline exceeded;
- transport unavailable / link down;
- protocol error / malformed payload;
- device absent / device-level failure;
- resource exhausted;
- internal failure.

Preserve the underlying transport or device error as diagnostic detail without requiring consumers to depend on vendor-specific error codes.

## 6. Reliability and resource rules

- Every potentially blocking operation has a finite timeout or deadline.
- Every message/frame has a documented maximum size.
- Queue depths, device counts, endpoint counts, and caches have explicit bounds.
- No unbounded heap allocation in high-frequency read/write paths; prefer caller-owned storage, static allocation, or bounded pools.
- Shared hardware resources must be synchronized. Lock ownership and lock ordering must be documented.
- Retries are policy-controlled and only automatic when the operation is safe to retry. Do not blindly retry non-idempotent writes.
- Device availability is distinct from the last known property value.
- Diagnostics must be optional and rate-limited so they cannot exhaust memory or flood logs.

## 7. Lifecycle

Minimum device states:
- unregistered;
- registered;
- initializing;
- ready;
- degraded/unavailable;
- stopping.

State transitions must be explicit. A transient transport error should not silently destroy registration. Recovery/reinitialization is performed by the owner of the device/transport lifecycle.

## 8. Repository component boundaries

Target component layout:

```text
components/
  nc_common/                 # shared error and primitive types
  nc_property/               # typed property descriptors/values
  nc_bus/                    # portable logical bus contract and registry
  nc_bus_transport_i2c/      # NC-BUS adapter for local I²C endpoints (when needed)
  nc_bus_transport_ethernet/ # network adapter (later)
  nc_i2c/                    # transport-level I²C transaction interface
  nc_i2c_espidf/              # ESP-IDF implementation of nc_i2c
  nc_tca9548a/               # I²C mux driver
  nc_ds1307/                 # RTC driver
  nc_pcf8575/                # GPIO expander driver
  nc_ads1115/                # ADC driver
  nc_sht/                    # selected SHT-series driver
```

The names `nc_i2c` and `nc_i2c_espidf` describe the hardware transaction layer. They are not replacements for NC-BUS. Do not create placeholder transport components until their public interfaces are defined.

## 9. Current implementation mapping

The existing I²C driver framework is a starting point, not a complete NC-BUS implementation:
- `nc_common`, `nc_property`, and `nc_device` are early foundations.
- `nc_i2c` and `nc_i2c_espidf` provide the intended transport boundary.
- `nc_tca9548a`, `nc_ds1307`, and `nc_ads1115` are device-driver work.
- `nc_pcf8575` and `nc_sht` remain to be added.
- NC-BUS logical routing, remote-node messaging, Ethernet transport, and end-to-end registration are not yet complete.

## 10. Implementation sequence and acceptance gates

1. Review and freeze public types/error codes in `nc_common`.
2. Define typed property descriptors and values.
3. Implement a portable `nc_bus` registry and local read/write/invoke contract with bounded storage.
4. Add unit tests using fake endpoints/transports; test duplicate IDs, invalid descriptors, permissions, timeouts, and error propagation.
5. Finalize I²C synchronization, including atomic mux selection plus downstream transaction.
6. Register existing drivers as logical devices through a thin integration layer rather than coupling drivers directly to the bus core.
7. Add PCF8575 and express its relationship to ADS1115 in configuration/coordination if the hardware requires it.
8. Add network framing/security design before Ethernet implementation.
9. Build against the project's actual ESP-IDF version and run hardware tests. Do not label the framework production-ready until build, concurrency, failure-recovery, and hardware tests pass.

## 11. Non-goals for the first core iteration

- No assumption that NC-BUS is a single wire/protocol.
- No dynamic string-based dispatch in the hot path.
- No automatic discovery protocol until identity, trust, and resource limits are specified.
- No claim of end-to-end delivery guarantees across transports until each adapter documents them.
- No assumption that every device supports remote access, writes, events, or streaming.
