#include <stdlib.h>
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/sys/printk.h>
#include <zephyr/drivers/gpio.h>

#include "Button.h"
#include "Led.h"
#include "Temperature.h"

#define MAIN_THREAD_SLEEP_TIME_MS (100)

int main(void) {
  const struct gpio_dt_spec buttonGpio = GPIO_DT_SPEC_GET_OR(DT_ALIAS(sw0), gpios, {0});

  const struct gpio_dt_spec redLedGpio = GPIO_DT_SPEC_GET_OR(DT_ALIAS(led2), gpios, {0});
  const struct gpio_dt_spec greenLedGpio = GPIO_DT_SPEC_GET_OR(DT_ALIAS(led0), gpios, {0});
  const struct gpio_dt_spec blueLedGpio = GPIO_DT_SPEC_GET_OR(DT_ALIAS(led1), gpios, {0})
  
  const struct device *temperatureDevice = DEVICE_DT_GET(DT_NODELABEL(die_temp));

  Button button(&buttonGpio);

  Led greenLed(&greenLedGpio);
  Led redLed(&redLedGpio);
  Led blueLed(&blueLedGpio);

  Temperature temperature(temperatureDevice);

  while (true) {
    if (button.isPressed()) {
      printk("Button is pressed\r\n");
      printk("CPU temperature: %.1f °C\r\n", temperature.read());
      redLed.toggle();
      greenLed.toggle();
      blueLed.toggle();
    }
    k_msleep(MAIN_THREAD_SLEEP_TIME_MS);
  }

  return EXIT_SUCCESS;
}
