#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <inttypes.h>

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/sys/printk.h>
#include <zephyr/drivers/gpio.h>

#include "Button.h"
#include "Led.h"
#include "Temperature.h"
#include "Serial.h"
#include "Network.h"
#include "HttpClient.h"
#include "Storage.h"

typedef enum {
  EVENT_NETWORK_AVAILABLE = 0,
  EVENT_MAX
} event_t;

static constexpr uint32_t MAIN_THREAD_SLEEP_TIME_MS = 1000;
static constexpr uint32_t LED_THREAD_SLEEP_TIME_MS = 50;
static constexpr uint32_t TEMPERATURE_THREAD_SLEEP_TIME_MS = 1000;
static constexpr uint32_t BUTTON_THREAD_SLEEP_TIME_MS = 100;
static constexpr uint32_t NETWORK_THREAD_SLEEP_TIME_MS = 1000;
static constexpr uint32_t HTTP_CLIENT_THREAD_SLEEP_TIME_MS = 1000;
static constexpr uint32_t STORAGE_THREAD_SLEEP_TIME_MS = 1000;

static void ledThreadHandler(void);
static void temperatureThreadHandler(void);
static void buttonThreadHandler(void);
static void networkThreadHandler(void);
static void httpClientThreadHandler(void);
static void storageThreadHandler(void);

K_THREAD_DEFINE(ledThread, 512, ledThreadHandler, NULL, NULL, NULL, 7, 0, 0);
K_THREAD_DEFINE(temperatureThread, 1024, temperatureThreadHandler, NULL, NULL, NULL, 7, 0, 0);
K_THREAD_DEFINE(buttonThread, 1024, buttonThreadHandler, NULL, NULL, NULL, 7, 0, 0);
K_THREAD_DEFINE(networkThread, 512, networkThreadHandler, NULL, NULL, NULL, 7, 0, 0);
K_THREAD_DEFINE(httpClientThread, 1024, httpClientThreadHandler, NULL, NULL, NULL, 7, 0, 0);
K_THREAD_DEFINE(storageThread, 2*1024, storageThreadHandler, NULL, NULL, NULL, 7, 0, 0);

K_MSGQ_DEFINE(queue, sizeof(event_t), 8, 1);

int main(void) {
  while (true) {
    k_msleep(MAIN_THREAD_SLEEP_TIME_MS);
  }

  return EXIT_SUCCESS;
}

static void ledThreadHandler(void) {
  // Reference devices from device tree
  const struct gpio_dt_spec greenLedGpio = GPIO_DT_SPEC_GET_OR(DT_ALIAS(led0), gpios, {0});
  const struct gpio_dt_spec blueLedGpio = GPIO_DT_SPEC_GET_OR(DT_ALIAS(led1), gpios, {0});
  const struct gpio_dt_spec redLedGpio = GPIO_DT_SPEC_GET_OR(DT_ALIAS(led2), gpios, {0});

  // Create local object using the device tree device
  Led greenLed(&greenLedGpio);
  Led blueLed(&blueLedGpio);
  Led redLed(&redLedGpio);

  // Continuously toggle LEDs
  while (true) {
    greenLed.toggle();
    k_msleep(LED_THREAD_SLEEP_TIME_MS);
    blueLed.toggle();
    k_msleep(LED_THREAD_SLEEP_TIME_MS);
    redLed.toggle();
    k_msleep(LED_THREAD_SLEEP_TIME_MS);
  }
}

static void temperatureThreadHandler(void) {
  // Local variable to hold the received event on the queue
  event_t event;

  // Reference device from device tree
  const struct device *temperatureDevice = DEVICE_DT_GET(DT_NODELABEL(die_temp));

  // Create local object using the device tree device
  Temperature temperature(temperatureDevice);

  printk("Waiting for EVENT_NETWORK_AVAILABLE event...\r\n");

  // Wait here forever for an event
  k_msgq_get(&queue, &event, K_FOREVER);

  // Check if we got the right event
  if (event == EVENT_NETWORK_AVAILABLE) {
    printk("Received EVENT_NETWORK_AVAILABLE event\r\n");
  } else {
    printk("Received a wrong event (%u)\r\n", event);
  }

  // Continuously read temperature
  while (true) {
    printk("CPU temperature: %.1f °C\r\n", temperature.read());
    k_msleep(TEMPERATURE_THREAD_SLEEP_TIME_MS);
  }
}

static void buttonThreadHandler(void) {
  // Reference device from device tree
  const struct gpio_dt_spec buttonGpio = GPIO_DT_SPEC_GET_OR(DT_ALIAS(sw0), gpios, {0});

  // Create local object using the device tree device
  Button button(&buttonGpio);

  // Continuously check if a button is pressed
  while (true) {
    if (button.isPressed()) {
      printk("Button is pressed\r\n");
    }
    k_msleep(BUTTON_THREAD_SLEEP_TIME_MS);
  }
}

static void networkThreadHandler(void) {
  // Get the singleton instance of Network
  Network& network = Network::getInstance();

  // Set up the lambda callback for IP address notification
  network.onGotIP([](const char *ipAddress) {
    event_t event = EVENT_NETWORK_AVAILABLE;

    printf("Got IP address: %s\r\n", ipAddress);
    k_msgq_put(&queue, &event, K_NO_WAIT);
  });

  // Start the network and wait for an IP address
  network.start();

  while (true) {
    k_msleep(NETWORK_THREAD_SLEEP_TIME_MS);
  }
}

static void httpClientThreadHandler(void) {
  // Create local object using the device tree device
  HttpClient client((char *)"142.251.143.132", 80);

  // Send GET request and handle response in a lambda callback
  client.get("/", [](uint8_t *response, uint32_t length) {
    printk("Response[%d]: %.*s\r\n", length, length, response);
  });

  while (true) {
    k_msleep(NETWORK_THREAD_SLEEP_TIME_MS);
  }
}

static void storageThreadHandler(void) {
  int ret = 0;
  uint8_t buffer[16] = {0};

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
