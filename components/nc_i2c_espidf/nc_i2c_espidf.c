#include <string.h>
#include "esp_err.h"
#include "esp_rom_sys.h"
#include "nc_i2c_espidf.h"
typedef struct { i2c_master_dev_handle_t handle; uint8_t address; } device_cache_t;
#define NC_I2C_CACHE_SIZE 8
typedef struct { nc_i2c_espidf_t *owner; device_cache_t devices[NC_I2C_CACHE_SIZE]; } adapter_context_t;
/* Context is statically embedded to avoid heap allocation per transaction. */
static adapter_context_t contexts[2];
static nc_err_t map_err(esp_err_t e) { return e==ESP_OK?NC_OK:(e==ESP_ERR_NO_MEM?NC_ERR_NO_MEMORY:(e==ESP_ERR_INVALID_ARG?NC_ERR_INVALID_ARG:NC_ERR_IO)); }
static i2c_master_dev_handle_t get_device(nc_i2c_espidf_t *a, uint8_t address)
{
    adapter_context_t *ctx=(adapter_context_t *)a->bus.context;
    for(int i=0;i<NC_I2C_CACHE_SIZE;i++) if(ctx->devices[i].handle && ctx->devices[i].address==address) return ctx->devices[i].handle;
    for(int i=0;i<NC_I2C_CACHE_SIZE;i++) if(!ctx->devices[i].handle) {
        i2c_device_config_t cfg={.dev_addr_length=I2C_ADDR_BIT_LEN_7,.device_address=address,.scl_speed_hz=100000};
        if(i2c_master_bus_add_device(a->handle,&cfg,&ctx->devices[i].handle)!=ESP_OK) return NULL;
        ctx->devices[i].address=address;
        return ctx->devices[i].handle;
    }
    return NULL;
}
static nc_err_t op_write_read(nc_i2c_bus_t *bus,uint8_t address,const uint8_t *wd,size_t wl,uint8_t *rd,size_t rl,uint32_t timeout)
{
    nc_i2c_espidf_t *a=(nc_i2c_espidf_t *)((adapter_context_t *)bus->context)->owner;
    i2c_master_dev_handle_t dev=get_device(a,address); if(!dev) return NC_ERR_NO_MEMORY;
    return map_err(i2c_master_transmit_receive(dev,wd,wl,rd,rl,timeout?timeout:a->default_timeout_ms));
}
static nc_err_t op_write(nc_i2c_bus_t *bus,uint8_t address,const uint8_t *data,size_t len,uint32_t timeout)
{
    nc_i2c_espidf_t *a=(nc_i2c_espidf_t *)((adapter_context_t *)bus->context)->owner;
    i2c_master_dev_handle_t dev=get_device(a,address); if(!dev) return NC_ERR_NO_MEMORY;
    return map_err(i2c_master_transmit(dev,data,len,timeout?timeout:a->default_timeout_ms));
}
static void op_delay(nc_i2c_bus_t *bus,uint32_t ms) { (void)bus; esp_rom_delay_us(ms*1000U); }
static const nc_i2c_ops_t ops={.write_read=op_write_read,.write=op_write,.delay_ms=op_delay};
nc_err_t nc_i2c_espidf_init(nc_i2c_espidf_t *a,i2c_port_num_t port,int sda,int scl,uint32_t hz,bool pullups)
{
    if(!a || sda<0 || scl<0 || !hz) return NC_ERR_INVALID_ARG;
    memset(a,0,sizeof(*a));
    adapter_context_t *ctx=NULL;
    for(int i=0;i<2;i++) if(contexts[i].owner==NULL) { ctx=&contexts[i]; break; }
    if(!ctx) return NC_ERR_NO_MEMORY;
    i2c_master_bus_config_t cfg={.i2c_port=port,.sda_io_num=sda,.scl_io_num=scl,
        .clk_source=I2C_CLK_SRC_DEFAULT,.glitch_ignore_cnt=7,.flags.enable_internal_pullup=pullups};
    esp_err_t e=i2c_new_master_bus(&cfg,&a->handle); if(e!=ESP_OK) return map_err(e);
    a->default_timeout_ms=100;
    ctx->owner=a; a->bus.context=ctx; a->bus.ops=&ops;
    return NC_OK;
}
nc_err_t nc_i2c_espidf_deinit(nc_i2c_espidf_t *a)
{
    if(!a || !a->handle) return NC_ERR_INVALID_ARG;
    adapter_context_t *ctx=(adapter_context_t *)a->bus.context;
    for(int i=0;i<NC_I2C_CACHE_SIZE;i++) if(ctx->devices[i].handle) {
        i2c_master_bus_rm_device(ctx->devices[i].handle);
        ctx->devices[i].handle=NULL;
    }
    esp_err_t e=i2c_del_master_bus(a->handle);
    if(e==ESP_OK) { ctx->owner=NULL; memset(a,0,sizeof(*a)); }
    return map_err(e);
}
nc_i2c_bus_t *nc_i2c_espidf_bus(nc_i2c_espidf_t *a) { return a && a->handle ? &a->bus : NULL; }