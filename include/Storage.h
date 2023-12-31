/*
Usage example:

// Lib C includes
#include <stdint.h>
#include <stdbool.h>

// Zephyr includes
#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>

// User C++ class headers
#include "Storage.h"

// Thread handler function declaration
static void storageThreadHandler();

// Delay value used inside thread loops to yield back to scheduler
static constexpr uint32_t STORAGE_THREAD_SLEEP_TIME_MS = 1000;

// Threads definition
K_THREAD_DEFINE(storageThread, 1024, storageThreadHandler, NULL, NULL, NULL, 7, 0, 0);

static void storageThreadHandler() {
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

  while (true) {
    k_msleep(STORAGE_THREAD_SLEEP_TIME_MS);
  }
}
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

  int read(uint16_t id, uint8_t *buffer, size_t length);
  int write(uint16_t id, uint8_t *data, size_t length);
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
