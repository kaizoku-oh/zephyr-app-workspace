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
#include "HttpClient.h"

// Delay value used inside the thread loop to yield back to the scheduler
static constexpr uint32_t MAIN_THREAD_SLEEP_TIME_MS = 1000;

static volatile bool networkAvailable = false;

int main(void) {

  // Get the singleton instance of Network
  Network& network = Network::getInstance();

  // Set up the lambda callback for IP address notification
  network.onGotIP([](const char *ipAddress) {

    networkAvailable = true;

    // Initialize local variable to hold the event
    event_t event = {.id = EVENT_START_SENSOR_DATA_ACQUISITION};

    LOG_INF("Got IP address: %s\r\n", ipAddress);

    // Publish the event
    zbus_chan_pub(&eventsChannel, &event, K_NO_WAIT);
  });

  // Start the network and wait for an IP address
  network.start();

  // Wait for network to be available
  while (!networkAvailable);

  // Create an HTTP client
  HttpClient client((char *)"10.42.0.1", 1880);

  // Try to download a text file from server
  client.get("/file.txt", [](uint8_t *response, uint32_t length) {
    LOG_INF("\r\nResponse(%d bytes): %.*s\r\n", length, length, response);
  });

  while (true) {
    k_msleep(MAIN_THREAD_SLEEP_TIME_MS);
  }

  return EXIT_SUCCESS;
}
