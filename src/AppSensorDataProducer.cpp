// Zephyr includes
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/zbus/zbus.h>
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(AppSensorDataProducer);

// User C++ class headers
#include "EventManager.h"
#include "Temperature.h"
#include "Storage.h"

// Function declaration of thread handlers
static void sensorDataProducerThreadHandler();

// ZBUS subscribers definition
ZBUS_SUBSCRIBER_DEFINE(sensorDataProducerSubscriber, 4);

// Add a subscriber observer to ZBUS events channel
ZBUS_CHAN_ADD_OBS(eventsChannel, sensorDataProducerSubscriber, 4);

// Thread definition
K_THREAD_DEFINE(sensorDataProducerThread, 1024, sensorDataProducerThreadHandler, NULL, NULL, NULL, 7, 0, 0);

static void sensorDataProducerThreadHandler() {
  int ret = 0;

  // Initialize local variable to hold the event
  event_t event = {.id = EVENT_INITIAL_VALUE};

  // Used to figure out on which channel the event came from
  const struct zbus_channel *channel = NULL;

  // Variable to hold temperature reading in Celsius
  double temperatureDouble = 0;
  char temperatureString[6] = {0};

  // Reference die temperature device from device tree
  const struct device *temperatureDevice = DEVICE_DT_GET(DT_NODELABEL(die_temp));

  // Create local object using the device
  Temperature temperature(temperatureDevice);

  // Get the Storage instance
  Storage& storage = Storage::getInstance();

  while (true) {

    // Wait forever for an event
    ret = zbus_sub_wait(&sensorDataProducerSubscriber, &channel, K_FOREVER);

    // Check if notification is received
    if (ret == 0) {

      // Make sure the event came on the right channel
      if (&eventsChannel == channel) {

        // Read the event
        ret = zbus_chan_read(&eventsChannel, &event, K_NO_WAIT);

        if (ret == 0) {

          LOG_DBG("Subscriber <%s> received event <%s> on <%s>\r\n",
                  sensorDataProducerSubscriber.name,
                  EVENT_ID_TO_STRING(event.id),
                  channel->name);

          // Make sure the event is the one we are interested in
          switch (event.id) {

            case EVENT_NETWORK_AVAILABLE:
            case EVENT_START_SENSOR_DATA_ACQUISITION:
            case EVENT_SENSOR_DATA_SENT: {
              LOG_INF("Started acquiring sensor data and saving it to storage");

              // Take 8 readings and save them in storage
              for (uint16_t readingID = 0; readingID < 8; readingID++) {

                // Read temperature as a double
                temperatureDouble = temperature.read();
                LOG_INF("Saved temperatureDouble %d: %.1f °C", readingID, temperatureDouble);

                // Convert temperature to a string
                ret = snprintf(temperatureString, sizeof(temperatureString), "%.2f", temperatureDouble);
                LOG_INF("Saved temperatureString %d: %.*s °C", readingID, sizeof(temperatureString), temperatureString);

                // Save reading in storage
                ret = storage.write(readingID, &temperatureString, sizeof(temperatureString));
                if (ret < 0) {
                  LOG_ERR("Failed to save temperature reading (id=%d) in storage\r\n", readingID);
                  break;
                }
                k_msleep(1000);
              }

              // Publish the <EVENT_SENSOR_DATA_SAVED> event on <eventsChannel>
              event.id = EVENT_SENSOR_DATA_SAVED;
              zbus_chan_pub(&eventsChannel, &event, K_NO_WAIT);

              break;
            }

            default: {
              // I'm not interested in this event
              LOG_DBG("<%s> is not interested in this event: <%s>",
                      sensorDataProducerSubscriber.name,
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
                sensorDataProducerSubscriber.name,
                channel->name);
      }
    } else {
      // Something wrong happened while waiting for event
      LOG_ERR("Something wrong happened while waiting for event: %d", ret);
    }
  }
}
