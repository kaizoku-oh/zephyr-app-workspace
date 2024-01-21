/*
Usage example:

// Lib C includes
#include <stdbool.h>

// Zephyr includes
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>

// User C++ class headers
#include "Led.h"

// Reference GPIO devices from device tree
const struct gpio_dt_spec greenLedGpio = GPIO_DT_SPEC_GET_OR(DT_ALIAS(led0), gpios, {0});
const struct gpio_dt_spec blueLedGpio = GPIO_DT_SPEC_GET_OR(DT_ALIAS(led1), gpios, {0});
const struct gpio_dt_spec redLedGpio = GPIO_DT_SPEC_GET_OR(DT_ALIAS(led2), gpios, {0});

// Create local object using the device tree devices
Led greenLed(&greenLedGpio);
Led blueLed(&blueLedGpio);
Led redLed(&redLedGpio);

// Wait forever for an event
while (true) {
  // Toggle LEDs
  greenLed.toggle();
  k_msleep(100);

  blueLed.toggle();
  k_msleep(100);

  redLed.toggle();
  k_msleep(100);
}
*/

#ifndef LED_H
#define LED_H

class Led {

public:
  Led(const struct gpio_dt_spec *gpio);
  ~Led();
  void on();
  void off();
  void toggle();

private:
  const struct gpio_dt_spec *device;

};

#endif // LED_H