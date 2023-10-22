#include <stdlib.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/drivers/gpio.h>
#ifdef DIE_TEMPERATURE
#include <zephyr/device.h>
#include <zephyr/drivers/sensor.h>
#endif // DIE_TEMPERATURE

#include "Button.h"
#include "Led.h"

#define MAIN_THREAD_SLEEP_TIME_MS (100)

#ifdef DIE_TEMPERATURE
static void test_die_temperature();
#endif // DIE_TEMPERATURE

int main(void) {
  const struct gpio_dt_spec buttonGpio = GPIO_DT_SPEC_GET_OR(DT_ALIAS(sw0), gpios, {0});

  const struct gpio_dt_spec redLedGpio = GPIO_DT_SPEC_GET_OR(DT_ALIAS(led2), gpios, {0});
  const struct gpio_dt_spec greenLedGpio = GPIO_DT_SPEC_GET_OR(DT_ALIAS(led0), gpios, {0});
  const struct gpio_dt_spec blueLedGpio = GPIO_DT_SPEC_GET_OR(DT_ALIAS(led1), gpios, {0});

  Button button(&buttonGpio);

  Led greenLed(&greenLedGpio);
  Led redLed(&redLedGpio);
  Led blueLed(&blueLedGpio);

  while (true) {
    if (button.isPressed()) {
      printk("Button is pressed\r\n");
      redLed.toggle();
      greenLed.toggle();
      blueLed.toggle();
    }
    k_msleep(MAIN_THREAD_SLEEP_TIME_MS);
  }

  return EXIT_SUCCESS;
}

#ifdef DIE_TEMPERATURE
static void test_die_temperature() {
  const struct device *const sensorDevice = DEVICE_DT_GET(DT_NODELABEL(die_temp));

  int ret = 0;
  struct sensor_value value = {0};

  if (device_is_ready(sensorDevice)) {
    return;
  }

  while (true) {
    ret = sensor_sample_fetch(sensorDevice);
    if (ret) {
      printk("Failed to fetch sample (%d)\n", ret);
      return;
    }

    ret = sensor_channel_get(sensorDevice, SENSOR_CHAN_DIE_TEMP, &value);
    if (ret) {
      printk("Failed to get data (%d)\n", ret);
      return;
    }

    printk("CPU Die temperature: %.1f °C\r\n", sensor_value_to_double(&value));

    k_msleep(MAIN_THREAD_SLEEP_TIME_MS);
  }
}
#endif // DIE_TEMPERATURE
