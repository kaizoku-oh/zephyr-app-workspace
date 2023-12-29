// Zephyr includes
#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/zbus/zbus.h>

// User C++ class headers
#include "EventManager.h"

// Function declaration of thread handlers
static void SensorDataProducerThreadHandler();

// ZBUS subscribers definition
ZBUS_SUBSCRIBER_DEFINE(SensorDataProducerSubscriber, 4);

// Add a subscriber observer to ZBUS events channel
ZBUS_CHAN_ADD_OBS(eventsChannel, SensorDataProducerSubscriber, 4);

// Thread definition
K_THREAD_DEFINE(SensorDataProducerThread, 1024, SensorDataProducerThreadHandler, NULL, NULL, NULL, 7, 0, 0);

static void SensorDataProducerThreadHandler() {
  // Initialize local variable to hold the event
  event_t event = {.id = EVENT_INITIAL_VALUE};

  // Used to figure out on which channel the event came from
  const struct zbus_channel *channel = NULL;

  // Wait forever for an event
  while (!zbus_sub_wait(&SensorDataProducerSubscriber, &channel, K_FOREVER)) {

    // Make sure the event came on the right channel
    if (&eventsChannel == channel) {

      // Read the event
      zbus_chan_read(&eventsChannel, &event, K_NO_WAIT);
      printk("Subscriber <%s> received event <%s> on <%s>\r\n",
             SensorDataProducerSubscriber.name,
             EVENT_ID_TO_STRING(event.id),
             channel->name);

      // Make sure the event is the one we are interested in
      switch (event.id) {
        case EVENT_START_SENSOR_DATA_ACQUISITION:
        case EVENT_SENSOR_DATA_SENT: {
          printk("Started acquiring sensor data and saving it to storage");

          // Fake acquisition (to be replaced with real sensor acquisition function)
          for (size_t i = 0; i < 10; i++) {
            printk(".");
            k_msleep(500);
          }
          printk("\r\n");

          // Publish the <EVENT_SENSOR_DATA_SAVED> event on <eventsChannel>
          event.id = EVENT_SENSOR_DATA_SAVED;
          zbus_chan_pub(&eventsChannel, &event, K_NO_WAIT);

          break;
        }

        default: {
          // I'm not interested in this event
          break;
        }
      }
    } else {
      // I'm not interested in this channel
    }
  }
}
