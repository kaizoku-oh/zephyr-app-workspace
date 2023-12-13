#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <inttypes.h>

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/sys/printk.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/net/net_if.h>
#include <zephyr/net/net_core.h>
#include <zephyr/net/net_context.h>
#include <zephyr/net/net_mgmt.h>
#include <zephyr/smf.h>

#include "Button.h"
#include "Led.h"
#include "Temperature.h"
#include "Serial.h"
#include "Network.h"
#include "HttpClient.h"

static constexpr uint32_t MAIN_THREAD_SLEEP_TIME_MS = 1000;
static constexpr uint32_t LED_THREAD_SLEEP_TIME_MS = 500;
static constexpr uint32_t TEMPERATURE_THREAD_SLEEP_TIME_MS = 1000;
static constexpr uint32_t BUTTON_THREAD_SLEEP_TIME_MS = 100;
static constexpr uint32_t NETWORK_THREAD_SLEEP_TIME_MS = 1000;
static constexpr uint32_t HTTP_CLIENT_THREAD_SLEEP_TIME_MS = 2000;

static constexpr uint32_t EVENT_GOT_IP  = BIT(0);

static void ledThreadHandler(void);
static void temperatureThreadHandler(void);
static void buttonThreadHandler(void);
static void networkThreadHandler(void);
static void httpClientThreadHandler(void);

K_THREAD_DEFINE(ledThread, 512, ledThreadHandler, NULL, NULL, NULL, 7, 0, 0);
K_THREAD_DEFINE(temperatureThread, 512, temperatureThreadHandler, NULL, NULL, NULL, 7, 0, 0);
K_THREAD_DEFINE(buttonThread, 512, buttonThreadHandler, NULL, NULL, NULL, 7, 0, 0);
K_THREAD_DEFINE(networkThread, 512, networkThreadHandler, NULL, NULL, NULL, 7, 0, 0);
K_THREAD_DEFINE(httpClientThread, 512, httpClientThreadHandler, NULL, NULL, NULL, 7, 0, 0);

enum demo_state { S0, S1 };

struct s_object {
  /* This must be first */
  struct smf_ctx ctx;

  /* Events */
  struct k_event smf_event;
  int32_t events;

  /* Other state specific data add here */
} s_obj;

/* State S0 */
static void s0_entry(void *o)
{
  printk("STATE0\n");
}

static void s0_run(void *o)
{
  struct s_object *s = (struct s_object *)o;

  /* Change states on Button Press Event */
  if (s->events & EVENT_GOT_IP) {
    smf_set_state(SMF_CTX(&s_obj), &demo_states[S1]);
  }
}

/* State S1 */
static void s1_entry(void *o)
{
  printk("STATE1\n");
}

static void s1_run(void *o)
{
  struct s_object *s = (struct s_object *)o;

  /* Change states on Button Press Event */
  if (s->events & EVENT_GOT_IP) {
    smf_set_state(SMF_CTX(&s_obj), &demo_states[S0]);
  }
}

static const struct smf_state demo_states[] = {
  [S0] = SMF_CREATE_STATE(s0_entry, s0_run, NULL),
  [S1] = SMF_CREATE_STATE(s1_entry, s1_run, NULL),
};

int main(void) {
  /* Initialize the event */
  k_event_init(&s_obj.smf_event);

  /* Set initial state */
  smf_set_initial(SMF_CTX(&s_obj), &demo_states[S0]);

  while (true) {
    /* Block until an event is detected */
    s_obj.events = k_event_wait(&s_obj.smf_event, EVENT_GOT_IP, true, K_FOREVER);

    /* State machine terminates if a non-zero value is returned */
    ret = smf_run_state(SMF_CTX(&s_obj));

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
    blueLed.toggle();
    redLed.toggle();
    k_msleep(LED_THREAD_SLEEP_TIME_MS);
  }
}

static void temperatureThreadHandler(void) {
  // Reference device from device tree
  const struct device *temperatureDevice = DEVICE_DT_GET(DT_NODELABEL(die_temp));

  // Create local object using the device tree device
  Temperature temperature(temperatureDevice);

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
    printf("Got IP address: %s\r\n", ipAddress);
  });

  // Start the network and wait for an IP address
  network.start();

  while (true) {
    k_msleep(NETWORK_THREAD_SLEEP_TIME_MS);
  }
}

static void httpClientThreadHandler(void) {
  // Create an HTTP client as a local object
  HttpClient client("142.250.180.174", 80);

  // Send a GET request and response in the lambda callback
  client.get("/", [](uint8_t *data, uint32_t length) {
    printk("Received response: %.*s\r\n", length, data);
  });

  while (true) {
    k_msleep(HTTP_CLIENT_THREAD_SLEEP_TIME_MS);
  }
}
