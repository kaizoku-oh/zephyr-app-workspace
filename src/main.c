#include <zephyr/drivers/gpio.h>

#define SLEEP_TIME_MS (3000)
#define LED_1_NODE    DT_ALIAS(led1)

static const struct gpio_dt_spec blueLED = GPIO_DT_SPEC_GET(LED_1_NODE, gpios);

int main(void)
{
  while (true) {
    gpio_pin_toggle_dt(&blueLED);
    k_msleep(SLEEP_TIME_MS);
  }

  return 0;
}