#include "esp_log.h"
#include "nc_common.h"
#include "nc_i2c_espidf.h"
#include "nc_tca9548a.h"
#include "nc_ds1307.h"
#include "nc_ads1115.h"
#ifndef NC_TEST_I2C_SDA_GPIO
#define NC_TEST_I2C_SDA_GPIO 21
#endif
#ifndef NC_TEST_I2C_SCL_GPIO
#define NC_TEST_I2C_SCL_GPIO 22
#endif
#ifndef NC_TEST_I2C_PORT
#define NC_TEST_I2C_PORT I2C_NUM_0
#endif
static const char *TAG="nc_libraries_test";
static void report(const char *name,nc_err_t e) { if(e==NC_OK) ESP_LOGI(TAG,"%s: PASS",name); else ESP_LOGW(TAG,"%s: FAIL (%s)",name,nc_err_to_name(e)); }
void app_main(void)
{
    report("I2C null-bus validation",nc_i2c_bus_validate(NULL)==NC_ERR_INVALID_ARG?NC_OK:NC_ERR_INTERNAL);
    static nc_i2c_espidf_t adapter;
    nc_err_t e=nc_i2c_espidf_init(&adapter,NC_TEST_I2C_PORT,NC_TEST_I2C_SDA_GPIO,NC_TEST_I2C_SCL_GPIO,100000,true);
    if(e!=NC_OK) { ESP_LOGE(TAG,"I2C init failed: %s",nc_err_to_name(e)); return; }
    nc_i2c_bus_t *bus=nc_i2c_espidf_bus(&adapter);
    ESP_LOGI(TAG,"Hardware smoke test; SDA=%d SCL=%d. Override NC_TEST_I2C_* macros for your board.",NC_TEST_I2C_SDA_GPIO,NC_TEST_I2C_SCL_GPIO);
    nc_tca9548a_config_t mux={.bus=bus,.address=NC_TCA9548A_DEFAULT_ADDRESS};
    report("TCA9548A disable all channels",nc_tca9548a_disable_all_channels(&mux));
    nc_ds1307_config_t rtc={.bus=bus,.address=NC_DS1307_DEFAULT_ADDRESS,.timeout_ms=100};
    nc_datetime_t dt;
    e=nc_ds1307_read_datetime(&rtc,&dt);
    if(e==NC_OK) ESP_LOGI(TAG,"DS1307 PASS %04u-%02u-%02u %02u:%02u:%02u",dt.year,dt.month,dt.day,dt.hour,dt.minute,dt.second); else report("DS1307 read time",e);
    nc_ads1115_config_t adc={.bus=bus,.address=NC_ADS1115_DEFAULT_ADDRESS,.timeout_ms=100,.range=NC_ADS1115_FS_4V096,.rate=NC_ADS1115_RATE_128};
    int32_t mv;
    e=nc_ads1115_read_voltage_mv(&adc,0,&mv);
    if(e==NC_OK) ESP_LOGI(TAG,"ADS1115 A0 PASS %ld mV",(long)mv); else report("ADS1115 channel 0",e);
    ESP_LOGI(TAG,"Hardware test complete. Missing-device failures are expected when a device is not connected.");
}