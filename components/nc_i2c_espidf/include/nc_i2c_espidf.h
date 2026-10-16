#pragma once
#include <stdbool.h>
#include "driver/i2c_master.h"
#include "nc_i2c.h"
#ifdef __cplusplus
extern "C" {
#endif
#define NC_I2C_ESP_IDF_MAX_DEVICES 8
typedef struct { i2c_master_dev_handle_t handle; uint8_t address; } nc_i2c_espidf_device_t;
typedef struct {
    i2c_master_bus_handle_t handle;
    nc_i2c_bus_t bus;
    uint32_t default_timeout_ms;
    uint32_t frequency_hz;
    nc_i2c_espidf_device_t devices[NC_I2C_ESP_IDF_MAX_DEVICES];
} nc_i2c_espidf_t;
/* ESP-IDF 5.x master adapter. GPIO selection and pull-up policy are board-specific. */
nc_err_t nc_i2c_espidf_init(nc_i2c_espidf_t *adapter, i2c_port_num_t port, int sda_gpio, int scl_gpio, uint32_t frequency_hz, bool enable_pullups);
nc_err_t nc_i2c_espidf_deinit(nc_i2c_espidf_t *adapter);
nc_i2c_bus_t *nc_i2c_espidf_bus(nc_i2c_espidf_t *adapter);
#ifdef __cplusplus
}
#endif