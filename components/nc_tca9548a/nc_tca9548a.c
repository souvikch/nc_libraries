#include "nc_tca9548a.h"

nc_err_t nc_tca9548a_validate_config(const nc_tca9548a_config_t *config)
{
    if (config == 0 || config->bus == 0 || config->address < 0x70 || config->address > 0x77) {
        return NC_ERR_INVALID_ARG;
    }
    return nc_i2c_bus_validate(config->bus);
}

static nc_err_t write_mask(const nc_tca9548a_config_t *config, uint8_t mask)
{
    nc_err_t err = nc_tca9548a_validate_config(config);
    if (err != NC_OK) return err;
    return config->bus->ops->write(config->bus, config->address, &mask, 1, 100);
}

nc_err_t nc_tca9548a_select_channel(const nc_tca9548a_config_t *config, uint8_t channel)
{
    if (channel >= NC_TCA9548A_CHANNEL_COUNT) return NC_ERR_INVALID_ARG;
    return write_mask(config, (uint8_t)(1U << channel));
}

nc_err_t nc_tca9548a_disable_all_channels(const nc_tca9548a_config_t *config)
{
    return write_mask(config, 0x00);
}
