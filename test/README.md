# Test application and acceptance tests

The default main/app_main.c is a no-hardware smoke test for invalid configuration handling. The ESP-IDF hardware transport adapter is components/nc_i2c_espidf.

## Build
- Use ESP-IDF 5.3 or newer (the adapter uses the i2c_master API).
- Configure the actual board's SDA/SCL GPIOs and bus clock in app_main before calling nc_i2c_espidf_init.
- Run idf.py set-target esp32, idf.py build, then flash and monitor.

## Hardware acceptance
- TCA9548A: select each channel 0–7, disable all channels, and verify downstream isolation.
- DS1307: set/read a known date/time, verify seconds advance, and check oscillator status.
- ADS1115: compare input readings against a calibrated meter at multiple input ranges.
- Fault tests: disconnect devices, use a wrong address, and verify timeout/error propagation.
- Concurrency: verify shared bus transactions are serialized. When a mux is used, channel selection plus downstream transaction must be protected as one logical critical section.

A hardware pass has not been claimed until this builds against the selected ESP-IDF version and is tested on the actual board. Check pull-ups, common ground, voltage levels, and device supply separately.