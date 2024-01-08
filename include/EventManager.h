#ifndef EVENT_MANAGER_H
#define EVENT_MANAGER_H

// Zephyr includes
#include <zephyr/kernel.h>
#include <zephyr/zbus/zbus.h>

#define EVENT_ACTION_LIST_SIZE(list) (sizeof(list) / sizeof(list[0]))

// Possible events
typedef enum {
  EVENT_INITIAL_VALUE = 0,
  EVENT_NETWORK_AVAILABLE,
  EVENT_BUTTON_PRESSED,
  EVENT_START_OTA_UPDATE,
  EVENT_SENSOR_DATA_READY,
  EVENT_SENSOR_DATA_SENT,
  EVENT_MAX_VALUE
} event_id_t;

// event_id_t enum is embedded inside event_t struct because ZBUS only accepts struct or union
typedef struct {
  event_id_t id;
} event_t;

typedef void (*event_action_t) (void);

typedef struct {
  event_id_t id;
  event_action_t action;
} event_action_pair_t;

void processEvent(event_t *event, const event_action_pair_t *eventActionList, uint8_t listLength);
int waitForEvent(const zbus_observer *subscriber, event_t *event, k_timeout_t timeout);
int sendEvent(event_t *event, k_timeout_t timeout);

// Import channel and make it exportable by just including "EventManager.h"
ZBUS_CHAN_DECLARE(eventsChannel);

#endif // EVENT_MANAGER_H