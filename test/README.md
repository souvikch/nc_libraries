# Test application and acceptance tests

The default `main/app_main.c` is a hardware smoke test. The ESP-IDF transport adapter is `components/nc_i2c_espidf`.

## Host unit test: DS1307

The DS1307 driver can be tested without ESP-IDF hardware using its fake I²C transport:

```sh
cc -std=c11 -Wall -Wextra -Werror \
  -Icomponents/nc_common/include \
  -Icomponents/nc_i2c/include \
  -Icomponents/nc_device/include \
  -Icomponents/nc_property/include \
  -Icomponents/nc_ds1307/include \
  test/test_nc_ds1307.c \
  components/nc_ds1307/nc_ds1307.c \
  components/nc_i2c/nc_i2c.c \
  -o test_nc_ds1307
./test_nc_ds1307
```

The host test covers valid leap-day reads, stopped oscillator reporting, invalid BCD input, rejection of an impossible calendar date, setting a valid date/time, oscillator status, and I²C error propagation.

## ESP-IDF build and hardware acceptance

- Use ESP-IDF 5.3 or newer (the adapter uses the I²C master API).
- Configure the board's SDA/SCL GPIOs and bus clock in `main/app_main.c` before calling `nc_i2c_espidf_init`.
- Run `idf.py set-target esp32`, `idf.py build`, then flash and monitor.
- TCA9548A: select each channel 0–7, disable all channels, and verify downstream isolation.
- DS1307: set/read a known date/time, verify seconds advance, and check oscillator status.
- ADS1115: compare input readings against a calibrated meter at multiple input ranges.
- Fault tests: disconnect devices, use a wrong address, and verify timeout/error propagation.
- Concurrency: verify shared bus transactions are serialized. When a mux is used, channel selection plus downstream transaction must be protected as one logical critical section.

A hardware pass has not been claimed until the firmware builds against the selected ESP-IDF version and is tested on the actual board. Check pull-ups, common ground, voltage levels, and device supply separately.
