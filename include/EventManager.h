#ifndef EVENT_MANAGER_H
#define EVENT_MANAGER_H

// Zephyr includes
#include <zephyr/zbus/zbus.h>

// Macro to convert event ID to string
#define EVENT_ID_TO_STRING(id) (EVENT_NAMES[(id)])

// Possible events
typedef enum {
  EVENT_INITIAL_VALUE = 0,
  EVENT_NETWORK_AVAILABLE,
  EVENT_BUTTON_PRESSED,
  EVENT_START_SENSOR_DATA_ACQUISITION,
  EVENT_SENSOR_DATA_SAVED,
  EVENT_SENSOR_DATA_SENT,
  EVENT_MAX_VALUE
} event_id_t;

// event_id_t enum is embedded inside event_t struct because ZBUS only accepts struct or union
typedef struct {
  event_id_t id;
} event_t;

// Event id to string mapping
static const char *EVENT_NAMES[] = {
  [EVENT_INITIAL_VALUE]                 = "EVENT_INITIAL_VALUE",
  [EVENT_NETWORK_AVAILABLE]             = "EVENT_NETWORK_AVAILABLE",
  [EVENT_BUTTON_PRESSED]                = "EVENT_BUTTON_PRESSED",
  [EVENT_START_SENSOR_DATA_ACQUISITION] = "EVENT_START_SENSOR_DATA_ACQUISITION",
  [EVENT_SENSOR_DATA_SAVED]             = "EVENT_SENSOR_DATA_SAVED",
  [EVENT_SENSOR_DATA_SENT]              = "EVENT_SENSOR_DATA_SENT",
  [EVENT_MAX_VALUE]                     = "EVENT_MAX_VALUE"
};

// ZBUS channel definition
ZBUS_CHAN_DEFINE(
  eventsChannel,                           // Channel name
  event_t,                                 // Message type
  NULL,                                    // Validator function
  NULL,                                    // User data
  ZBUS_OBSERVERS(ZBUS_OBSERVERS_EMPTY),    // Initial observers list
  ZBUS_MSG_INIT(.id = EVENT_INITIAL_VALUE) // Message initialization
);

#endif // EVENT_MANAGER_H