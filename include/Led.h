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
#include "Led.h"

// Thread handler function declaration
static void ledThreadHandler();

// Delay value used inside thread loops to yield back to scheduler
static constexpr uint32_t HTTP_CLIENT_THREAD_SLEEP_TIME_MS = 100;

// Threads definition
K_THREAD_DEFINE(ledThread, 1024, ledThreadHandler, NULL, NULL, NULL, 7, 0, 0);

static void ledThreadHandler() {
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
    k_msleep(LED_THREAD_SLEEP_TIME_MS);

    blueLed.toggle();
    k_msleep(LED_THREAD_SLEEP_TIME_MS);

    redLed.toggle();
    k_msleep(LED_THREAD_SLEEP_TIME_MS);
  }
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
  void blink(int dutyCycle);

private:
  int _dutyCycle;
  const struct gpio_dt_spec *_device;

};

#endif // LED_H