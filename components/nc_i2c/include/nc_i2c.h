#pragma once
#include <stddef.h>
#include <stdint.h>
#include "nc_common.h"
#ifdef __cplusplus
extern "C" {
#endif
/* Provisional adapter boundary. Bind this to the real NCCore I2C Bus Manager API before hardware I/O. */
typedef struct nc_i2c_bus nc_i2c_bus_t;
typedef struct {
    nc_err_t (*write_read)(nc_i2c_bus_t *bus, uint8_t address, const uint8_t *write_data, size_t write_length, uint8_t *read_data, size_t read_length, uint32_t timeout_ms);
    nc_err_t (*write)(nc_i2c_bus_t *bus, uint8_t address, const uint8_t *data, size_t length, uint32_t timeout_ms);
} nc_i2c_ops_t;
struct nc_i2c_bus { void *context; const nc_i2c_ops_t *ops; };
nc_err_t nc_i2c_bus_validate(const nc_i2c_bus_t *bus);
#ifdef __cplusplus
}
#endif
