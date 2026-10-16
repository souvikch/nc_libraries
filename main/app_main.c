#include "esp_log.h"
#include "nc_common.h"
#include "nc_i2c.h"
#include "nc_tca9548a.h"
#include "nc_ds1307.h"
#include "nc_ads1115.h"
static const char *TAG = "nc_libraries_test";
static void report(const char *name, nc_err_t result) {
    if (result == NC_OK) ESP_LOGI(TAG, "%s: PASS", name);
    else ESP_LOGE(TAG, "%s: FAIL (%s)", name, nc_err_to_name(result));
}
void app_main(void) {
    ESP_LOGI(TAG, "nimbleController shared-library smoke test");
    report("I2C null-bus validation", nc_i2c_bus_validate(NULL) == NC_ERR_INVALID_ARG ? NC_OK : NC_ERR_INTERNAL);
    nc_tca9548a_config_t mux = { .bus = NULL, .address = NC_TCA9548A_DEFAULT_ADDRESS };
    report("TCA9548A rejects missing bus", nc_tca9548a_validate_config(&mux) == NC_ERR_INVALID_ARG ? NC_OK : NC_ERR_INTERNAL);
    nc_ds1307_config_t rtc = { .bus = NULL, .address = NC_DS1307_DEFAULT_ADDRESS };
    report("DS1307 rejects missing bus", nc_ds1307_validate_config(&rtc) == NC_ERR_INVALID_ARG ? NC_OK : NC_ERR_INTERNAL);
    nc_ads1115_config_t adc = { .bus = NULL, .address = NC_ADS1115_DEFAULT_ADDRESS,
        .timeout_ms = 100, .range = NC_ADS1115_FS_2V048, .rate = NC_ADS1115_RATE_128 };
    report("ADS1115 rejects missing bus", nc_ads1115_validate_config(&adc) == NC_ERR_INVALID_ARG ? NC_OK : NC_ERR_INTERNAL);
    ESP_LOGI(TAG, "Smoke tests complete. Initialize nc_i2c_espidf with board-specific GPIOs for hardware tests.");
}