#pragma once
#include <stdint.h>
#include "nc_common.h"
#include "nc_i2c.h"
#ifdef __cplusplus
extern "C" {
#endif
#define NC_TCA9548A_DEFAULT_ADDRESS 0x70
#define NC_TCA9548A_CHANNEL_COUNT 8
typedef struct {
    nc_i2c_bus_t *bus;
    uint8_t address;
} nc_tca9548a_config_t;
nc_err_t nc_tca9548a_validate_config(const nc_tca9548a_config_t *config);
nc_err_t nc_tca9548a_select_channel(const nc_tca9548a_config_t *config, uint8_t channel);
nc_err_t nc_tca9548a_disable_all_channels(const nc_tca9548a_config_t *config);
#ifdef __cplusplus
}
#endif
