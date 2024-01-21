/*
Usage example:

// Lib C includes
#include <stdint.h>

// Zephyr includes
#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>

// User C++ class headers
#include "Storage.h"

int ret = 0;

// Buffer used to read and write to NVS
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

// Save double in storage but first covert it to string
int ret = 0;
double piNumber = 3.14;
char piString[6] = {0};
char jsonString[32] = {0};

printk("sizeof(piNumber) = %d\r\n", sizeof(piNumber));

ret = snprintf(piString, sizeof(piString), "%.2f", piNumber);

// Write data to NVS
ret = storage.write(0, piString, sizeof(piString));
if (ret > 0) {
  printk("Data written successfully to NVS!\r\n");
} else {
  printk("Failed to write data to NVS\r\n");
}

// Read data from NVS
memset(piString, 0x00, sizeof(piString));
ret = storage.read(0, piString, sizeof(piString));
if (ret > 0) {
  printk("Data read successfully from NVS: %.*s\r\n", sizeof(piString), piString);
} else {
  printk("Failed to read data from NVS\r\n");
}

ret = snprintf(jsonString, sizeof(jsonString), "{\"pi\": %s}", piString);

printk("jsonString = %s", jsonString);
*/

#ifndef STORAGE_H
#define STORAGE_H

#include <stdlib.h>
#include <stdint.h>

#include <zephyr/fs/nvs.h>

class Storage {
public:
  // Static method to access the singleton instance
  static Storage& getInstance();

  int read(uint16_t id, void *buffer, size_t length);
  int write(uint16_t id, void *data, size_t length);
  int remove(uint16_t id);
  int clear();

private:
  // Private constructor and destructor to prevent direct instantiation and destruction
  Storage();
  ~Storage();

  // Static member to hold the singleton instance
  static Storage instance;

  // NVS file system, internally protected by a mutex
  struct nvs_fs fs;
};

#endif // STORAGE_H
