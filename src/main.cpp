#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <inttypes.h>

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/sys/printk.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/zbus/zbus.h>

#include "Button.h"
#include "Led.h"
#include "Temperature.h"
#include "Serial.h"
#include "Network.h"
#include "HttpClient.h"
#include "Storage.h"

#define EVENT_ID_TO_STRING(id) (event_names[(id)])

typedef enum {
  EVENT_INITIAL_VALUE = 0,
  EVENT_NETWORK_AVAILABLE,
  EVENT_BUTTON_PRESSED,
  EVENT_MAX
} event_id_t;

typedef struct {
  event_id_t id;
} event_t;

static const char *event_names [] = {
  [EVENT_INITIAL_VALUE]     = "EVENT_INITIAL_VALUE",
  [EVENT_NETWORK_AVAILABLE] = "EVENT_NETWORK_AVAILABLE",
  [EVENT_BUTTON_PRESSED]    = "EVENT_BUTTON_PRESSED",
  [EVENT_MAX]               = "EVENT_MAX",
};

static void ledThreadHandler();
static void temperatureThreadHandler();
static void buttonThreadHandler();
static void networkThreadHandler();
static void httpGetRequestThreadHandler();
static void httpPostRequestThreadHandler();
static void storageThreadHandler();

static void triggerTimerHandler(struct k_timer *timer);

static void eventsListenerCallback(const struct zbus_channel *channel);

static constexpr uint32_t MAIN_THREAD_SLEEP_TIME_MS = 1000;
static constexpr uint32_t LED_THREAD_SLEEP_TIME_MS = 50;
static constexpr uint32_t TEMPERATURE_THREAD_SLEEP_TIME_MS = 5000;
static constexpr uint32_t BUTTON_THREAD_SLEEP_TIME_MS = 100;
static constexpr uint32_t NETWORK_THREAD_SLEEP_TIME_MS = 1000;
static constexpr uint32_t STORAGE_THREAD_SLEEP_TIME_MS = 1000;

K_TIMER_DEFINE(triggerTimer, triggerTimerHandler, NULL);

ZBUS_LISTENER_DEFINE(eventsListener, eventsListenerCallback);

ZBUS_SUBSCRIBER_DEFINE(httpGetRequestSubscriber, 4);
ZBUS_SUBSCRIBER_DEFINE(httpPostRequestSubscriber, 4);
ZBUS_SUBSCRIBER_DEFINE(ledSubscriber, 4);

ZBUS_CHAN_DEFINE(
  eventsChannel,                           // Channel name
  event_t,                                 // Message type
  NULL,                                    // Validator function
  NULL,                                    // User data
  ZBUS_OBSERVERS(
    eventsListener,
    httpGetRequestSubscriber,
    httpPostRequestSubscriber
  ),                                       // Initial observers list
  ZBUS_MSG_INIT(.id = EVENT_INITIAL_VALUE) // Message initialization
);

ZBUS_CHAN_ADD_OBS(eventsChannel, ledSubscriber, 4);

K_THREAD_DEFINE(ledThread, 1024, ledThreadHandler, NULL, NULL, NULL, 7, 0, 0);
K_THREAD_DEFINE(temperatureThread, 1024, temperatureThreadHandler, NULL, NULL, NULL, 7, 0, 0);
K_THREAD_DEFINE(buttonThread, 1024, buttonThreadHandler, NULL, NULL, NULL, 7, 0, 0);
K_THREAD_DEFINE(networkThread, 1024, networkThreadHandler, NULL, NULL, NULL, 7, 0, 0);
K_THREAD_DEFINE(httpGetRequestThread, 3*1024, httpGetRequestThreadHandler, NULL, NULL, NULL, 7, 0, 0);
K_THREAD_DEFINE(httpPostRequestThread, 3*1024, httpPostRequestThreadHandler, NULL, NULL, NULL, 7, 0, 0);
K_THREAD_DEFINE(storageThread, 2*1024, storageThreadHandler, NULL, NULL, NULL, 7, 0, 0);

int main(void) {
  k_timer_start(&triggerTimer, K_SECONDS(10), K_SECONDS(10));

  while (true) {
    k_msleep(MAIN_THREAD_SLEEP_TIME_MS);
  }

  return EXIT_SUCCESS;
}

static void ledThreadHandler() {
  // Initilize local variable to hold the event
  event_t event = {.id = EVENT_INITIAL_VALUE};

  const struct zbus_channel *channel = NULL;

  // Reference GPIO devices from device tree
  const struct gpio_dt_spec greenLedGpio = GPIO_DT_SPEC_GET_OR(DT_ALIAS(led0), gpios, {0});
  const struct gpio_dt_spec blueLedGpio = GPIO_DT_SPEC_GET_OR(DT_ALIAS(led1), gpios, {0});
  const struct gpio_dt_spec redLedGpio = GPIO_DT_SPEC_GET_OR(DT_ALIAS(led2), gpios, {0});

  // Create local object using the device tree device
  Led greenLed(&greenLedGpio);
  Led blueLed(&blueLedGpio);
  Led redLed(&redLedGpio);

  // Wait forever for an event
  while (!zbus_sub_wait(&ledSubscriber, &channel, K_FOREVER)) {

    // Make sure the event came on the right channel
    if (&eventsChannel == channel) {

      // Read the event
      zbus_chan_read(&eventsChannel, &event, K_NO_WAIT);
      printk("Subscriber <%s> received event <%s> on <%s>\r\n",
             ledSubscriber.name,
             EVENT_ID_TO_STRING(event.id),
             channel->name);

      // Make sure the event is the one we are intersted in
      if (event.id == EVENT_BUTTON_PRESSED) {

        // Toggle LEDs
        greenLed.toggle();
        k_msleep(LED_THREAD_SLEEP_TIME_MS);

        blueLed.toggle();
        k_msleep(LED_THREAD_SLEEP_TIME_MS);

        redLed.toggle();
        k_msleep(LED_THREAD_SLEEP_TIME_MS);

      } else {
        // I'm not interested in this event
      }
    } else {
      // I'm not interested in this channel
    }
  }
}

static void temperatureThreadHandler() {
  // Variable to hold temperature reading in Celsius
  double temperatureReading = 0;

  // Reference die temperature device from device tree
  const struct device *temperatureDevice = DEVICE_DT_GET(DT_NODELABEL(die_temp));

  // Create local object using the device
  Temperature temperature(temperatureDevice);

  // Continuously read temperature
  while (true) {
    // Publish temperature value to the zbus
    temperatureReading = temperature.read();
    k_msleep(TEMPERATURE_THREAD_SLEEP_TIME_MS);
  }
}

static void buttonThreadHandler() {
  // Initilize local variable to hold the event
  event_t event = {.id = EVENT_BUTTON_PRESSED};

  // Reference GPIO device from device tree
  const struct gpio_dt_spec buttonGpio = GPIO_DT_SPEC_GET_OR(DT_ALIAS(sw0), gpios, {0});

  // Create local object using the device tree device
  Button button(&buttonGpio);

  // Continuously check if a button is pressed
  while (true) {
    if (button.isPressed()) {
      printk("Button is pressed\r\n");
      // Publish the <EVENT_BUTTON_PRESSED> event on <eventsChannel>
      zbus_chan_pub(&eventsChannel, &event, K_NO_WAIT);
    }
    k_msleep(BUTTON_THREAD_SLEEP_TIME_MS);
  }
}

static void networkThreadHandler() {
  // Get the singleton instance of Network
  Network& network = Network::getInstance();

  // Set up the lambda callback for IP address notification
  network.onGotIP([](const char *ipAddress) {
    event_t event = {.id = EVENT_NETWORK_AVAILABLE};

    printf("Got IP address: %s\r\n", ipAddress);
    // Publish the <EVENT_NETWORK_AVAILABLE> event on <eventsChannel>
    zbus_chan_pub(&eventsChannel, &event, K_NO_WAIT);
  });

  // Start the network and wait for an IP address
  network.start();

  while (true) {
    k_msleep(NETWORK_THREAD_SLEEP_TIME_MS);
  }
}

static void httpGetRequestThreadHandler() {
  // Initilize local variable to hold the event
  event_t event = {.id = EVENT_INITIAL_VALUE};

  const struct zbus_channel *channel = NULL;

  // Create an HTTP client as a local object
  HttpClient client((char *)"10.42.0.1", 1880);

  // Wait forever for an event
  while (!zbus_sub_wait(&httpGetRequestSubscriber, &channel, K_FOREVER)) {

    // Make sure the event came on the right channel
    if (&eventsChannel == channel) {

      // Read the event
      zbus_chan_read(&eventsChannel, &event, K_NO_WAIT);
      printk("Subscriber <%s> received event <%s> on <%s>\r\n",
             httpGetRequestSubscriber.name,
             EVENT_ID_TO_STRING(event.id),
             channel->name);

      // Make sure the event is the one we are intersted in
      if (event.id == EVENT_NETWORK_AVAILABLE) {

        // Send GET request and handle response in a lambda callback
        client.get("/data", [](uint8_t *response, uint32_t length) {
          size_t index = 0;

          // Skip headers by looking for the start of the json message
          for (index = 0; index < length; index++) {
            if (response[index] == '{') {
              break;
            }
          }
          printk("\r\nResponse(%d bytes): %.*s\r\n", length-index, length-index, &response[index]);
        });
      } else {
        // I'm not interested in this event
      }
    } else {
      // I'm not interested in this channel
    }
  }
}

static void httpPostRequestThreadHandler() {
  // Initilize local variable to hold the event
  event_t event = {.id = EVENT_INITIAL_VALUE};

  const struct zbus_channel *channel = NULL;

  // Create an HTTP client as a local object
  HttpClient client((char *)"10.42.0.1", 1880);

  // Wait forever for an event
  while (!zbus_sub_wait(&httpPostRequestSubscriber, &channel, K_FOREVER)) {

    // Make sure the event came on the right channel
    if (&eventsChannel == channel) {

      // Read the event
      zbus_chan_read(&eventsChannel, &event, K_NO_WAIT);
      printk("Subscriber <%s> received event <%s> on <%s>\r\n",
             httpPostRequestSubscriber.name,
             EVENT_ID_TO_STRING(event.id),
             channel->name);

      // Make sure the event is the one we are intersted in
      if (event.id == EVENT_NETWORK_AVAILABLE) {

        // Send POST request and handle response in a lambda callback
        client.post("/data", "{\"status\": \"connected\"}",
                    sizeof("{\"status\": \"connected\"}")-1,
                    [](uint8_t *response, uint32_t length) {
          size_t index = 0;

          // Skip headers by looking for the start of the json response
          for (index = 0; index < length; index++) {
            if (response[index] == '{') {
              break;
            }
          }
          printk("\r\nResponse(%d bytes): %.*s\r\n", length-index, length-index, &response[index]);
        });
      } else {
        // I'm not interested in this event
      }
    } else {
      // I'm not interested in this channel
    }
  }
}

static void storageThreadHandler() {
  int ret = 0;
  uint8_t buffer[sizeof("xxx.xxx.xxx.xxx")] = {0};

  const uint16_t IP_ADDRESS_ID = 1;

  // Get the Storage instance
  Storage& storage = Storage::getInstance();

  // Write data to NVS
  ret = storage.write(IP_ADDRESS_ID, (uint8_t *)"192.168.1.2", sizeof("192.168.1.2")-1);
  if (ret > 0) {
    printk("Data written successfully to NVS!\r\n");
  } else {
    printk("Failed to write data to NVS\r\n");
  }

  // Read data from NVS
  ret = storage.read(IP_ADDRESS_ID, buffer, sizeof(buffer));
  if (ret > 0) {
    printk("Data read successfully from NVS: %.*s\r\n", ret, buffer);
  } else {
    printk("Failed to read data from NVS\r\n");
  }

  while (true) {
    k_msleep(STORAGE_THREAD_SLEEP_TIME_MS);
  }
}

static void eventsListenerCallback(const struct zbus_channel *channel) {
  // Read event from events channel
  const event_t *event = (event_t *)zbus_chan_const_msg(channel);

  printk("Listener <%s> received event: <%d> on <%s>\r\n", eventsListener.name, event->id, channel->name);
}

static void triggerTimerHandler(struct k_timer *timer) {
  printk("Timer is triggered\r\n");
}
