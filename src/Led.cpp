#include <zephyr/drivers/gpio.h>
#include <zephyr/sys/printk.h>

#include "Led.h"

Led::Led(const struct gpio_dt_spec *gpio) {
  if (gpio == NULL) {
    printk("Error: Invalid argument\r\n");
    return;
  }

  this->_gpioDevice = gpio;

  if (!gpio_is_ready_dt(gpio)) {
    printk("Error: Led device %s is not ready\n", gpio->port->name);
    return;
  }

  if (gpio_pin_configure_dt(gpio, GPIO_OUTPUT_ACTIVE) != 0) {
    printk("Error: Failed to configure %s pin %d\n", gpio->port->name, gpio->pin);
    return;
  }
}

Led::~Led() {
  // Destructor is automatically called when the object goes out of scope or is explicitly deleted
}

void Led::on() {
  gpio_pin_set_dt(this->_gpioDevice, 1);
}

void Led::off() {
  gpio_pin_set_dt(this->_gpioDevice, 0);
}

void Led::toggle() {
  int ret = 0;

  ret = gpio_pin_toggle_dt(this->_gpioDevice);
  if (ret < 0) {
    printk("Failed to toggle LED pin!\r\n");
    return;
  }
}

void Led::blink(int dutyCycle) {
}
