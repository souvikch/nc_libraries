# NC-BUS implementation and validation checklist

## Architecture alignment

- [x] Inspect `nimble-runtime` architecture before further public API work.
- [x] Record NC-BUS as the portable controller-side device/data communication contract, distinct from Runtime's Virtual Resource Registry and Digital Twin.
- [ ] Compare NCCore I2C Bus Manager, Device Registry, Virtual Resource Engine, profile schemas and remote-node messaging contracts.
- [ ] Reconcile logical IDs, standardized errors, lifecycle, and property types before freezing ABI.
- [ ] Resolve overlap between `nc_device` and the planned `nc_bus` descriptors without creating a duplicate registry.

## Implementation order

1. [ ] Finalize `nc_common` errors and primitive types against NCCore taxonomy.
2. [ ] Finalize portable typed property/value contract.
3. [ ] Implement bounded `nc_bus` APIs for device/endpoints, operation dispatch, read/write/invoke, status and lifecycle.
4. [ ] Add host/fake-endpoint tests: duplicate ID, invalid descriptor, permission denial, unsupported op, timeout, lifecycle, error propagation and preservation of last known value.
5. [ ] Add I2C transaction serialization and atomic TCA9548A select + downstream transfer.
6. [ ] Register TCA9548A and DS1307 through a thin adapter layer.
7. [ ] Add PCF8575 driver and document hardware dependency before ADS1115 sampling. Keep this sequencing out of the generic bus core.
8. [ ] Integrate ADS1115 and then the selected SHT-series model.
9. [ ] Review existing Runtime MQTT/node contract; specify Ethernet framing, identity, authentication, authorization, deadlines, duplicate handling and compatibility before creating an Ethernet adapter.
10. [ ] Build with the exact project ESP-IDF release, then run fake-transport tests, concurrency/failure-injection, and board-level hardware tests.
11. [ ] Measure memory/resource bounds before any production-readiness statement.

Hardware checks must include missing device, wrong address, SDA/SCL fault where safely testable, mux contention, timeout, recovery, and PCF8575-controlled preconditions for ADS1115 if applicable.