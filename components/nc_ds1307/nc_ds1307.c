#include "nc_ds1307.h"
nc_err_t nc_ds1307_validate_config(const nc_ds1307_config_t *config)
{
    if (config == 0 || config->bus == 0 || config->address != NC_DS1307_DEFAULT_ADDRESS) return NC_ERR_INVALID_ARG;
    return nc_i2c_bus_validate(config->bus);
}
