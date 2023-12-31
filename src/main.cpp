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

// Delay values used inside thread loops to yield back to scheduler
static constexpr uint32_t MAIN_THREAD_SLEEP_TIME_MS = 1000;

int main(void) {
  // Initialize local variable to hold the event
  event_t event = {.id = EVENT_INITIAL_VALUE};

  // Publish the <EVENT_START_SENSOR_DATA_ACQUISITION> event on <eventsChannel>
  event.id = EVENT_START_SENSOR_DATA_ACQUISITION;
  zbus_chan_pub(&eventsChannel, &event, K_NO_WAIT);

  LOG_ERR("Error message example.");
  LOG_WRN("Warning message example.");
  LOG_INF("Info message example.");
  LOG_DBG("Debug message example.");

  while (true) {
    k_msleep(MAIN_THREAD_SLEEP_TIME_MS);
  }

  return EXIT_SUCCESS;
}
