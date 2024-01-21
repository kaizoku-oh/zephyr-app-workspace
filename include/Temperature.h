/*
Usage example:

// Lib C includes
#include <stdbool.h>

// Zephyr includes
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/sys/printk.h>

// User C++ class headers
#include "Temperature.h"

// Variable to hold temperature reading in Celsius
double temperatureReading = 0;

// Reference die temperature device from device tree
const struct device *temperatureDevice = DEVICE_DT_GET(DT_NODELABEL(die_temp));

// Create local object using the device
Temperature temperature(temperatureDevice);

// Continuously read temperature
while (true) {
  temperatureReading = temperature.read();
  printk("CPU temperature: %.1f °C\r\n", temperatureReading);
  k_msleep(1000);
}
*/

#ifndef TEMPERATURE_H
#define TEMPERATURE_H

class Temperature {

public:
  Temperature(const struct device *device);
  ~Temperature();
  double read();

private:
  const struct device *device;

};

#endif // TEMPERATURE_H
