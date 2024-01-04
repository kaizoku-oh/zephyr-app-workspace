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

// Thread handler function declaration
static void temperatureThreadHandler();

// Delay value used inside thread loops to yield back to scheduler
static constexpr uint32_t TEMPERATURE_THREAD_SLEEP_TIME_MS = 1000;

// Threads definition
K_THREAD_DEFINE(temperatureThread, 1024, temperatureThreadHandler, NULL, NULL, NULL, 7, 0, 0);

static void temperatureThreadHandler() {
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
    k_msleep(TEMPERATURE_THREAD_SLEEP_TIME_MS);
  }
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
  const struct device *_device;

};

#endif // TEMPERATURE_H
