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

// Reference serial device from device tree
const struct device *serialDevice = DEVICE_DT_GET(DT_NODELABEL(usart2));

// Create local object using the device
Serial serial(serialDevice);

// Register receive callback as a lambda callback
serial.onReceive([](Serial *serial, SerialEvent event) {
  uint8_t receivedByte = 0;
  uint32_t length = 0;

  switch (event) {
    case SerialEvent::RX:
      serial->read(&receivedByte, &length);
      if (length) {
        LOG_INF("Received byte: %c", receivedByte);
      } else {
        LOG_ERR("Failed to read received byte");
      }
      break;
    case SerialEvent::TX:
    case SerialEvent::ERROR:
    default:
      break;
  }
});

// Continuously write on serial
while (true) {
  serial.write((uint8_t *)"Hello world!\r\n", sizeof("Hello world!\r\n")-1);
  k_msleep(1000);
}
*/

#ifndef SERIAL_H
#define SERIAL_H

// Lib C
#include <stdint.h>
#include <functional>

// Zephyr includes
#include <zephyr/sys/ring_buffer.h>

#ifndef SERIAL_RING_BUFFER_SIZE
#define SERIAL_RING_BUFFER_SIZE (1024U)
#endif // SERIAL_RING_BUFFER_SIZE

enum class SerialEvent {
  RX,
  TX,
  ERROR,
};

class Serial {

public:
  std::function<void(Serial *, SerialEvent)> callback;

  Serial(const struct device *device);
  ~Serial();

  void write(uint8_t *data, uint32_t length);
  void read(uint8_t *data, uint32_t *length);
  void onReceive(std::function<void(Serial *, SerialEvent)> callback);

  const struct device * getDevice();
  struct ring_buf * getRingBuffer();

private:
  const struct device *device;
  uint8_t buffer[SERIAL_RING_BUFFER_SIZE];
  struct ring_buf ringBuffer;
};

#endif // SERIAL_H
