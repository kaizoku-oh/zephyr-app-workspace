// Lib C
#include <assert.h>

// Zephyr includes
#include <zephyr/kernel.h>
#include <zephyr/zbus/zbus.h>
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(AppTelemetry);

// User C++ class headers
#include "EventManager.h"
#include "Storage.h"
#include "HttpClient.h"

// Function declaration of thread handlers
static void AppTelemetryThreadHandler();

// Function declaration of event actions
static void onButtonPressAction();
static void sendDataAction();
static void onNetworkAvailableAction();

// Read data from storage and format it into JSON array
static int prepareData(char *data, uint32_t *length);

// ZBUS subscribers definition
ZBUS_SUBSCRIBER_DEFINE(AppTelemetrySubscriber, 4);

// Add a subscriber observer to ZBUS events channel
ZBUS_CHAN_ADD_OBS(eventsChannel, AppTelemetrySubscriber, 4);

// Thread definition
K_THREAD_DEFINE(AppTelemetryThread, 4096, AppTelemetryThreadHandler, NULL, NULL, NULL, 7, 0, 0);

static const event_action_pair_t eventActionList[] {
  {EVENT_SENSOR_DATA_READY, sendDataAction          },
  {EVENT_BUTTON_PRESSED,    onButtonPressAction     },
  {EVENT_NETWORK_AVAILABLE, onNetworkAvailableAction},
};

static void AppTelemetryThreadHandler() {
  int ret = 0;
  event_t event = {.id = EVENT_INITIAL_VALUE};

  while (true) {
    ret = waitForEvent(&AppTelemetrySubscriber, &event, K_FOREVER);
    if (ret == 0) {
      processEvent(&event, eventActionList, EVENT_ACTION_LIST_SIZE(eventActionList));
    }
  }
}

static void sendDataAction() {
  char jsonArray[128] = {0};
  uint32_t length = sizeof(jsonArray);

  // Create an HTTP client as a local object
  HttpClient client((char *)"192.168.1.22", 1880);

  LOG_INF("Started sending sensor data to cloud");

  // Read 8 temperature strings from storage and append them to the JSON string
  // The final JSON string should be something like the following:
  // {"temperature":[20.4, 20.32, 21.9, 22.51, 21.33, 20.65, 21.78, 20.8]}
  prepareData(jsonArray, &length);

  // Send the readings to the HTTP server
  client.post("/data", jsonArray, length, [](HttpResponse *response) {
    event_t event = {.id = EVENT_SENSOR_DATA_SENT};

    LOG_INF("Response(%d bytes): %.*s", response->bodyLength, response->bodyLength, response->body);
    sendEvent(&event, K_NO_WAIT);
  });
}

static void onButtonPressAction() {
  LOG_INF("Button is pressed");
}

static void onNetworkAvailableAction() {
  LOG_INF("Network is now available");
}

static int prepareData(char *jsonArray, uint32_t *length) {
  int ret = 0;
  char temperatureString[6] = {0};
  int jsonStringOffset = 0;
  uint16_t readingID = 0;

  Storage& storage = Storage::getInstance();

  assert(jsonArray);
  assert(length);

  jsonStringOffset = 0;
  jsonStringOffset += snprintf(jsonArray + jsonStringOffset,
                               *length - jsonStringOffset,
                               "{\"temperature\":[");
  for (readingID = 0; readingID < 8; readingID++) {
    ret = storage.read(readingID, temperatureString, sizeof(temperatureString));
    if (ret < 0) {
      LOG_ERR("Failed to read temperature reading (id=%d) from storage", readingID);
      break;
    }
    LOG_INF("Read temperatureReading %d: %s °C", readingID, temperatureString);
    jsonStringOffset += snprintf(jsonArray + jsonStringOffset,
                                 *length - jsonStringOffset,
                                 "%s%s",
                                 temperatureString,
                                 (readingID < 7)?",":"");
    k_msleep(1000);
  }
  jsonStringOffset += snprintf(jsonArray + jsonStringOffset,
                               *length - jsonStringOffset,
                               "]}");
  LOG_INF("jsonArray: %s", jsonArray);

  *length = jsonStringOffset;

  return ret;
}

