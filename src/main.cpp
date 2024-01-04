// Lib C includes
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

// Zephyr includes
#include <zephyr/kernel.h>
#include <zephyr/zbus/zbus.h>
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(main);

// User C++ class headers
#include "EventManager.h"
#include "Network.h"

// Delay value used inside the thread loop to yield back to the scheduler
static constexpr uint32_t MAIN_THREAD_SLEEP_TIME_MS = 1000;

int main(void) {

  // Get the singleton instance of Network
  Network& network = Network::getInstance();

  // Set up the lambda callback for IP address notification
  network.onGotIP([](const char *ipAddress) {

    // Initialize local variable to hold the event
    event_t event = {.id = EVENT_START_SENSOR_DATA_ACQUISITION};

    LOG_INF("Got IP address: %s\r\n", ipAddress);

    // Publish the event
    zbus_chan_pub(&eventsChannel, &event, K_NO_WAIT);
  });

  // Start the network and wait for an IP address
  network.start();

  while (true) {
    k_msleep(MAIN_THREAD_SLEEP_TIME_MS);
  }

  return EXIT_SUCCESS;
}
