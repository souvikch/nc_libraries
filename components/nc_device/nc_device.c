#include "nc_device.h"
nc_err_t nc_device_validate(const nc_device_t *device)
{
    if (device == 0 || device->id[0] == '\0' || device->type[0] == '\0' || device->ops == 0) return NC_ERR_INVALID_ARG;
    return NC_OK;
}
