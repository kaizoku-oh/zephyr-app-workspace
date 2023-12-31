/*
Usage example:

// Lib C includes
#include <stdint.h>
#include <stdbool.h>

// Zephyr includes
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/sys/printk.h>

// User C++ class headers
#include "Serial.h"

// Thread handler function declaration
static void serialThreadHandler();

// Delay value used inside thread loops to yield back to scheduler
static constexpr uint32_t SERIAL_THREAD_SLEEP_TIME_MS = 1000;

// Threads definition
K_THREAD_DEFINE(serialThread, 1024, serialThreadHandler, NULL, NULL, NULL, 7, 0, 0);

static void serialThreadHandler() {
  // Variable to hold serial reading in Celsius
  double serialReading = 0;

  // Reference serial device from device tree
  const struct device *serialDevice = DEVICE_DT_GET(DT_NODELABEL(usart2));

  // Create local object using the device
  Serial serial(serialDevice);

  // Register receive callback as a lambda callback
  serial.onReceive([](uint8_t *data, uint32_t length) {
    printk("Received: %.*s", length, data);
  });

  // Continuously read serial
  while (true) {
    serial.write("Hello world!\r\n");;
    k_msleep(SERIAL_THREAD_SLEEP_TIME_MS);
  }
}
*/

#ifndef SERIAL_H
#define SERIAL_H

#include <stdint.h>
#include <functional>

class Serial {

public:
  const struct device *device;
  std::function<void(uint8_t *, uint32_t)> callback;

  Serial(const struct device *device);
  ~Serial();

  void write(uint8_t *data, uint32_t length);
  void read(uint8_t *data, uint32_t *length);
  void onReceive(std::function<void(uint8_t *, uint32_t)> callback);

private:
};

#endif // SERIAL_H
