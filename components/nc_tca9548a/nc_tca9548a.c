#include "nc_tca9548a.h"
nc_err_t nc_tca9548a_validate_config(const nc_tca9548a_config_t *config)
{
    if (config == 0 || config->bus == 0 || config->address < 0x70 || config->address > 0x77) return NC_ERR_INVALID_ARG;
    return nc_i2c_bus_validate(config->bus);
}
/* Implement transaction functions after nc_i2c is connected to the NCCore bus manager. */
nc_err_t nc_tca9548a_select_channel(const nc_tca9548a_config_t *config, uint8_t channel)
{
    if (channel >= NC_TCA9548A_CHANNEL_COUNT) return NC_ERR_INVALID_ARG;
    nc_err_t err = nc_tca9548a_validate_config(config);
    return err == NC_OK ? NC_ERR_NOT_SUPPORTED : err;
}
nc_err_t nc_tca9548a_disable_all_channels(const nc_tca9548a_config_t *config)
{
    nc_err_t err = nc_tca9548a_validate_config(config);
    return err == NC_OK ? NC_ERR_NOT_SUPPORTED : err;
}
