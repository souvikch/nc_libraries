# NC-BUS Object Architecture

**Document ID:** NC-LIB-BUS-002  
**Version:** 0.1  
**Status:** Draft  
**Related:** [NC-BUS Core Architecture and Contract](nc-bus-specification.md)

This document applies the six object models mandated by the NimbleController Runtime architecture: hierarchy, ownership, relationships, lifecycle, state, and interaction. No public ABI is frozen by this draft.

## 1. NC-BUS Core

**Purpose:** Provide a portable logical operation/data contract that can dispatch to configured local or remote endpoints.

**Owner:** Controller-side NC-BUS lifecycle owner, started/stopped by firmware integration.

**Non-responsibilities:** Runtime topology authority, Digital Twin, provisioning, resource allocation, automation policy, physical bus initialization, device register semantics, and network security implementation.

**Hierarchy:** controller integration → NC-BUS core → endpoint/driver adapters.

**Relationships:** uses configured device descriptors and endpoint adapters; dispatches requests; publishes status/value updates through caller-provided hooks. It does not create Virtual Resources or own the Runtime Registry.

**Lifecycle:** uninitialized → initialized → active → stopping → stopped. Failed initialization must leave the component non-active.

**State:** immutable limits/configuration and adapter registrations; mutable active flag, dispatch statistics, and bounded registration state.

**Interactions:** callers submit typed requests; core validates identity/operation/permissions/lifecycle, dispatches via the selected adapter, and returns a typed result/error before the deadline.

## 2. Device Descriptor

**Purpose:** Describe a controller-visible physical/logical device and supported operations without exposing endpoint data to applications.

**Owner:** The driver/device integration layer that registers it. Exactly one owner manages its lifecycle.

**Hierarchy:** NC-BUS → device descriptor → property/operation descriptors.

**Relationships:** bound to one owning node ID, one stable device ID, zero or more configured transport endpoints (if supported), and driver-provided capability/operation descriptions.

**Lifecycle:** defined → validated → registered → initializing → ready/degraded/unavailable → stopping → retired.

**State:** static identity/type/schema/capability definition separated from mutable availability and diagnostics.

**Interactions:** registration rejects malformed descriptors and duplicate IDs; requests resolve by stable logical ID, not transport address. Canonical ID representation must match Runtime/controller provisioning contracts before ABI freeze.

## 3. Endpoint

**Purpose:** Describe a transport-specific route to a device or node.

**Owner:** Its transport/bus manager; endpoint resource handles must not be owned independently by every driver.

**Hierarchy:** NC-BUS routing view → transport adapter → endpoint → physical bus/network interface.

**Relationships:** belongs to a transport adapter, associates with a logical destination, and may include nested routes such as I²C bus + TCA9548A channel + downstream address.

**Lifecycle:** configured → opened/initialized → available → degraded/unavailable → closed.

**State:** immutable validated endpoint configuration; mutable link/device reachability, last error and counters.

**Interactions:** routing selects a configured endpoint; adapter executes within finite timeout; endpoint-specific failure maps to common error plus diagnostic detail.

## 4. Property Descriptor and Value

**Purpose:** Define a typed measurable/configurable attribute exposed by a device.

**Owner:** Device driver/schema integration layer owns the definition; the data/telemetry layer owns retained runtime values where the controller architecture assigns that ownership.

**Hierarchy:** device descriptor → property descriptor → current value/status.

**Relationships:** descriptor defines stable property ID, type, unit/scale, permissions and constraints; value refers to that property definition.

**Lifecycle:** descriptor validated and registered with device; value moves from unknown/unavailable to valid, then updates or becomes stale/invalid without fabricating a default.

**State:** definition is immutable for a schema version; value, timestamp, validity, quality and update sequence are mutable.

**Interactions:** read/write checks permissions and type; conversion/transport failure returns an error and preserves last-known-good value separately from the failure status.

## 5. Operation Request and Result

**Purpose:** Represent a bounded read, write, invoke, or status request.

**Owner:** Requesting component owns request storage until completion; receiving adapter must not retain pointers beyond the documented call lifetime unless it copies into owned bounded storage.

**Hierarchy:** caller → NC-BUS dispatch → adapter → driver/remote peer.

**Relationships:** request identifies destination device, property/operation, bounded input, deadline and optional correlation ID; result contains outcome, bounded output and normalized error.

**Lifecycle:** constructed → validated → dispatched → completed/failed/timed-out. Cancellation semantics must be specified before asynchronous APIs are added.

**State:** request input is immutable during dispatch; result changes once to terminal completion.

**Interactions:** reject expired/oversized/unsupported requests before I/O where possible. A timeout does not prove that a remote non-idempotent operation did not execute; correlation and duplicate handling are required for network requests.

## 6. Route

**Purpose:** Resolve stable logical destination to a configured endpoint/adapter.

**Owner:** Controller configuration/NC-BUS integration owns the routing view; Runtime remains authoritative for topology and supplies validated configuration.

**Hierarchy:** NC-BUS → route table/view → endpoint references.

**Relationships:** maps a device/node logical ID to an endpoint or explicitly supported endpoint set; route may include local mux segments.

**Lifecycle:** built from validated configuration → active → updated atomically through controlled reconfiguration → retired.

**State:** route definition is versioned; reachability and last error are runtime state.

**Interactions:** route lookup uses stable/numeric IDs in the hot path. It must not infer a route by scanning display names or treating a hardware address as a global identity.

## 7. Lifecycle/Status Event

**Purpose:** Signal device availability, transport link changes, and significant operation failures.

**Owner:** Event publisher owns publication; the subscribed event system owns queueing/delivery according to its documented contract.

**Hierarchy:** driver/adapter → controller event interface → controller event consumers.

**Relationships:** published by lifecycle owner; consumed by health monitor, diagnostics, and controller integration. It may inform telemetry but does not replace telemetry value updates.

**Lifecycle:** created → queued/published → delivered or dropped according to bounded policy.

**State:** event payload is immutable and bounded; event queue counters/drop status are mutable.

**Interactions:** events must be rate-limited/coalesced where repeated link or sensor faults could flood memory. A notification does not itself change Runtime policy or allocate/rebind Virtual Resources.

## Implementation constraints

- The final public structs/enums must be checked against NCCore APIs and schemas first.
- Avoid implementing a second general-purpose registry if the existing `nc_device` or NCCore registry already owns the same objects.
- Initial APIs should be synchronous and bounded unless an actual use case requires asynchronous message queues.
- Remote request semantics (acknowledgement, duplicate suppression, delivery guarantee) are not interchangeable with local I²C transaction semantics.
- No claim of implementation completion follows from this design document alone.
