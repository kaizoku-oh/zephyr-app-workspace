#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <inttypes.h>

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/sys/printk.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/sys/ring_buffer.h>

#include "Button.h"
#include "Led.h"
#include "Temperature.h"
#include "Serial.h"

#define MAIN_THREAD_SLEEP_TIME_MS (100)
#define RING_BUFFER_SIZE (1024)

static struct ring_buf ringBuffer;
static uint8_t buffer[RING_BUFFER_SIZE];

int main(void) {
  // Local variables used to read data from ring buffer
  uint8_t rxData[8] = {0};
  uint8_t length = 0;

  // Reference devices from device tree
  const struct gpio_dt_spec buttonGpio = GPIO_DT_SPEC_GET_OR(DT_ALIAS(sw0), gpios, {0});
  const struct gpio_dt_spec greenLedGpio = GPIO_DT_SPEC_GET_OR(DT_ALIAS(led0), gpios, {0});
  const struct gpio_dt_spec blueLedGpio = GPIO_DT_SPEC_GET_OR(DT_ALIAS(led1), gpios, {0});
  const struct gpio_dt_spec redLedGpio = GPIO_DT_SPEC_GET_OR(DT_ALIAS(led2), gpios, {0});
  const struct device *temperatureDevice = DEVICE_DT_GET(DT_NODELABEL(die_temp));
  const struct device *serialDevice = DEVICE_DT_GET(DT_NODELABEL(usart2));

  // Create local objects using the device tree devices
  Button button(&buttonGpio);
  Led greenLed(&greenLedGpio);
  Led blueLed(&blueLedGpio);
  Led redLed(&redLedGpio);
  Temperature temperature(temperatureDevice);
  Serial serial(serialDevice);

  // Link buffer buffer to ringBuffer and initialize them
  ring_buf_init(&ringBuffer, sizeof(buffer), buffer);

  // Register serial callback as a lambda function
  serial.onReceive([](uint8_t *data, uint32_t length) {
    ring_buf_put(&ringBuffer, data, length);
  });

  // Write a string over serial
  serial.write((uint8_t *)"Hello world!\r\n", (sizeof("Hello world!\r\n") - 1));

  // Continuously check if a button is pressed, if so read temperature and toggle LEDs
  while (true) {
    if (button.isPressed()) {
      printk("Button is pressed\r\n");
      printk("CPU temperature: %.1f °C\r\n", temperature.read());
      length = ring_buf_get(&ringBuffer, rxData, sizeof(rxData));
      printk("Received data: %.*s\r\n", length, rxData);
      redLed.toggle();
      greenLed.toggle();
      blueLed.toggle();
    }
    k_msleep(MAIN_THREAD_SLEEP_TIME_MS);
  }

  return EXIT_SUCCESS;
}
