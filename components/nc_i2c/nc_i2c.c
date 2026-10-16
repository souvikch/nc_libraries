#include "nc_i2c.h"
nc_err_t nc_i2c_bus_validate(const nc_i2c_bus_t *bus)
{
    if (!bus || !bus->ops || !bus->ops->write_read || !bus->ops->write) return NC_ERR_INVALID_ARG;
    return NC_OK;
}
nc_err_t nc_i2c_write_read(nc_i2c_bus_t *bus, uint8_t address, const uint8_t *wd, size_t wl, uint8_t *rd, size_t rl, uint32_t timeout)
{
    nc_err_t e=nc_i2c_bus_validate(bus);
    if(e!=NC_OK) return e;
    if(address>0x7f || (!wd && wl) || (!rd && rl) || (!wl && !rl)) return NC_ERR_INVALID_ARG;
    return bus->ops->write_read(bus,address,wd,wl,rd,rl,timeout?timeout:100);
}
nc_err_t nc_i2c_write(nc_i2c_bus_t *bus, uint8_t address, const uint8_t *data, size_t length, uint32_t timeout)
{
    nc_err_t e=nc_i2c_bus_validate(bus);
    if(e!=NC_OK) return e;
    if(address>0x7f || !data || !length) return NC_ERR_INVALID_ARG;
    return bus->ops->write(bus,address,data,length,timeout?timeout:100);
}