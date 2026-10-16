#include "nc_i2c.h"
nc_err_t nc_i2c_bus_validate(const nc_i2c_bus_t *bus)
{
    if (bus == 0 || bus->ops == 0 || bus->ops->write_read == 0 || bus->ops->write == 0) return NC_ERR_INVALID_ARG;
    return NC_OK;
}
