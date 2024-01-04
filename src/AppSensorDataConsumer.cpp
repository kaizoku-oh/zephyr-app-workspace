// Zephyr includes
#include <zephyr/kernel.h>
#include <zephyr/zbus/zbus.h>
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(AppSensorDataConsumer);

// User C++ class headers
#include "EventManager.h"
#include "Storage.h"
#include "HttpClient.h"

// Function declaration of thread handlers
static void sensorDataConsumerThreadHandler();

// ZBUS subscribers definition
ZBUS_SUBSCRIBER_DEFINE(sensorDataConsumerSubscriber, 4);

// Add a subscriber observer to ZBUS events channel
ZBUS_CHAN_ADD_OBS(eventsChannel, sensorDataConsumerSubscriber, 4);

// Thread definition
K_THREAD_DEFINE(sensorDataConsumerThread, 4096, sensorDataConsumerThreadHandler, NULL, NULL, NULL, 7, 0, 0);

static void sensorDataConsumerThreadHandler() {
  int ret = 0;

  // Initialize local variable to hold the event
  event_t event = {.id = EVENT_INITIAL_VALUE};

  // Used to figure out on which channel the event came from
  const struct zbus_channel *channel = NULL;

  // Variables to hold temperature reading in Celsius
  char temperatureString[6] = {0};
  char jsonArray[128] = {0};
  int jsonStringOffset = 0;
  uint16_t readingID = 0;

  // Get the Storage instance
  Storage& storage = Storage::getInstance();

  // Create an HTTP client as a local object
  HttpClient client((char *)"192.168.43.145", 1880);

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

              // Read 8 temperature strings from storage and append them to the JSON string
              // The final JSON string should be something like the following:
              // {"temperature":[20.4, 20.32, 21.9, 22.51, 21.33, 20.65, 21.78, 20.8]}
              jsonStringOffset = 0;
              jsonStringOffset += snprintf(jsonArray + jsonStringOffset,
                                           sizeof(jsonArray) - jsonStringOffset,
                                           "{\"temperature\":[");
              for (readingID = 0; readingID < 8; readingID++) {
                ret = storage.read(readingID, temperatureString, sizeof(temperatureString));
                if (ret < 0) {
                  LOG_ERR("Failed to read temperature reading (id=%d) from storage\r\n", readingID);
                  break;
                }
                LOG_INF("Read temperatureReading %d: %s °C", readingID, temperatureString);
                jsonStringOffset += snprintf(jsonArray + jsonStringOffset,
                                             sizeof(jsonArray) - jsonStringOffset,
                                             "%s%s",
                                             temperatureString,
                                             (readingID < 7)?",":"");
                k_msleep(1000);
              }
              jsonStringOffset += snprintf(jsonArray + jsonStringOffset,
                                           sizeof(jsonArray) - jsonStringOffset,
                                           "]}");
              LOG_INF("jsonArray: %s", jsonArray);

              // Send the readings to the HTTP server
              client.post("/data", jsonArray, jsonStringOffset, [](uint8_t *response, uint32_t length) {
                size_t index = 0;
                event_t event = {.id = EVENT_SENSOR_DATA_SENT};

                // Skip headers by looking for the start of the response body: '{'
                for (index = 0; index < length; index++) {
                  if (response[index] == '{') {
                    break;
                  }
                }
                printk("\r\nResponse(%d bytes): %.*s\r\n", length-index, length-index, &response[index]);

                // Publish the <EVENT_SENSOR_DATA_SENT> event on <eventsChannel>
                zbus_chan_pub(&eventsChannel, &event, K_NO_WAIT);
              });

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
