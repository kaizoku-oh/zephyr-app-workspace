// Zephyr includes
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/zbus/zbus.h>
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(AppAcquisition);

// User C++ class headers
#include "EventManager.h"
#include "Temperature.h"
#include "Storage.h"

// Function declaration of thread handlers
static void acquisitionThreadHandler();

// Function declaration of event actions
static void onButtonPressAction();
static void onNetworkAvailableAction();
static void takeMeasurementsAction();

// ZBUS subscribers definition
ZBUS_SUBSCRIBER_DEFINE(acquisitionSubscriber, 4);

// Add a subscriber observer to ZBUS events channel
ZBUS_CHAN_ADD_OBS(eventsChannel, acquisitionSubscriber, 4);

// Thread definition
K_THREAD_DEFINE(acquisitionThread, 1024, acquisitionThreadHandler, NULL, NULL, NULL, 7, 0, 0);

static const event_action_pair_t eventActionList[] {
  {EVENT_BUTTON_PRESSED,    onButtonPressAction     },
  {EVENT_NETWORK_AVAILABLE, onNetworkAvailableAction},
  {EVENT_SENSOR_DATA_SENT,  takeMeasurementsAction  },
};

static volatile bool networkIsAvailable = false;

static void acquisitionThreadHandler() {
  int ret = 0;
  event_t event = {.id = EVENT_INITIAL_VALUE};

  while (true) {
    ret = waitForEvent(&acquisitionSubscriber, &event, K_FOREVER);
    if (ret == 0) {
      processEvent(&event, eventActionList, EVENT_ACTION_LIST_SIZE(eventActionList));
    }
  }
}

static void takeMeasurementsAction() {
  int ret = 0;

  // Initialize local variable to hold the event
  event_t event = {.id = EVENT_INITIAL_VALUE};

  // Variable to hold temperature reading in Celsius
  double temperatureDouble = 0;
  char temperatureString[6] = {0};

  // Reference die temperature device from device tree
  const struct device *temperatureDevice = DEVICE_DT_GET(DT_NODELABEL(die_temp));

  // Create local object using the device
  Temperature temperature(temperatureDevice);

  // Get the Storage instance
  Storage& storage = Storage::getInstance();

  LOG_INF("Started acquiring sensor data and saving it to storage");

  // Take 8 readings and save them in storage
  for (uint16_t readingID = 0; readingID < 8; readingID++) {

    // Read temperature as a double
    temperatureDouble = temperature.read();

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

  // Publish the <EVENT_SENSOR_DATA_READY> event on <eventsChannel>
  event.id = EVENT_SENSOR_DATA_READY;
  publishEvent(&event, K_NO_WAIT);
}

static void onButtonPressAction() {
  LOG_INF("Button is pressed");
}

static void onNetworkAvailableAction() {
  LOG_INF("Network is now available");
  networkIsAvailable = true;
}

