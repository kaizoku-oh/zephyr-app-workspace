/*
Usage example:

// Lib C includes
#include <stdbool.h>

// Zephyr includes
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/sys/printk.h>
#include <zephyr/drivers/gpio.h>

// User C++ class headers
#include "Button.h"

// Thread handler function declaration
static void buttonThreadHandler();

// Delay value used inside thread loops to yield back to scheduler
static constexpr uint32_t BUTTON_THREAD_SLEEP_TIME_MS = 100;

// Threads definition
K_THREAD_DEFINE(buttonThread, 1024, buttonThreadHandler, NULL, NULL, NULL, 7, 0, 0);

static void buttonThreadHandler() {
  // Reference GPIO device from device tree
  const struct gpio_dt_spec buttonGpio = GPIO_DT_SPEC_GET_OR(DT_ALIAS(sw0), gpios, {0});

  // Create local object using the device
  Button button(&buttonGpio);

  // Continuously check if a button is pressed
  while (true) {
    if (button.isPressed()) {
      printk("Button is pressed\r\n");
    }
    k_msleep(BUTTON_THREAD_SLEEP_TIME_MS);
  }
}
*/

#ifndef BUTTON_H
#define BUTTON_H

#include <stdbool.h>

class Button {

public:
  Button(const struct gpio_dt_spec *gpio);
  ~Button();
  bool isPressed();

private:
  const struct gpio_dt_spec *_device;

};

#endif // BUTTON_H
