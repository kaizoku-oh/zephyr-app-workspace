// Lib C
#include <assert.h>

// Zephyr includes
#include <zephyr/drivers/gpio.h>
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(Button);

// User C++ class headers
#include "Button.h"

static void gpioCallback(const struct device *dev, struct gpio_callback *callbackData, uint32_t pins);

Button::Button(const struct gpio_dt_spec *gpio) {
  assert(gpio);

  this->gpio = gpio;

  if (!gpio_is_ready_dt(gpio)) {
    LOG_ERR("Error: button device %s is not ready", gpio->port->name);
    return;
  }

  if (gpio_pin_configure_dt(gpio, GPIO_INPUT) != 0) {
    LOG_ERR("Error: Failed to configure %s pin %d", gpio->port->name, gpio->pin);
    return;
  }
}

Button::~Button() {
  // Destructor is automatically called when the object goes out of scope or is explicitly deleted
}

bool Button::isPressed(Polarity polarity) {
  bool result = false;

  if (polarity == Polarity::NORMAL) {
    result = (gpio_pin_get_dt(this->gpio) == 0) ? true : false;
  } else {
    result = (gpio_pin_get_dt(this->gpio) == 1) ? true : false;
  }

  return result;
}

void Button::onPressed(std::function<void()> callback) {
  assert(callback);

  this->callback = callback;

  if (gpio_pin_interrupt_configure_dt(this->gpio, GPIO_INT_EDGE_TO_ACTIVE) != 0) {
    LOG_ERR("Error: failed to configure interrupt on %s pin %d",
            this->gpio->port->name,
            this->gpio->pin);
    return;
  }

  gpio_init_callback(&this->callbackData, gpioCallback, BIT(this->gpio->pin));
  if (gpio_add_callback(this->gpio->port, &this->callbackData) != 0) {
    LOG_ERR("Error: failed to add gpio callback");
    return;
  }
}

static void gpioCallback(const struct device *dev, struct gpio_callback *callbackData, uint32_t pins) {
  // TODO: Find a way to get the Button instance and raise its callback
}
