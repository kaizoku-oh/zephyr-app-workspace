// Lib C
#include <assert.h>

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

int waitForEvent(const zbus_observer *subscriber, event_t *event, k_timeout_t timeout) {
  int ret = 0;
  const struct zbus_channel *channel = NULL;

  assert(subscriber);
  assert(event);

  // Wait forever for an event
  ret = zbus_sub_wait(subscriber, &channel, K_FOREVER);

  // Check if notification is received
  if (ret == 0) {

    // Make sure the event came on the right channel
    if (&eventsChannel == channel) {

      // Read the event
      ret = zbus_chan_read(&eventsChannel, event, K_NO_WAIT);

      if (ret == 0) {
        LOG_DBG("Subscriber <%s> received event <%d> on <%s>\r\n",
                subscriber->name,
                event->id,
                channel->name);
      } else {
        // Something wrong happened while reading event from channel
        LOG_ERR("Something wrong happened while reading from channel: %d", ret);
      }
    } else {
      // I'm not interested in this channel
      LOG_WRN("<%s> is not interested in this channel: <%s>", subscriber->name, channel->name);
    }
  } else {
    // Something wrong happened while waiting for event
    LOG_ERR("Something wrong happened while waiting for event: %d", ret);
  }

  return ret;
}

void processEvent(event_t *event, const event_action_pair_t *eventActionList, uint8_t listLength) {
  uint8_t eventIndex = 0;

  assert(event);
  assert(eventActionList);
  assert(listLength);

  // For each event/action pair
  for (eventIndex = 0; eventIndex < listLength; eventIndex++) {
    // If the event that needs to be processed is found
    if (event->id == eventActionList[eventIndex].id) {
      // And has a valid action function
      if (eventActionList[eventIndex].action) {
        // Execute its associated action
        eventActionList[eventIndex].action();
      }
    }
  }
}

int publishEvent(event_t *event, k_timeout_t timeout) {
  int ret = 0;

  assert(event);

  ret = zbus_chan_pub(&eventsChannel, event, timeout);

  return ret;
}
