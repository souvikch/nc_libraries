#pragma once
#include <stdbool.h>
#include <stdint.h>
#include "nc_device.h"
#include "nc_i2c.h"
#ifdef __cplusplus
extern "C" {
#endif
#define NC_ADS1115_DEFAULT_ADDRESS 0x48
#define NC_ADS1115_CHANNEL_COUNT 4
typedef enum { NC_ADS1115_FS_6V144=0, NC_ADS1115_FS_4V096, NC_ADS1115_FS_2V048, NC_ADS1115_FS_1V024, NC_ADS1115_FS_0V512, NC_ADS1115_FS_0V256 } nc_ads1115_range_t;
typedef enum { NC_ADS1115_RATE_8=0, NC_ADS1115_RATE_16, NC_ADS1115_RATE_32, NC_ADS1115_RATE_64, NC_ADS1115_RATE_128, NC_ADS1115_RATE_250, NC_ADS1115_RATE_475, NC_ADS1115_RATE_860 } nc_ads1115_rate_t;
typedef struct { nc_i2c_bus_t *bus; uint8_t address; uint32_t timeout_ms; nc_ads1115_range_t range; nc_ads1115_rate_t rate; } nc_ads1115_config_t;
nc_err_t nc_ads1115_validate_config(const nc_ads1115_config_t *config);
nc_err_t nc_ads1115_read_raw(const nc_ads1115_config_t *config, uint8_t channel, int16_t *raw);
nc_err_t nc_ads1115_read_voltage_mv(const nc_ads1115_config_t *config, uint8_t channel, int32_t *millivolts);
#ifdef __cplusplus
}
#endif