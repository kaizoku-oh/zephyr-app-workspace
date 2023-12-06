#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <inttypes.h>

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/sys/printk.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/net/net_if.h>
#include <zephyr/net/net_core.h>
#include <zephyr/net/net_context.h>
#include <zephyr/net/net_mgmt.h>

#include "Button.h"
#include "Led.h"
#include "Temperature.h"
#include "Serial.h"
#include "Network.h"

#define MAIN_THREAD_SLEEP_TIME_MS        (1000)
#define LED_THREAD_SLEEP_TIME_MS         (500)
#define TEMPERATURE_THREAD_SLEEP_TIME_MS (1000)
#define BUTTON_THREAD_SLEEP_TIME_MS      (100)
#define NETWORK_THREAD_SLEEP_TIME_MS     (1000)

static void led_thread_handler(void);
static void temperature_thread_handler(void);
static void button_thread_handler(void);
static void network_thread_handler(void);

K_THREAD_DEFINE(led_thread, 512, led_thread_handler, NULL, NULL, NULL, 7, 0, 0);
K_THREAD_DEFINE(temperature_thread, 512, temperature_thread_handler, NULL, NULL, NULL, 7, 0, 0);
K_THREAD_DEFINE(button_thread, 512, button_thread_handler, NULL, NULL, NULL, 7, 0, 0);
K_THREAD_DEFINE(network_thread, 512, network_thread_handler, NULL, NULL, NULL, 7, 0, 0);

int main(void) {
  while (true) {
    k_msleep(MAIN_THREAD_SLEEP_TIME_MS);
  }

  return EXIT_SUCCESS;
}

static void led_thread_handler(void) {
  // Reference devices from device tree
  const struct gpio_dt_spec greenLedGpio = GPIO_DT_SPEC_GET_OR(DT_ALIAS(led0), gpios, {0});
  const struct gpio_dt_spec blueLedGpio = GPIO_DT_SPEC_GET_OR(DT_ALIAS(led1), gpios, {0});
  const struct gpio_dt_spec redLedGpio = GPIO_DT_SPEC_GET_OR(DT_ALIAS(led2), gpios, {0});

  // Create local object using the device tree device
  Led greenLed(&greenLedGpio);
  Led blueLed(&blueLedGpio);
  Led redLed(&redLedGpio);

  // Continuously toggle LEDs
  while (true) {
    greenLed.toggle();
    blueLed.toggle();
    redLed.toggle();
    k_msleep(LED_THREAD_SLEEP_TIME_MS);
  }
}

static void temperature_thread_handler(void) {
  // Reference device from device tree
  const struct device *temperatureDevice = DEVICE_DT_GET(DT_NODELABEL(die_temp));

  // Create local object using the device tree device
  Temperature temperature(temperatureDevice);

  // Continuously read temperature
  while (true) {
    printk("CPU temperature: %.1f °C\r\n", temperature.read());
    k_msleep(TEMPERATURE_THREAD_SLEEP_TIME_MS);
  }
}

static void button_thread_handler(void) {
  // Reference device from device tree
  const struct gpio_dt_spec buttonGpio = GPIO_DT_SPEC_GET_OR(DT_ALIAS(sw0), gpios, {0});

  // Create local object using the device tree device
  Button button(&buttonGpio);

  // Continuously check if a button is pressed
  while (true) {
    if (button.isPressed()) {
      printk("Button is pressed\r\n");
    }
    k_msleep(BUTTON_THREAD_SLEEP_TIME_MS);
  }
}

static void network_thread_handler(void) {
  // Get the singleton instance of Network
  Network& network = Network::getInstance();

  // Set up the lambda callback for IP address notification
  network.onGotIP([](const char *ipAddress) {
    printf("Got IP address: %s\r\n", ipAddress);
  });

  // Start the network and wait for an IP address
  network.start();

  while (true) {
    k_msleep(NETWORK_THREAD_SLEEP_TIME_MS);
  }
}
