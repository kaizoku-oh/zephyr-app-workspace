// Lib C
#include <assert.h>

// Zephyr includes
#include <zephyr/device.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/sys/ring_buffer.h>
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(Serial);

// User C++ class headers
#include "Serial.h"

static void serialCallback(const struct device *device, void *userData);

Serial::Serial(const struct device *device) {
  assert(device);

  this->device = device;

  if (!device_is_ready(this->device)) {
    LOG_ERR("Unable to get UART device");
    return;
  }

  uart_irq_rx_enable(this->device);

  ring_buf_init(&this->ringBuffer, sizeof(this->ringBuffer), this->buffer);
}

Serial::~Serial() {
  // Destructor is automatically called when the object goes out of scope or is explicitly deleted
}

void Serial::write(uint8_t *data, uint32_t length) {
  assert(data);
  assert(length);

  while (length--) {
    uart_poll_out(this->device, *data++);
  }
}

void Serial::read(uint8_t *data, uint32_t *length) {
  uint32_t len = 0;

  assert(data);
  assert(length);

  len = *length;
  *length = ring_buf_get(&this->ringBuffer, data, len);
}

void Serial::onReceive(std::function<void(Serial *, SerialEvent)> callback) {
  assert(callback);

  this->callback = callback;
  uart_irq_callback_user_data_set(this->device, serialCallback, this);
}

const struct device * Serial::getDevice() {
  return this->device;
}

struct ring_buf * Serial::getRingBuffer() {
  return &this->ringBuffer;
}

static void serialCallback(const struct device *device, void *userData) {
  int ret = 0;
  uint8_t rxByte = 0;
  Serial *instance = nullptr;

  assert(device);

  instance = static_cast<Serial *>(userData);

  if (device == instance->getDevice()) {
    ret = uart_irq_update(device);
    ret = uart_irq_rx_ready(device);
    ret = uart_fifo_read(device, &rxByte, sizeof(rxByte));

    if (instance) {
      if (instance->callback) {
        ring_buf_put(instance->getRingBuffer(), &rxByte, sizeof(rxByte));
        instance->callback(instance, SerialEvent::RX);
      }
    }
  }
}
