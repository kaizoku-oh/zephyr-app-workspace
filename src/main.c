#include <stdlib.h>
#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>

#define LED_NODE      DT_ALIAS(led0)
#define SLEEP_TIME_MS (300)

static const struct gpio_dt_spec greenLED = GPIO_DT_SPEC_GET(LED_NODE, gpios);

int main(void)
{
  int ret;

  /* Validate that GPIO port is ready */
  ret = gpio_is_ready_dt(&greenLED);
  if (!ret) {
    return EXIT_FAILURE;
  }

  /* Configure a single pin */
  ret = gpio_pin_configure_dt(&greenLED, GPIO_OUTPUT_ACTIVE);
  if (ret < 0) {
    return EXIT_FAILURE;
  }

  /* Blink LED each SLEEP_TIME_MS */
  while (true) {
    ret = gpio_pin_toggle_dt(&greenLED);
    if (ret < 0) {
      return EXIT_FAILURE;
    }
    k_msleep(SLEEP_TIME_MS);
  }

  return EXIT_SUCCESS;
}
