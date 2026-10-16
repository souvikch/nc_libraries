#pragma once
#include <stdint.h>
#include "nc_device.h"
#include "nc_i2c.h"
#ifdef __cplusplus
extern "C" {
#endif
#define NC_ADS1115_DEFAULT_ADDRESS 0x48
typedef struct { nc_i2c_bus_t *bus; uint8_t address; } nc_ads1115_config_t;
/* Validation only: device initialization and hardware transactions are not implemented yet. */
nc_err_t nc_ads1115_validate_config(const nc_ads1115_config_t *config);
#ifdef __cplusplus
}
#endif
