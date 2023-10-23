#include <zephyr/drivers/gpio.h>
#include <zephyr/sys/printk.h>

#include "Button.h"

Button::Button(const struct gpio_dt_spec *gpio) {

  if (gpio == NULL) {
    printk("Error: Invalid argument\r\n");
    return;
  }

  _gpioDevice = gpio;

  if (!gpio_is_ready_dt(gpio)) {
    printk("Error: button device %s is not ready\n", gpio->port->name);
    return;
  }

  if (gpio_pin_configure_dt(gpio, GPIO_INPUT) != 0) {
    printk("Error: Failed to configure %s pin %d\n", gpio->port->name, gpio->pin);
    return;
  }
}

Button::~Button() {
  // Destructor is automatically called when the object goes out of scope or is explicitly deleted
}

bool Button::isPressed() {
  return (gpio_pin_get_dt(_gpioDevice) == 0) ? true : false;
}
