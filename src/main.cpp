// Lib C includes
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

// Zephyr includes
#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/zbus/zbus.h>
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(main);

// User C++ class headers
#include "EventManager.h"
#include "Network.h"
#include "HttpClient.h"
#include "Button.h"

int main(void) {
  event_t event = {.id = EVENT_BUTTON_PRESSED};

  const struct gpio_dt_spec buttonGpio = GPIO_DT_SPEC_GET_OR(DT_ALIAS(sw0), gpios, {0});
  Button button(&buttonGpio);

  Network& network = Network::getInstance();

  network.onGotIP([](const char *ipAddress) {
    event_t event = {.id = EVENT_NETWORK_AVAILABLE};
    zbus_chan_pub(&eventsChannel, &event, K_NO_WAIT);
  });

  network.start();

  while (true) {
    if (button.isPressed()) {
      zbus_chan_pub(&eventsChannel, &event, K_NO_WAIT);
    }
    k_msleep(100);
  }

  return EXIT_SUCCESS;
}
