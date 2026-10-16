#pragma once
#include <stdint.h>
#include "nc_device.h"
#include "nc_i2c.h"
#ifdef __cplusplus
extern "C" {
#endif
#define NC_DS1307_DEFAULT_ADDRESS 0x68
typedef struct {
    nc_i2c_bus_t *bus;
    uint8_t address;
} nc_ds1307_config_t;
nc_err_t nc_ds1307_validate_config(const nc_ds1307_config_t *config);
#ifdef __cplusplus
}
#endif
