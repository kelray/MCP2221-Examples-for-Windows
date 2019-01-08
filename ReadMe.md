This repository has all examples I have created for the MCP2221 USB-to-I2C/UART chip from Microchip. This chip is like a swiss army knife that you can use to proof of concept any new I2C or UART based system, whether it's a chip or a full embedded system. All examples compile and run under Windows OS, to use the examples under Linux you might need to look into setting up the MCP2221 as an I2C adapter then use existing I2C functions in Linux native C headers.
The examples are for interfacing the MCP2221 via I2C with following sensors/modules:
- MCP4725 12-bit D/A.
- 16x2 I2C Character LCD (PCF8574).
- MCP9808 digital output temperature sensor.
- MMA8452q 3-Axis accelerometer.
- MLX90614 Remote IR temperature sensor.
- INA219 digital output power monitor.
- AMC6821 stand-alone PWM Controller.
