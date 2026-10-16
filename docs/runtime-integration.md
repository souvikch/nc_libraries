# NC-BUS Integration with NimbleController Runtime

**Status:** Draft — integration contract pending NCCore API audit.

## Architectural boundary

The architecture documented by `souvikch/nimble-runtime` distinguishes:
- Runtime: authoritative Digital Twin, topology, policy, profiles, constraints, provisioning and resource allocation.
- Controller: executes a validated Runtime Model, manages drivers, exposes Virtual Resources, maintains operational state, publishes telemetry and executes commands.
- Driver/bus stack: performs bounded operations on physical hardware through bus managers and HAL.
- Application/automation code: consumes Virtual Resources, not physical addresses or direct hardware APIs.

NC-BUS is a portable controller-side communication/data contract used by the driver stack and, where applicable, local/remote node integration. It must not replace the Runtime's Virtual Resource Registry or invent a second resource allocation model.

## Integration flow

### Startup
1. Initialize HAL and physical bus managers.
2. Load/validate the Platform Profile and Device Profile using established firmware/runtime APIs.
3. Initialize platform devices and configured provisioned devices in dependency order.
4. Register driver capabilities/operations through the controller's existing Device Manager and Runtime Registry interfaces.
5. Construct/activate Virtual Resources only through the established Virtual Resource Engine/registry, using the validated Runtime Model.
6. Start service/event/network integration after hardware/runtime state is consistent.
7. Report health and partial availability; optional missing devices must not prevent safe operation of unrelated resources.

### Local sensor read
1. Application requests a Virtual Resource operation.
2. Virtual Resource Engine resolves the validated binding.
3. Device/driver operation is invoked through the established driver contract.
4. Driver uses the correct bus manager and configured endpoint.
5. Data is converted and returned with validity/status/timestamp as defined by the current controller contract.
6. Controller runtime state and telemetry are updated through their established owners.

NC-BUS may standardize a device/operation envelope for this path, but it must not bypass the existing registry or introduce a second binding resolver.

### Remote node operation
1. Controller integration resolves the configured remote node/device identity.
2. Request is serialized through the existing Runtime/node communication contract when that is the selected link.
3. The authenticated peer validates message identity, permissions, version, size and deadline.
4. Response/telemetry includes correlation and an unambiguous result/status.
5. Duplicate/retry behavior is defined for each operation class; non-idempotent operations are never blindly retried.

Direct Ethernet/IP transport should be added only if it serves a documented path distinct from existing Runtime MQTT/node messaging. Ethernet PHY/link, IP transport, and application message semantics are separate concepts.

## Integration prerequisites before ABI freeze

- Inspect actual NCCore public I²C Bus Manager and HAL APIs.
- Inspect driver interface, Device Manager/Registry and Virtual Resource Engine contracts.
- Inspect Platform Profile, Device Profile and compiled Runtime Model schema.
- Inspect MQTT/node command and telemetry envelopes used by Runtime.
- Reconcile stable IDs, capability IDs, status/error enums, timestamps, units, permissions and lifecycle states.
- Define who owns values and device lifecycle, then remove duplicate concepts from `nc_device` vs `nc_bus`.
- Keep this repository's core independent of ESP-IDF/FreeRTOS; put OS-specific synchronization/queues behind adapters.

## Explicitly not specified yet

This draft does not invent exact NCCore function names, identity formats, MQTT topics, JSON fields, wire format, Ethernet port, authentication handshake, retry values or queue limits. Those must be extracted from the authoritative implementation/contracts before they become API guarantees.
