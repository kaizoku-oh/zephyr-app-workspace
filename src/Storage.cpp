// Zephyr includes
#include <zephyr/drivers/flash.h>
#include <zephyr/storage/flash_map.h>
#include <zephyr/fs/nvs.h>
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(Storage);

// User C++ class headers
#include "Storage.h"

#define NVS_PARTITION_DEVICE FIXED_PARTITION_DEVICE(storage_partition)
#define NVS_PARTITION_OFFSET FIXED_PARTITION_OFFSET(storage_partition)

// Define the static member
Storage Storage::instance;

Storage& Storage::getInstance() {
  // Return the singleton instance
  return instance;
}

Storage::Storage() {
  int ret = 0;
  struct flash_pages_info pageInfo = {0};

  // Verify that the device been successfully initialized
  this->fs.flash_device = NVS_PARTITION_DEVICE;
  ret = device_is_ready(this->fs.flash_device);
  if (ret == 0) {
    LOG_ERR("Flash device %s is not ready\r\n", this->fs.flash_device->name);
    return;
  }

  // Get the size and start offset of flash page at certain flash offset
  this->fs.offset = NVS_PARTITION_OFFSET;
  ret = flash_get_page_info_by_offs(this->fs.flash_device, this->fs.offset, &pageInfo);
  if (ret != 0) {
    LOG_ERR("Unable to get page inf>>o\r\n");
    return;
  }

  // Mount an NVS file system onto the flash device
  this->fs.sector_size = pageInfo.size;
  this->fs.sector_count = 2U;
  ret = nvs_mount(&this->fs);
  if (ret != 0) {
    LOG_ERR("Flash Init failed -(%d)\r\n", ret);
    return;
  }
}

Storage::~Storage() {
}

int Storage::read(uint16_t id, void *buffer, size_t length) {
  int ret = 0;

  // Read an entry by its id from the NVS file system
  ret = nvs_read(&this->fs, id, buffer, sizeof(buffer));

  return ret;
}

int Storage::write(uint16_t id, void *data, size_t length) {
  int ret = 0;

  // Write an entry by its id to the NVS file system
  ret = nvs_write(&this->fs, id, data, sizeof(data));
  if (ret > 0) {
    LOG_DBG("%d bytes written to NVS\r\n", ret);
  } else if (ret == 0) {
    LOG_DBG("Rewriting the same data already stored, nothing is written to NVS\r\n");
  } else {
    LOG_ERR("Failed to write to NVS: -(%d)\r\n", ret);
  }

  return ret;
}

int Storage::remove(uint16_t id) {
  int ret = 0;

  // Delete an entry from the NVS file system
  ret = nvs_delete(&this->fs, id);
  if (ret == 0) {
    LOG_DBG("Entry with id %d is deleted from NVS\r\n", id);
  } else {
    LOG_ERR("Failed to delete entry with id %d from NVS: -(%d)\r\n", id, ret);
  }

  return ret;
}

int Storage::clear() {
  int ret = 0;

  // Clear the NVS file system from flash
  ret = nvs_clear(&this->fs);
  if (ret == 0) {
    LOG_DBG("NVS file system is cleared from flash\r\n");
  } else {
    LOG_ERR("Failed to clear NVS file system from flash: -(%d)\r\n", ret);
  }

  return ret;
}
