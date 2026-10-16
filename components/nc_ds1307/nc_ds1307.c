#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "nc_ds1307.h"

#define DS1307_REG_SECONDS 0x00U
#define DS1307_REG_MINUTES 0x01U
#define DS1307_REG_HOURS   0x02U
#define DS1307_REG_WEEKDAY 0x03U
#define DS1307_REG_DATE    0x04U
#define DS1307_REG_MONTH   0x05U
#define DS1307_REG_YEAR    0x06U
#define DS1307_REG_COUNT   7U

static uint8_t bcd_to_bin(uint8_t value)
{
    return (uint8_t)(((value >> 4U) * 10U) + (value & 0x0FU));
}

static uint8_t bin_to_bcd(uint8_t value)
{
    return (uint8_t)(((value / 10U) << 4U) | (value % 10U));
}

static bool valid_bcd(uint8_t value)
{
    return ((value & 0x0FU) <= 9U) && (((value >> 4U) & 0x0FU) <= 9U);
}

static bool valid_calendar_date(uint16_t year, uint8_t month, uint8_t day)
{
    static const uint8_t days_per_month[12] = {
        31U, 28U, 31U, 30U, 31U, 30U,
        31U, 31U, 30U, 31U, 30U, 31U
    };

    if (year < 2000U || year > 2099U || month < 1U || month > 12U || day < 1U)
        return false;

    uint8_t maximum = days_per_month[month - 1U];
    const bool leap = (year % 4U) == 0U;
    if (month == 2U && leap)
        maximum = 29U;

    return day <= maximum;
}

static bool valid_datetime(const nc_datetime_t *dt)
{
    if (!dt || !valid_calendar_date(dt->year, dt->month, dt->day) ||
        dt->weekday < 1U || dt->weekday > 7U ||
        dt->hour > 23U || dt->minute > 59U || dt->second > 59U)
        return false;

    return true;
}

nc_err_t nc_ds1307_validate_config(const nc_ds1307_config_t *config)
{
    if (!config || !config->bus || config->address != NC_DS1307_DEFAULT_ADDRESS)
        return NC_ERR_INVALID_ARG;

    return nc_i2c_bus_validate(config->bus);
}

nc_err_t nc_ds1307_read_datetime(const nc_ds1307_config_t *config, nc_datetime_t *datetime)
{
    nc_err_t error = nc_ds1307_validate_config(config);
    if (error != NC_OK)
        return error;
    if (!datetime)
        return NC_ERR_INVALID_ARG;

    uint8_t register_address = DS1307_REG_SECONDS;
    uint8_t registers[DS1307_REG_COUNT] = {0};
    error = nc_i2c_write_read(config->bus, config->address,
                              &register_address, 1U,
                              registers, sizeof(registers), config->timeout_ms);
    if (error != NC_OK)
        return error;

    /* CH=1 means the oscillator is stopped; do not report this time as valid. */
    if ((registers[0] & 0x80U) != 0U)
        return NC_ERR_NOT_INITIALIZED;

    const uint8_t seconds_bcd = (uint8_t)(registers[0] & 0x7FU);
    const uint8_t minutes_bcd = (uint8_t)(registers[1] & 0x7FU);
    const uint8_t hours_reg = registers[2];
    const uint8_t weekday_bcd = (uint8_t)(registers[3] & 0x07U);
    const uint8_t date_bcd = (uint8_t)(registers[4] & 0x3FU);
    const uint8_t month_bcd = (uint8_t)(registers[5] & 0x1FU);
    const uint8_t year_bcd = registers[6];

    if (!valid_bcd(seconds_bcd) || !valid_bcd(minutes_bcd) ||
        !valid_bcd(weekday_bcd) || !valid_bcd(date_bcd) ||
        !valid_bcd(month_bcd) || !valid_bcd(year_bcd))
        return NC_ERR_IO;

    datetime->second = bcd_to_bin(seconds_bcd);
    datetime->minute = bcd_to_bin(minutes_bcd);

    if ((hours_reg & 0x40U) != 0U)
    {
        /* 12-hour mode: bit 5 is PM; the hour field must be 1..12. */
        const uint8_t hour_bcd = (uint8_t)(hours_reg & 0x1FU);
        if (!valid_bcd(hour_bcd))
            return NC_ERR_IO;
        const uint8_t hour12 = bcd_to_bin(hour_bcd);
        if (hour12 < 1U || hour12 > 12U)
            return NC_ERR_IO;
        datetime->hour = (uint8_t)((hour12 % 12U) +
                                   (((hours_reg & 0x20U) != 0U) ? 12U : 0U));
    }
    else
    {
        const uint8_t hour_bcd = (uint8_t)(hours_reg & 0x3FU);
        if (!valid_bcd(hour_bcd))
            return NC_ERR_IO;
        datetime->hour = bcd_to_bin(hour_bcd);
    }

    datetime->weekday = bcd_to_bin(weekday_bcd);
    datetime->day = bcd_to_bin(date_bcd);
    datetime->month = bcd_to_bin(month_bcd);
    datetime->year = (uint16_t)(2000U + bcd_to_bin(year_bcd));

    if (datetime->second > 59U || datetime->minute > 59U ||
        datetime->hour > 23U || datetime->weekday < 1U ||
        datetime->weekday > 7U ||
        !valid_calendar_date(datetime->year, datetime->month, datetime->day))
        return NC_ERR_IO;

    return NC_OK;
}

nc_err_t nc_ds1307_set_datetime(const nc_ds1307_config_t *config, const nc_datetime_t *datetime)
{
    nc_err_t error = nc_ds1307_validate_config(config);
    if (error != NC_OK)
        return error;
    if (!valid_datetime(datetime))
        return NC_ERR_INVALID_ARG;

    /* Write starting at seconds; writing CH=0 starts the oscillator. */
    const uint8_t registers[1U + DS1307_REG_COUNT] = {
        DS1307_REG_SECONDS,
        bin_to_bcd(datetime->second),
        bin_to_bcd(datetime->minute),
        bin_to_bcd(datetime->hour), /* force 24-hour mode */
        bin_to_bcd(datetime->weekday),
        bin_to_bcd(datetime->day),
        bin_to_bcd(datetime->month),
        bin_to_bcd((uint8_t)(datetime->year - 2000U))
    };

    return nc_i2c_write(config->bus, config->address,
                        registers, sizeof(registers), config->timeout_ms);
}

nc_err_t nc_ds1307_is_oscillator_running(const nc_ds1307_config_t *config, bool *running)
{
    nc_err_t error = nc_ds1307_validate_config(config);
    if (error != NC_OK)
        return error;
    if (!running)
        return NC_ERR_INVALID_ARG;

    uint8_t register_address = DS1307_REG_SECONDS;
    uint8_t seconds = 0U;
    error = nc_i2c_write_read(config->bus, config->address,
                              &register_address, 1U, &seconds, 1U,
                              config->timeout_ms);
    if (error == NC_OK)
        *running = (seconds & 0x80U) == 0U;

    return error;
}
