This repository has all examples I have developed for the MCP2221 USB-to-I2C/UART chip from Microchip. This chip is like a swiss army knife that you can use to proof of concept any new I2C or UART based system, whether it's a chip or a full embedded system. 
All examples compile and run under Windows OS, all examples use Microchip library and DLL for the MCP2221 available at: https://www.microchip.com/wwwproducts/en/MCP2221A

The examples are for interfacing the MCP2221 via I2C with following sensors/modules:
- MCP4725 12-bit D/A.
- 16x2 I2C Character LCD (PCF8574).
- MCP9808 digital output temperature sensor.
- MMA8452q 3-Axis accelerometer.
- MLX90614 Remote IR temperature sensor.
- INA219 digital output power monitor.
- AMC6821 stand-alone PWM Controller.
- MCP23008 and MCP23018 I2C I/O Expanders.
- I2C slave devices scanner (scans I2C bus and lists all connected devices).

To use the examples under Linux you might need to look into setting up the MCP2221 as an I2C adapter then use existing I2C functions in Linux native C headers. These are the steps to set up the MCP2221 as an I2C adapter (tested on Raspberry Pi):
- sudo apt-get upgrade
- sudo apt-get install raspberrypi-kernel-headers
- wget http://ww1.microchip.com/downloads/en/DeviceDoc/mcp2221_0_1.tar.gz
- tar -xf mcp2221_0_1.tar.gz
- cd /"mcp2221_directory"/
- make modules
- sudo make install
- Override linux driver with MCP2221 driver by creating usb device rule: sudo nano /etc/udev/rules.d/100-usb.rules
- Add this line:  ACTION=="add", SUBSYSTEM=="usb", DRIVER=="usb", RUN+="/directory/to/mcp2221/driver/driver_load.sh"
- Restart udev services: sudo service udev restart



