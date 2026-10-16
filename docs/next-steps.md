# Next steps

1. Confirm the existing NCCore I2C Bus Manager public API and lifecycle.
2. Adapt `nc_i2c` to that API without duplicating bus ownership.
3. Implement TCA9548A channel selection and disable-all behavior; test each channel.
4. Implement DS1307 register access, BCD conversion, clock read/write, and oscillator/status handling.
5. Implement ADS1115 configuration and conversion reads.
6. Add dynamic property enumeration, a device registry, unit tests, and hardware validation instructions.
