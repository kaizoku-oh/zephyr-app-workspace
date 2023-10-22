#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/sys/printk.h>

#define BUTTON_NODE DT_ALIAS(sw0)

typedef enum {
  BUTTON_EVENT_PRESSED = 0,
  BUTTON_EVENT_MAX,
} button_event_t;

static void button_thread_entry(void);
static void button_notify_thread(uint32_t event);
static void button_pressed_isr(const struct device *dev, struct gpio_callback *cb, uint32_t pins);
static void button_configure(const struct gpio_dt_spec *gpio, struct gpio_callback *button_cb_data);

K_THREAD_DEFINE(button_thread, 1024, button_thread_entry, NULL, NULL, NULL, 7, 0, 0);
K_MSGQ_DEFINE(button_msg_queue, sizeof(uint32_t), 10, 1);

static void button_thread_entry(void)
{
  button_event_t event = BUTTON_EVENT_MAX;
  struct gpio_callback button_cb_data = {0};
  const struct gpio_dt_spec button = GPIO_DT_SPEC_GET_OR(BUTTON_NODE, gpios, {0});

  button_configure(&button, &button_cb_data);

  while (true) {
    if (k_msgq_get(&button_msg_queue, &event, K_FOREVER) == 0) {
      switch (event) {
        case BUTTON_EVENT_PRESSED: {
          printk("Button is pressed!\r\n");
          break;
        }
        default: {
          break;
        }
      }
    }
  }
}

static void button_notify_thread(uint32_t event)
{
  if (k_msgq_put(&button_msg_queue, &event, K_NO_WAIT) != 0) {
    printk("Failed to notify button thread!\r\n");
  }
}

static void button_pressed_isr(const struct device *dev, struct gpio_callback *cb, uint32_t pins)
{
  button_notify_thread(BUTTON_EVENT_PRESSED);
}

static void button_configure(const struct gpio_dt_spec *gpio, struct gpio_callback *button_cb_data)
{
  int ret = 0;

  if (!gpio_is_ready_dt(gpio)) {
    printk("Error: button device %s is not ready\n", gpio->port->name);
    return;
  }

  ret = gpio_pin_configure_dt(gpio, GPIO_INPUT);
  if (ret != 0) {
    printk("Error %d: failed to configure %s pin %d\n", ret, gpio->port->name, gpio->pin);
    return;
  }

  ret = gpio_pin_interrupt_configure_dt(gpio, GPIO_INT_EDGE_TO_ACTIVE);
  if (ret != 0) {
    printk("Error %d: failed to configure interrupt on %s pin %d\n",
            ret,
            gpio->port->name,
            gpio->pin);
    return;
  }

  gpio_init_callback(button_cb_data, button_pressed_isr, BIT(gpio->pin));
  ret = gpio_add_callback(gpio->port, button_cb_data);
  if (ret < 0) {
    printk("Error %d: failed to add GPIO callback\n", ret);
    return;
  }
}
