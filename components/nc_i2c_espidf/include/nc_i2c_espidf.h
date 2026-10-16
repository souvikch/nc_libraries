#pragma once
#include "driver/i2c_master.h"
#include "nc_i2c.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct {
    i2c_master_bus_handle_t handle;
    nc_i2c_bus_t bus;
    uint32_t default_timeout_ms;
} nc_i2c_espidf_t;
/* Create the ESP-IDF 5.x I2C master bus. Caller chooses board-specific GPIOs. */
nc_err_t nc_i2c_espidf_init(nc_i2c_espidf_t *adapter, i2c_port_num_t port, int sda_gpio, int scl_gpio, uint32_t frequency_hz, bool enable_pullups);
nc_err_t nc_i2c_espidf_deinit(nc_i2c_espidf_t *adapter);
nc_i2c_bus_t *nc_i2c_espidf_bus(nc_i2c_espidf_t *adapter);
#ifdef __cplusplus
}
#endif