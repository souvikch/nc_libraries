# Validation and integration checklist

1. Build with the target ESP-IDF release and resolve compiler/API differences.
2. Configure the board-specific SDA/SCL pins, clock and pull-up policy in the test application.
3. Test TCA9548A channel selection and disable-all.
4. Test DS1307 read/write, BCD conversion, oscillator-stop indication and date limits.
5. Test ADS1115 conversion polling, ranges, data rates and signed voltage conversion.
6. Add atomic mux-routing operations so channel select and downstream transfer share one lock.
7. Bind device properties and readings to the established NC virtual sensor/data bus and registry interfaces.
8. Add the selected SHT model after the exact part number is specified.
9. Add host-side tests with a fake I2C backend and hardware-in-the-loop tests.
10. Do not mark the implementation production-ready until hardware tests, bus-fault behavior and memory usage are measured.
