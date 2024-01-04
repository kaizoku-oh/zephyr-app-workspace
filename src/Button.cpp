// Zephyr includes
#include <zephyr/drivers/gpio.h>
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(Button);

// User C++ class headers
#include "Button.h"

Button::Button(const struct gpio_dt_spec *gpio) {

  if (gpio == NULL) {
    LOG_ERR("Error: Invalid argument\r\n");
    return;
  }

  this->_device = gpio;

  if (!gpio_is_ready_dt(gpio)) {
    LOG_ERR("Error: button device %s is not ready\n", gpio->port->name);
    return;
  }

  if (gpio_pin_configure_dt(gpio, GPIO_INPUT) != 0) {
    LOG_ERR("Error: Failed to configure %s pin %d\n", gpio->port->name, gpio->pin);
    return;
  }
}

Button::~Button() {
  // Destructor is automatically called when the object goes out of scope or is explicitly deleted
}

bool Button::isPressed() {
  return (gpio_pin_get_dt(this->_device) == 0) ? true : false;
}
