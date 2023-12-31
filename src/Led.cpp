// Zephyr includes
#include <zephyr/drivers/gpio.h>
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(Led);

// User C++ class headers
#include "Led.h"

Led::Led(const struct gpio_dt_spec *gpio) {
  if (gpio == NULL) {
    LOG_ERR("Error: Invalid argument\r\n");
    return;
  }

  this->_device = gpio;

  if (!gpio_is_ready_dt(gpio)) {
    LOG_ERR("Error: Led device %s is not ready\n", gpio->port->name);
    return;
  }

  if (gpio_pin_configure_dt(gpio, GPIO_OUTPUT_ACTIVE) != 0) {
    LOG_ERR("Error: Failed to configure %s pin %d\n", gpio->port->name, gpio->pin);
    return;
  }
}

Led::~Led() {
  // Destructor is automatically called when the object goes out of scope or is explicitly deleted
}

void Led::on() {
  gpio_pin_set_dt(this->_device, 1);
}

void Led::off() {
  gpio_pin_set_dt(this->_device, 0);
}

void Led::toggle() {
  int ret = 0;

  ret = gpio_pin_toggle_dt(this->_device);
  if (ret < 0) {
    LOG_ERR("Failed to toggle LED pin!\r\n");
    return;
  }
}

void Led::blink(int dutyCycle) {
}
