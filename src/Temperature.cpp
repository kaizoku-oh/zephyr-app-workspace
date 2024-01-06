// Lib C
#include <assert.h>

// Zephyr includes
#include <zephyr/device.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(Temperature);

// User C++ class headers
#include "Temperature.h"

Temperature::Temperature(const struct device *device) {
  assert(device);

  this->device = device;

  if (!device_is_ready(this->device)) {
    LOG_ERR("Error: Device is not ready\r\n");
    return;
  }
}

Temperature::~Temperature() {
  // Destructor is automatically called when the object goes out of scope or is explicitly deleted
}

double Temperature::read() {
  int ret = 0;
  struct sensor_value value = {0};

  ret = sensor_sample_fetch(this->device);
  if (ret) {
    LOG_ERR("Failed to fetch sample (%d)\n", ret);
    return 0;
  }

  ret = sensor_channel_get(this->device, SENSOR_CHAN_DIE_TEMP, &value);
  if (ret) {
    LOG_ERR("Failed to get data (%d)\n", ret);
    return 0;
  }

  return sensor_value_to_double(&value);
}
