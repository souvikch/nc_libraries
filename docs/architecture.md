# Architecture

## Device implementation order

1. **TCA9548A**: establish I2C channel selection and routing.
2. **DS1307 RTC**: build a basic register-based clock driver on the shared I2C layer.
3. **ADS1115 ADC**: add conversion, configuration, gain, and data-rate support on the same layer.

## Layers

1. Application uses generic device and property interfaces.
2. Device framework provides a common device contract.
3. Device components implement chip-specific behavior.
4. I2C transport delegates transactions to the platform bus manager.
5. The bus manager owns bus setup, timing, recovery, and transaction policy.

## Integration boundary

The existing NCCore I2C Bus Manager API was not available in this repository context. The `nc_i2c_bus_t` interface is a provisional adapter boundary, not a claim about the current NCCore implementation. Adapt it to the actual bus-manager API and lifecycle before adding hardware transactions.

## Dynamic properties

Properties carry a name, type, access flags, optional unit, and typed value. Property enumeration/registration and device-registry ownership remain future implementation steps.
