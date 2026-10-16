#include "nc_ads1115.h"
nc_err_t nc_ads1115_validate_config(const nc_ads1115_config_t *config)
{
    if (config == 0 || config->bus == 0 || config->address < 0x48 || config->address > 0x4B) return NC_ERR_INVALID_ARG;
    return nc_i2c_bus_validate(config->bus);
}
