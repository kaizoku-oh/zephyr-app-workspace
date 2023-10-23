#include <zephyr/device.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/sys/printk.h>

#include "Temperature.h"

Temperature::Temperature(const struct device *device) {

  if (device == NULL) {
    printk("Error: Invalid argument\r\n");
    return;
  }

  _device = device;

  if (!device_is_ready(_device)) {
    printk("Error: Device is not ready\r\n");
    return;
  }
}

Temperature::~Temperature() {
  // Destructor is automatically called when the object goes out of scope or is explicitly deleted
}

double Temperature::read() {
  int ret = 0;
  struct sensor_value value = {0};

  ret = sensor_sample_fetch(_device);
  if (ret) {
      printk("Failed to fetch sample (%d)\n", ret);
      return 0;
  }

  ret = sensor_channel_get(_device, SENSOR_CHAN_DIE_TEMP, &value);
  if (ret) {
      printk("Failed to get data (%d)\n", ret);
      return 0;
  }

  return sensor_value_to_double(&value);
}
