#include "nc_ads1115.h"
#define REG_CONVERSION 0x00
#define REG_CONFIG 0x01
static const int32_t fs_uv[] = {6144000,4096000,2048000,1024000,512000,256000};
static const uint8_t rates[] = {0,1,2,3,4,5,6,7};
nc_err_t nc_ads1115_validate_config(const nc_ads1115_config_t *c)
{
    if (!c || !c->bus || c->address < 0x48 || c->address > 0x4b ||
        c->range > NC_ADS1115_FS_0V256 || c->rate > NC_ADS1115_RATE_860) return NC_ERR_INVALID_ARG;
    return nc_i2c_bus_validate(c->bus);
}
static nc_err_t write_reg(const nc_ads1115_config_t *c, uint8_t reg, uint16_t value)
{
    uint8_t b[3] = {reg, (uint8_t)(value >> 8), (uint8_t)value};
    return nc_i2c_write(c->bus,c->address,b,sizeof(b),c->timeout_ms);
}
static nc_err_t read_reg(const nc_ads1115_config_t *c, uint8_t reg, uint16_t *value)
{
    uint8_t b[2];
    nc_err_t e=nc_i2c_write_read(c->bus,c->address,&reg,1,b,2,c->timeout_ms);
    if(e==NC_OK) *value=((uint16_t)b[0]<<8)|b[1];
    return e;
}
nc_err_t nc_ads1115_read_raw(const nc_ads1115_config_t *c, uint8_t channel, int16_t *raw)
{
    nc_err_t e=nc_ads1115_validate_config(c);
    if(e!=NC_OK) return e;
    if(!raw || channel>=NC_ADS1115_CHANNEL_COUNT) return NC_ERR_INVALID_ARG;
    uint16_t cfg=(uint16_t)(0x8000U | ((uint16_t)(4+channel)<<12) |
        ((uint16_t)c->range<<9) | 0x0100U | ((uint16_t)rates[c->rate]<<5) | 0x0003U);
    e=write_reg(c,REG_CONFIG,cfg); if(e!=NC_OK) return e;
    uint32_t timeout=c->timeout_ms?c->timeout_ms:100, elapsed=0;
    for(;;) {
        uint16_t current; e=read_reg(c,REG_CONFIG,&current); if(e!=NC_OK) return e;
        if(current&0x8000U) break;
        if(elapsed>=timeout) return NC_ERR_BUSY;
        if(c->bus->ops->delay_ms) c->bus->ops->delay_ms(c->bus,1);
        elapsed++;
    }
    uint16_t v; e=read_reg(c,REG_CONVERSION,&v); if(e==NC_OK) *raw=(int16_t)v;
    return e;
}
nc_err_t nc_ads1115_read_voltage_mv(const nc_ads1115_config_t *c, uint8_t channel, int32_t *mv)
{
    if(!mv) return NC_ERR_INVALID_ARG;
    int16_t raw; nc_err_t e=nc_ads1115_read_raw(c,channel,&raw);
    if(e!=NC_OK) return e;
    *mv=((int32_t)raw*fs_uv[c->range])/32768/1000;
    return NC_OK;
}