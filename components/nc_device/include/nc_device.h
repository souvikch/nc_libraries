#pragma once
#include "nc_common.h"
#include "nc_property.h"
#ifdef __cplusplus
extern "C" {
#endif
#define NC_DEVICE_ID_MAX_LEN 31
#define NC_DEVICE_TYPE_MAX_LEN 31
typedef struct nc_device nc_device_t;
typedef struct {
    nc_err_t (*read_property)(nc_device_t *device, const char *name, nc_property_value_t *value);
    nc_err_t (*write_property)(nc_device_t *device, const char *name, const nc_property_value_t *value);
    nc_err_t (*refresh)(nc_device_t *device);
    void (*destroy)(nc_device_t *device);
} nc_device_ops_t;
struct nc_device { char id[NC_DEVICE_ID_MAX_LEN + 1]; char type[NC_DEVICE_TYPE_MAX_LEN + 1]; void *driver_context; const nc_device_ops_t *ops; };
nc_err_t nc_device_validate(const nc_device_t *device);
#ifdef __cplusplus
}
#endif
