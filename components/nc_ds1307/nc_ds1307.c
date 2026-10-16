#include <stdbool.h>
#include "nc_ds1307.h"
static uint8_t bcd_to_bin(uint8_t v) { return (uint8_t)((v >> 4) * 10 + (v & 0x0f)); }
static uint8_t bin_to_bcd(uint8_t v) { return (uint8_t)(((v / 10) << 4) | (v % 10)); }
nc_err_t nc_ds1307_validate_config(const nc_ds1307_config_t *c)
{
    if (!c || !c->bus || c->address != NC_DS1307_DEFAULT_ADDRESS) return NC_ERR_INVALID_ARG;
    return nc_i2c_bus_validate(c->bus);
}
nc_err_t nc_ds1307_read_datetime(const nc_ds1307_config_t *c, nc_datetime_t *dt)
{
    nc_err_t e = nc_ds1307_validate_config(c);
    if (e != NC_OK) return e;
    if (!dt) return NC_ERR_INVALID_ARG;
    uint8_t reg = 0, b[7];
    e = c->bus->ops->write_read(c->bus, c->address, &reg, 1, b, sizeof(b), c->timeout_ms ? c->timeout_ms : 100);
    if (e != NC_OK) return e;
    if (b[0] & 0x80U) return NC_ERR_NOT_INITIALIZED; /* oscillator halted */
    dt->second = bcd_to_bin(b[0] & 0x7f); dt->minute = bcd_to_bin(b[1] & 0x7f);
    if (b[2] & 0x40U) { uint8_t h=bcd_to_bin(b[2]&0x1f); dt->hour=(uint8_t)(h%12 + ((b[2]&0x20U)?12:0)); }
    else dt->hour=bcd_to_bin(b[2]&0x3f);
    dt->weekday=bcd_to_bin(b[3]&0x07); dt->day=bcd_to_bin(b[4]&0x3f);
    dt->month=bcd_to_bin(b[5]&0x1f); dt->year=(uint16_t)(2000+bcd_to_bin(b[6]));
    if (dt->second>59 || dt->minute>59 || dt->hour>23 || dt->weekday<1 || dt->weekday>7 ||
        dt->day<1 || dt->day>31 || dt->month<1 || dt->month>12) return NC_ERR_IO;
    return NC_OK;
}
nc_err_t nc_ds1307_set_datetime(const nc_ds1307_config_t *c, const nc_datetime_t *dt)
{
    nc_err_t e=nc_ds1307_validate_config(c);
    if(e!=NC_OK) return e;
    if(!dt || dt->year<2000 || dt->year>2099 || dt->month<1 || dt->month>12 ||
       dt->day<1 || dt->day>31 || dt->weekday<1 || dt->weekday>7 ||
       dt->hour>23 || dt->minute>59 || dt->second>59) return NC_ERR_INVALID_ARG;
    uint8_t b[8]={0,bin_to_bcd(dt->second),bin_to_bcd(dt->minute),bin_to_bcd(dt->hour),
        bin_to_bcd(dt->weekday),bin_to_bcd(dt->day),bin_to_bcd(dt->month),bin_to_bcd((uint8_t)(dt->year-2000))};
    return c->bus->ops->write(c->bus,c->address,b,sizeof(b),c->timeout_ms?c->timeout_ms:100);
}
nc_err_t nc_ds1307_is_oscillator_running(const nc_ds1307_config_t *c, bool *running)
{
    nc_err_t e=nc_ds1307_validate_config(c); if(e!=NC_OK) return e;
    if(!running) return NC_ERR_INVALID_ARG;
    uint8_t reg=0, sec=0;
    e=c->bus->ops->write_read(c->bus,c->address,&reg,1,&sec,1,c->timeout_ms?c->timeout_ms:100);
    if(e==NC_OK) *running=(sec&0x80U)==0;
    return e;
}