#include <stdlib.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/drivers/gpio.h>

#include "Button.h"
#include "Led.h"

#define MAIN_THREAD_SLEEP_TIME_MS (100)

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
