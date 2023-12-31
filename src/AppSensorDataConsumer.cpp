// Zephyr includes
#include <zephyr/kernel.h>
#include <zephyr/zbus/zbus.h>
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(AppSensorDataConsumer);

// User C++ class headers
#include "EventManager.h"

// Function declaration of thread handlers
static void sensorDataConsumerThreadHandler();

// ZBUS subscribers definition
ZBUS_SUBSCRIBER_DEFINE(sensorDataConsumerSubscriber, 4);

// Add a subscriber observer to ZBUS events channel
ZBUS_CHAN_ADD_OBS(eventsChannel, sensorDataConsumerSubscriber, 4);

// Thread definition
K_THREAD_DEFINE(sensorDataConsumerThread, 1024, sensorDataConsumerThreadHandler, NULL, NULL, NULL, 7, 0, 0);

static void sensorDataConsumerThreadHandler() {
  int ret = 0;

  // Initialize local variable to hold the event
  event_t event = {.id = EVENT_INITIAL_VALUE};

  // Used to figure out on which channel the event came from
  const struct zbus_channel *channel = NULL;

  while (true) {

    // Wait forever for an event
    ret = zbus_sub_wait(&sensorDataConsumerSubscriber, &channel, K_FOREVER);

    // Check if notification is received
    if (ret == 0) {

      // Make sure the event came on the right channel
      if (&eventsChannel == channel) {

        // Read the event
        ret = zbus_chan_read(&eventsChannel, &event, K_NO_WAIT);
        if (ret == 0) {
          LOG_DBG("Subscriber <%s> received event <%s> on <%s>\r\n",
                  sensorDataConsumerSubscriber.name,
                  EVENT_ID_TO_STRING(event.id),
                  channel->name);

          // Make sure the event is the one we are interested in
          switch (event.id) {

            case EVENT_SENSOR_DATA_SAVED: {
              LOG_INF("Started sending sensor data to cloud");

              // Fake sending (to be replaced with real sending function)
              k_msleep(5000);
              LOG_INF("\r\n===========================================================\r\n");

              // Publish the <EVENT_SENSOR_DATA_SENT> event on <eventsChannel>
              event.id = EVENT_SENSOR_DATA_SENT;
              zbus_chan_pub(&eventsChannel, &event, K_NO_WAIT);

              break;
            }

            default: {
              // I'm not interested in this event
              LOG_DBG("<%s> is not interested in this event: <%s>",
                      sensorDataConsumerSubscriber.name,
                      EVENT_ID_TO_STRING(event.id));
              break;
            }
          }
        } else {
          // Something wrong happened while reading event from channel
          LOG_ERR("Something wrong happened while reading from channel: %d", ret);
        }
      } else {
        // I'm not interested in this channel
        LOG_WRN("<%s> is not interested in this channel: <%s>",
                sensorDataConsumerSubscriber.name,
                channel->name);
      }
    } else {
      // Something wrong happened while waiting for event
      LOG_ERR("Something wrong happened while waiting for event: %d", ret);
    }
  }
}
