#include <zephyr/device.h>
#include <zephyr/sys/printk.h>
#include <zephyr/drivers/uart.h>

#include "Serial.h"

static void serialCallback(const struct device *dev, void *userData);

Serial::Serial(const struct device *device) {
  if (device == NULL) {
    printk("Error: Invalid argument\r\n");
    return;
  }

  this->device = device;

  if (!device_is_ready(this->device)) {
    printk("Unable to get UART device\r\n");
    return;
  }

  uart_irq_rx_enable(this->device);
}

Serial::~Serial() {
  // Destructor is automatically called when the object goes out of scope or is explicitly deleted
}

void Serial::write(uint8_t *data, uint32_t length) {
  size_t index;

  if (data && length) {
    for (index = 0; index < length; index++) {
      uart_poll_out(this->device, *data++);
    }
  }
}

void Serial::read(uint8_t *data, uint32_t *length) {
}

void Serial::onReceive(std::function<void(uint8_t*, uint32_t)> callback) {
  int ret;

  if (callback == nullptr) {
    printk("Failed to register callback\r\n");
    return;
  }

  this->callback = callback;

  ret = uart_irq_callback_user_data_set(this->device, serialCallback, this);
  if (ret < 0) {
    if (ret == -ENOTSUP) {
      printk("Interrupt-driven UART API support not enabled\r\n");
    } else if (ret == -ENOSYS) {
      printk("UART device does not support interrupt-driven API\r\n");
    } else {
      printk("Error setting UART callback: %d\r\n", ret);
    }
    return;
  }
}

static void serialCallback(const struct device *dev, void *userData) {
  Serial *serialInstance = nullptr;
  uint8_t rxByte = 0;
  int ret = 0;

  if ((dev == nullptr) || (userData == nullptr)) {
    printk("Invalid callback parameters\r\n");
    return;
  }

  serialInstance = static_cast<Serial *>(userData);

  ret = uart_irq_update(dev);
  if (ret < 0) {
    if (ret == -ENOSYS) {
      printk("uart_irq_update() function is not implemented\r\n");
    } else if (ret == -ENOTSUP) {
      printk("UART API is not enabled");
    }
    return;
  }

  ret = uart_irq_rx_ready(dev);
  if (ret < 0) {
    if (ret == -ENOSYS) {
      printk("uart_irq_rx_ready() function is not implemented\r\n");
    } else if (ret == -ENOTSUP) {
      printk("UART API is not enabled");
    }
    return;
  }

  ret = uart_fifo_read(dev, &rxByte, sizeof(rxByte));
  if (ret == 1) {
    if (serialInstance->callback) {
      serialInstance->callback(&rxByte, ret);
    }
  } else if (ret == 0) {
    printk("Got a UART RX interrupt but FIFO is empty!\r\n");
  } else if (ret > 1) {
    printk("Didn't expect to find more than 1 byte in FIFO!\r\n");
  } else if (ret == -ENOSYS) {
    printk("uart_fifo_read() function is not implemented\r\n");
  } else if (ret == -ENOTSUP) {
    printk("UART API is not enabled");
  } else {
    printk("Unknown error: %d\r\n", ret);
  }
}
