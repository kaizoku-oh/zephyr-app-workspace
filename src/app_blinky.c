#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/drivers/gpio.h>

#define BLINKY_LED_NODE DT_ALIAS(led0)
#define BLINKY_THREAD_SLEEP_TIME_MS (300)

typedef enum {
  BLINKY_EVENT_TOGGLE_LED = 0,
  BLINKY_EVENT_MAX,
} blinky_event_t;

static void blinky_thread_entry(void);
static void blinky_thread_notify(uint32_t event);
static void blinky_timer_elapsed_cb(struct k_timer *timer_id);
static void configure_led(const struct gpio_dt_spec gpio);
static void toggle_led(const struct gpio_dt_spec gpio);

K_THREAD_DEFINE(blinky_thread, 1024, blinky_thread_entry, NULL, NULL, NULL, 7, 0, 0);
K_MSGQ_DEFINE(blinky_msg_queue, sizeof(uint32_t), 10, 1);
K_TIMER_DEFINE(blinky_timer, blinky_timer_elapsed_cb, NULL);

static void blinky_thread_entry(void)
{
  blinky_event_t event = BLINKY_EVENT_MAX;
  const struct gpio_dt_spec greenLED = GPIO_DT_SPEC_GET(BLINKY_LED_NODE, gpios);

  configure_led(greenLED);
  k_timer_start(&blinky_timer, K_MSEC(100), K_MSEC(100));

  while (true) {
    if (k_msgq_get(&blinky_msg_queue, &event, K_FOREVER) == 0) {
      switch (event) {
        case BLINKY_EVENT_TOGGLE_LED: {
          toggle_led(greenLED);
          break;
        }
        default: {
          break;
        }
      }
    }
  }
}

static void blinky_thread_notify(uint32_t event)
{
  if (k_msgq_put(&blinky_msg_queue, &event, K_NO_WAIT) != 0) {
    printk("Failed to notify blinky thread!\r\n");
  }
}

static void blinky_timer_elapsed_cb(struct k_timer *timer)
{
  blinky_thread_notify(BLINKY_EVENT_TOGGLE_LED);
}

static void configure_led(const struct gpio_dt_spec gpio)
{
  int ret = 0;

  ret = gpio_is_ready_dt(&gpio);
  if (!ret) {
    printk("GPIO port is not ready!\r\n");
    return;
  }

  ret = gpio_pin_configure_dt(&gpio, GPIO_OUTPUT_ACTIVE);
  if (ret < 0) {
    printk("Failed to configure pin!\r\n");
    return;
  }
}

static void toggle_led(const struct gpio_dt_spec gpio)
{
  int ret = 0;

  ret = gpio_pin_toggle_dt(&gpio);
  if (ret < 0) {
    printk("Failed to toggle GPIO pin!\r\n");
    return;
  }
}