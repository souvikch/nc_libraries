# NC-BUS Core Architecture and Contract

**Document ID:** NC-LIB-BUS-001  
**Version:** 0.2 (architecture aligned with nimble-runtime)  
**Status:** Draft for implementation  
**Repository:** `souvikch/nc_libraries`  
**Development branch:** `nc-lib-i2c-framework`

## 1. Purpose and scope

NC-BUS is NimbleController's system-wide logical bus connecting sensors, devices, and controller nodes over multiple communication protocols, including Ethernet. It provides stable logical identities and common device/data operations while preserving the constraints and failure semantics of each transport.

NC-BUS is not just an I²C wrapper, a sensor registry, or the Runtime's Virtual Resource Registry. It is the portable controller-side bus contract and communication/data abstraction. The NimbleController Runtime remains authoritative for the system Digital Twin, topology, resource allocation, constraints, and provisioning. The controller executes the validated model delivered to it.

This specification is aligned to the `nimble-runtime` architecture documents, especially:
- `docs/architecture/System_Architecture.md`
- `docs/architecture/Hardware_Abstraction_Architecture.md`
- `docs/architecture/Firmware_Architecture.md`
- `docs/architecture/Runtime Architecture.md`
- `docs/architecture/Part III — Platform Architecture.md`
- `docs/architecture/Part V — Runtime Models.md`

The Runtime repository is authoritative for Runtime architecture. This document specifies only the contracts needed inside the library/controller domain and does not redefine Runtime responsibilities.

## 2. Architectural rules derived from nimble-runtime

1. Application and automation code must not access hardware directly.
2. Applications consume Virtual Resources; they do not address physical devices, GPIO numbers, I²C addresses, or network sockets.
3. Every physical device has a driver; bus managers implement communication infrastructure and never device-specific behavior.
4. Device registration, capability registration, and virtual-resource registration remain distinct responsibilities. NC-BUS must not become a second Runtime Registry or allocate Virtual Resources.
5. Runtime owns policy, topology, constraints, provisioning, resource allocation, and the Digital Twin. A controller executes the validated model and reports state/telemetry.
6. Platform Profile describes fixed controller infrastructure and bus topology; Device Profile describes provisioned devices and their logical/resource mappings.
7. Components need explicit ownership, lifecycle, state and relationships. No hidden/circular dependencies.
8. Event-oriented communication is preferred across services, but direct calls are acceptable for bounded local driver transactions.
9. Keep ESP-IDF, FreeRTOS, Arduino, sockets, and hardware-specific symbols out of the portable NC-BUS core.
10. Use bounded resources, finite deadlines, centralized error semantics, and observable health/status.

## 3. Layered model

```text
NimbleController Runtime
  - Digital Twin, topology, policy, compilation, resource allocation
  - Sends validated Runtime Model / provisioning to controller
                  |
         MQTT / defined network contract
                  |
Controller Runtime Integration
  - applies compiled resources and commands
  - reports runtime state and telemetry
                  |
Virtual Resource Engine / Controller Runtime Registry
  - logical application-facing resources, capabilities, current state
                  |
NC-BUS Core (portable)
  - local/remote logical device access contract
  - stable IDs, operations, value/status envelope, dispatch interfaces
                  |
NC-BUS Transport and Bus Managers
  - I²C / SPI / UART / GPIO / Ethernet-IP / future transports
                  |
Device Drivers / protocol endpoints
  - TCA9548A, DS1307, PCF8575, ADS1115, SHT-series, etc.
                  |
             Physical hardware
```

This is a responsibility view, not a requirement that all traffic traverse each layer synchronously. Local sensor reads may be direct bounded calls; remote-node interactions require a defined network message path.

### 3.1 NC-BUS Core

Portable C library, independent of ESP-IDF and any specific transport. Defines:
- logical node and device identifiers;
- operation and property identifiers;
- request/result structures;
- capability and permission checks;
- bounded payload/value representation;
- deadline and correlation semantics where request/response applies;
- common status/error categories;
- routing/dispatch interfaces that operate on configured endpoint records;
- device availability and lifecycle events or callbacks.

It does not own system policy, compile profiles, allocate application resources, manage Runtime services, or implement a physical protocol.

### 3.2 Virtual Resource Engine and Runtime Registry

These are controller/runtime concepts from nimble-runtime. They map validated logical resources and capabilities to driver operations. NC-BUS provides the device/transport operation interfaces they can use; it does not replace the authoritative registry or create a second virtual-resource allocation system.

### 3.3 Transport and bus managers

Transport implementations own connection/device handles, bus initialization, arbitration/serialization, finite timeouts, transport recovery, and transport-specific errors. They expose transport-specific endpoints to NC-BUS through narrow adapters.

### 3.4 Device drivers

Each driver owns the register-level semantics, initialization, data conversion, capability description, and driver-specific failure reporting for one hardware device type. Drivers must not hard-code application logic or directly call peer drivers. Shared hardware interaction goes through the bus manager; higher-level dependencies are declared in configuration or an orchestration/sequence component.

## 4. Identity, endpoints and topology

Logical identity must not be inferred from a physical address.

- **Node ID:** stable controller/node identity assigned by the established NimbleController provisioning/runtime contract.
- **Device ID:** stable per-device identity within the configured system/controller namespace. The final canonical format must match the existing provisioning/runtime schemas before ABI freeze.
- **Capability ID:** capability exposed by a device/driver.
- **Property ID / Operation ID:** stable numeric or enum identifiers for hot-path operations; human-readable names belong in configuration and diagnostics.
- **Endpoint:** transport-specific connection details.
  - I²C: controller/bus identifier, 7-bit address, speed/policy, optional mux route.
  - Ethernet: network-interface and authenticated peer/endpoint metadata, with framing defined by a separate network contract.
  - SPI/UART/GPIO and later protocols: their own typed endpoint data.
- **Topology:** Platform Profile defines fixed buses and integrated devices; Device Profile defines provisioned endpoints and mappings. NC-BUS consumes validated topology; Runtime remains its authority.

The same I²C address may exist behind different TCA9548A channels. The I²C address is endpoint data, not a global ID.

## 5. Data and operation contract

Initial logical operations:
- read property/value;
- write a writable property;
- invoke a declared device operation;
- publish value/state update;
- report device/endpoint health and availability.

Each request must have a bounded operation/payload size and finite deadline. Request/response operations use a correlation ID where needed. Results carry a typed outcome and normalized error. Fire-and-forget publication must not be represented as confirmed remote execution.

Property descriptors should define:
- numeric/enum property ID and schema/version;
- value type and unit/scale;
- read/write/command permissions;
- allowable range/constraints where applicable;
- sampling/update behavior where applicable.

Values should separate:
- value bytes/type;
- validity and quality/status;
- capture/update timestamp if available;
- optional sequence/version metadata.

A communication or conversion failure must not silently replace the last valid reading with a fake zero/default. Availability and last known measurement are separate states.

Prefer fixed-size values or caller-owned buffers in hot paths. Strings/bytes are supported only with explicit maximum lengths and ownership rules. Do not use dynamically allocated string-key maps for every read.

## 6. Transport contract

Each transport adapter shall:
- have an explicit owner and initialize/stop lifecycle;
- validate endpoint configuration;
- execute bounded transactions or bounded frames;
- apply finite timeout/deadline behavior;
- serialize shared physical resources;
- translate transport errors into NC-BUS categories while preserving diagnostic detail;
- expose link/endpoint health;
- document ordering, delivery, retry, broadcast and security properties.

The core must not assume all transports provide identical reliability or security. Retry policy belongs to the caller/orchestrator and must consider whether the operation is idempotent.

### 6.1 I²C and multiplexers

I²C is local peripheral access; it is not a general node-network protocol. `nc_i2c` is a low-level I²C transaction interface; `nc_i2c_espidf` is its ESP-IDF implementation. These components must not be renamed as NC-BUS or embedded into the core.

For TCA9548A routing, select-channel plus the dependent downstream transfer must be atomic under one shared lock. Independent select and transfer calls are not sufficient if other tasks can interleave. The lock owner should be the I²C/mux routing layer, with documented lock ordering and bounded waits.

### 6.2 PCF8575 and ADS1115 dependency

PCF8575 is an I²C GPIO expander driver, not a transport. If the board requires PCF8575 GPIO state (for example enable, reset, muxing, or signal conditioning) before ADS1115 conversion, express that relationship in the Platform/Device Profile or a higher-level sequence/coordination layer. Do not hard-code a PCF8575 dependency into the generic NC-BUS or I²C layer. The exact physical signal dependency still needs to be confirmed from the board design.

### 6.3 Ethernet and remote nodes

Ethernet is a physical/network link; TCP/IP and the chosen application framing provide the communication semantics. The NC-BUS Ethernet adapter should transport defined node messages and must not become an unauthenticated raw device-write interface.

Before implementation, separately specify:
- frame/message version and maximum size;
- node/device identity binding to authenticated peers;
- authentication and authorization;
- integrity and replay protections where relevant;
- connection/disconnect lifecycle and liveness;
- deadlines, acknowledgements, duplicate handling and retry rules;
- protocol compatibility/version negotiation;
- error mapping and rate-limited diagnostics.

Do not invent or duplicate the existing Runtime MQTT contract. If the existing Runtime message transport already carries node operations, use that contract through an adapter; only add direct Ethernet transport if there is a distinct requirement for it.

## 7. Errors, lifecycle and state

Normalized error categories must cover at least:
- invalid argument/configuration;
- invalid lifecycle state/not initialized;
- not found/duplicate logical ID;
- unsupported operation/permission denied;
- timeout/deadline exceeded;
- transport unavailable/link down;
- malformed protocol/payload;
- device absent/device-level fault;
- resource exhausted;
- internal failure.

The adapter may retain native/vendor error information as diagnostic detail.

Minimum device/endpoint lifecycle: `UNREGISTERED → REGISTERED → INITIALIZING → READY`, with transitions to `DEGRADED/UNAVAILABLE`, `STOPPING`, and `RETIRED` as appropriate. The final enum must be reconciled with existing NCCore/runtime contracts before ABI freeze. A transient bus failure does not erase logical registration. Lifecycle ownership must be singular.

## 8. Memory, concurrency and observability

- Define maximum device, endpoint, descriptor, and in-flight request counts.
- Avoid unbounded allocation in steady state.
- All blocking operations have finite deadlines.
- Synchronize every shared physical bus/endpoint.
- Document ownership, lifetime, and copying rules for buffers and descriptors.
- Do not spawn one task per device by default.
- Use bounded queues and optional/rate-limited diagnostics.
- Preserve last known valid values separately from endpoint health.
- Provide counters for successes, errors, timeouts, recoveries, and current availability where appropriate.
- Never claim production readiness until target builds, concurrency tests, fault-injection tests, and relevant hardware tests pass.

## 9. Proposed repository component boundaries

```text
components/
  nc_common/                  shared primitive/error types
  nc_property/                typed property descriptors/values
  nc_bus/                     portable NC-BUS contract and bounded dispatcher
  nc_device/                  driver-facing device contract (reconcile with nc_bus)
  nc_i2c/                     transport-level I²C transaction contract
  nc_i2c_espidf/              ESP-IDF implementation of nc_i2c
  nc_tca9548a/                 I²C mux driver/routing support
  nc_ds1307/                  RTC driver
  nc_pcf8575/                 GPIO-expander driver (to add)
  nc_ads1115/                 ADC driver
  nc_sht/                     selected SHT driver (exact part to confirm)
  nc_bus_transport_ethernet/  only after network contract is approved
```

Do not create placeholder adapters until the exact interface has a use case and tests. The existing `nc_device` component overlaps conceptually with logical device descriptors; resolve that overlap before implementing a second registry or adding more parallel device structures.

## 10. Runtime models required for the NC-BUS object specification

Every major NC-BUS object must document all six nimble-runtime models:
1. **Hierarchy:** where it resides and what it contains;
2. **Ownership:** the single lifecycle owner;
3. **Relationships:** uses/depends-on/creates/publishes/consumes/binds;
4. **Lifecycle:** allowed state transitions;
5. **State:** immutable definition versus mutable runtime status/value;
6. **Interaction:** calls, events and message flow.

At minimum, apply this template to: NC-BUS core, transport adapter, endpoint, device descriptor, property, operation request/result, route, and lifecycle/status event.

## 11. Integration sequence and acceptance gates

1. **Architecture reconciliation:** compare these component APIs to the actual NCCore Bus Managers, Device Registry, Virtual Resource Engine, Platform Profile, Device Profile and remote-node contracts. Resolve names/ownership/ID formats before public ABI freeze.
2. **Public primitive/error API:** expand common errors only after mapping them to the NCCore standardized error taxonomy.
3. **Property contract:** use stable numeric/enum IDs and typed value/status; preserve bounded memory and explicit ownership.
4. **NC-BUS core:** implement bounded device/endpoint registration and local operation dispatch with duplicate/permission/lifecycle tests. Avoid duplicating the Runtime Registry.
5. **Transport contract:** settle endpoint API and error/deadline semantics.
6. **I²C safety:** implement shared transaction locking and atomic mux select + downstream transaction.
7. **Driver integration:** expose TCA9548A, DS1307, PCF8575, ADS1115, and SHT drivers through a thin adapter/descriptor layer. Keep register logic in drivers.
8. **Node/network contract:** inspect existing MQTT/remote node contracts first; define Ethernet/IP adapter only for a justified path, with security and framing.
9. **Validation:** build against the exact target ESP-IDF release; add fake-transport unit tests, concurrency/failure-injection tests, and hardware tests for missing devices, mux routing, PCF8575-to-ADS1115 sequencing, timeout and recovery.
10. **Readiness:** do not label complete/production-ready until acceptance tests pass and memory/resource bounds are measured.

## 12. Current implementation status (branch audit)

Implemented foundations visible in the branch:
- `nc_common`, `nc_property`, and `nc_device`;
- `nc_i2c` interface and `nc_i2c_espidf` adapter skeleton;
- TCA9548A, DS1307, and ADS1115 driver components;
- smoke-test application and initial architecture documentation.

Not yet implemented/verified:
- complete NC-BUS core registry/dispatch;
- canonical ID/error schema alignment with NimbleController;
- NC-BUS registration of current drivers;
- atomic mux-routed transaction and proven shared locking;
- PCF8575 driver and exact dependency sequencing;
- selected SHT driver;
- Ethernet/network node transport and security design;
- build and hardware validation on the target setup.

This is an architecture contract, not evidence of implementation completion.