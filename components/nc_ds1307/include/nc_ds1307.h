#pragma once
#include <stdbool.h>
#include <stdint.h>
#include "nc_device.h"
#include "nc_i2c.h"
#ifdef __cplusplus
extern "C" {
#endif
#define NC_DS1307_DEFAULT_ADDRESS 0x68
typedef struct { uint16_t year; uint8_t month, day, weekday, hour, minute, second; } nc_datetime_t;
typedef struct { nc_i2c_bus_t *bus; uint8_t address; uint32_t timeout_ms; } nc_ds1307_config_t;
nc_err_t nc_ds1307_validate_config(const nc_ds1307_config_t *config);
nc_err_t nc_ds1307_read_datetime(const nc_ds1307_config_t *config, nc_datetime_t *datetime);
nc_err_t nc_ds1307_set_datetime(const nc_ds1307_config_t *config, const nc_datetime_t *datetime);
nc_err_t nc_ds1307_is_oscillator_running(const nc_ds1307_config_t *config, bool *running);
#ifdef __cplusplus
}
#endif