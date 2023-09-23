#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(golioth_hello, LOG_LEVEL_DBG);

#include <zephyr/drivers/gpio.h>

#include <net/golioth/system_client.h>
#include <samples/common/net_connect.h>

#define SLEEP_TIME_MS (3000)
#define LED_1_NODE    DT_ALIAS(led1)

static void golioth_on_connect(struct golioth_client *client);
static enum golioth_settings_status golioth_on_setting(const char *key,
                                                       const struct golioth_settings_value *value);

static K_SEM_DEFINE(connected, 0, 1);
static struct golioth_client *client = GOLIOTH_SYSTEM_CLIENT_GET();
static const struct gpio_dt_spec blueLED = GPIO_DT_SPEC_GET(LED_1_NODE, gpios);

int main(void)
{
  int counter = 0;
  int err;

  LOG_DBG("Start Hello sample");

  if (IS_ENABLED(CONFIG_GOLIOTH_SAMPLES_COMMON)) {
    net_connect();
  }

  if (IS_ENABLED(CONFIG_GOLIOTH_SETTINGS)) {
    err = golioth_settings_register_callback(client, golioth_on_setting);
    if (err) {
      LOG_ERR("Failed to register settings callback: %d", err);
    }
  }

  client->on_connect = golioth_on_connect;
  golioth_system_client_start();

  k_sem_take(&connected, K_FOREVER);

  while (true) {
    gpio_pin_toggle_dt(&blueLED);

    LOG_INF("Sending hello! %d", counter++);

    err = golioth_send_hello(client);
    if (err) {
      LOG_WRN("Failed to send hello! (%d)", err);
    }

    k_msleep(SLEEP_TIME_MS);
  }

  return 0;
}

static void golioth_on_connect(struct golioth_client *client)
{
  k_sem_give(&connected);
}

enum golioth_settings_status golioth_on_setting(const char *key,
                                                const struct golioth_settings_value *value)
{
  LOG_DBG("Received setting: key = %s, type = %d, value = %.*s",
          key,
          value->type,
          value->string.len,
          value->string.ptr);
  return GOLIOTH_SETTINGS_SUCCESS;
}
