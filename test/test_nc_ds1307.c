#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "nc_ds1307.h"

typedef struct {
    uint8_t registers[7];
    uint8_t last_write[8];
    size_t last_write_length;
    nc_err_t next_error;
} fake_i2c_t;

static nc_err_t fake_write_read(nc_i2c_bus_t *bus, uint8_t address,
                                const uint8_t *write_data, size_t write_length,
                                uint8_t *read_data, size_t read_length,
                                uint32_t timeout_ms)
{
    (void)address; (void)timeout_ms;
    fake_i2c_t *fake = (fake_i2c_t *)bus->context;
    if (fake->next_error != NC_OK) {
        nc_err_t e = fake->next_error;
        fake->next_error = NC_OK;
        return e;
    }
    if (!write_data || write_length != 1U || write_data[0] >= 7U ||
        !read_data || read_length > (size_t)(7U - write_data[0]))
        return NC_ERR_INVALID_ARG;
    memcpy(read_data, &fake->registers[write_data[0]], read_length);
    return NC_OK;
}

static nc_err_t fake_write(nc_i2c_bus_t *bus, uint8_t address,
                           const uint8_t *data, size_t length, uint32_t timeout_ms)
{
    (void)address; (void)timeout_ms;
    fake_i2c_t *fake = (fake_i2c_t *)bus->context;
    if (fake->next_error != NC_OK) {
        nc_err_t e = fake->next_error;
        fake->next_error = NC_OK;
        return e;
    }
    if (!data || length == 0U || length > sizeof(fake->last_write))
        return NC_ERR_INVALID_ARG;
    memcpy(fake->last_write, data, length);
    fake->last_write_length = length;
    if (data[0] == 0U && length == 8U)
        memcpy(fake->registers, &data[1], 7U);
    return NC_OK;
}

static const nc_i2c_ops_t fake_ops = {
    .write_read = fake_write_read,
    .write = fake_write,
    .delay_ms = NULL
};

static void set_valid_clock(fake_i2c_t *fake)
{
    const uint8_t registers[7] = {0x56, 0x34, 0x12, 0x03, 0x29, 0x02, 0x24};
    memcpy(fake->registers, registers, sizeof(registers));
}

int main(void)
{
    fake_i2c_t fake = {0};
    nc_i2c_bus_t bus = {.context = &fake, .ops = &fake_ops};
    nc_ds1307_config_t config = {
        .bus = &bus, .address = NC_DS1307_DEFAULT_ADDRESS, .timeout_ms = 25
    };
    nc_datetime_t dt = {0};

    set_valid_clock(&fake);
    assert(nc_ds1307_read_datetime(&config, &dt) == NC_OK);
    assert(dt.year == 2024 && dt.month == 2 && dt.day == 29);
    assert(dt.weekday == 3 && dt.hour == 12 && dt.minute == 34 && dt.second == 56);

    fake.registers[0] = 0x80;
    assert(nc_ds1307_read_datetime(&config, &dt) == NC_ERR_NOT_INITIALIZED);

    set_valid_clock(&fake);
    fake.registers[1] = 0x6A;
    assert(nc_ds1307_read_datetime(&config, &dt) == NC_ERR_IO);

    dt = (nc_datetime_t){.year=2023, .month=2, .day=29, .weekday=3,
                         .hour=12, .minute=0, .second=0};
    assert(nc_ds1307_set_datetime(&config, &dt) == NC_ERR_INVALID_ARG);

    dt.year = 2024;
    assert(nc_ds1307_set_datetime(&config, &dt) == NC_OK);
    assert(fake.last_write_length == 8U);
    assert(fake.last_write[0] == 0U);
    assert(fake.registers[4] == 0x29);

    bool running = true;
    fake.registers[0] = 0x80;
    assert(nc_ds1307_is_oscillator_running(&config, &running) == NC_OK);
    assert(!running);

    fake.next_error = NC_ERR_IO;
    assert(nc_ds1307_is_oscillator_running(&config, &running) == NC_ERR_IO);

    puts("DS1307 host tests: PASS");
    return 0;
}
