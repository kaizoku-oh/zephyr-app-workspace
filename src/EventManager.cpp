// Zephyr includes
#include <zephyr/zbus/zbus.h>
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(EventManager);

// User C++ class headers
#include "EventManager.h"

// ZBUS channel definition
ZBUS_CHAN_DEFINE(
  eventsChannel,                           // Channel name
  event_t,                                 // Message type
  NULL,                                    // Validator function
  NULL,                                    // User data
  ZBUS_OBSERVERS(ZBUS_OBSERVERS_EMPTY),    // Initial observers list
  ZBUS_MSG_INIT(.id = EVENT_INITIAL_VALUE) // Message initialization
);