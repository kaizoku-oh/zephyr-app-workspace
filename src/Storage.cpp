#include <zephyr/sys/printk.h>
#include <zephyr/drivers/flash.h>
#include <zephyr/storage/flash_map.h>
#include <zephyr/fs/nvs.h>

#include "Storage.h"

#include <zephyr/drivers/flash.h>
#include <zephyr/storage/flash_map.h>
#include <zephyr/fs/nvs.h>

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
    printk("Flash device %s is not ready\r\n", this->fs.flash_device->name);
    return;
  }

  // Get the size and start offset of flash page at certain flash offset
  this->fs.offset = NVS_PARTITION_OFFSET;
  ret = flash_get_page_info_by_offs(this->fs.flash_device, this->fs.offset, &pageInfo);
  if (ret != 0) {
    printk("Unable to get page inf>>o\r\n");
    return;
  }

  // Mount an NVS file system onto the flash device
  this->fs.sector_size = pageInfo.size;
  this->fs.sector_count = 2U;
  ret = nvs_mount(&this->fs);
  if (ret != 0) {
    printk("Flash Init failed -(%d)\r\n", ret);
    return;
  }
}

Storage::~Storage() {
}

int Storage::read(uint16_t id, uint8_t *buffer, size_t length) {
  int ret = 0;

  // Read an entry by its id from the NVS file system
  ret = nvs_read(&this->fs, id, buffer, sizeof(buffer));
  if (ret > 0) {
    printk("Found it (%d bytes)!\r\n", ret);
    printk("id: %d, value: %.*s\r\n", id, ret, buffer);
  } else {
    printk("id %d not found in NVS!\r\n", id);
  }

  return ret;
}

int Storage::write(uint16_t id, uint8_t *data, size_t length) {
  int ret = 0;

  // Write an entry by its id to the NVS file system
  ret = nvs_write(&this->fs, id, data, sizeof(data));
  if (ret > 0) {
    printk("%d bytes written to NVS\r\n", ret);
  } else if (ret == 0) {
    printk("Rewriting the same data already stored, nothing is written to NVS\r\n");
  } else {
    printk("Failed to write to NVS: -(%d)\r\n", ret);
  }

  return ret;
}

int Storage::remove(uint16_t id) {
  int ret = 0;

  // Delete an entry from the NVS file system
  ret = nvs_delete(&this->fs, id);
  if (ret == 0) {
    printk("Entry with id %d is deleted from NVS\r\n", id);
  } else {
    printk("Failed to delete entry with id %d from NVS: -(%d)\r\n", id, ret);
  }

  return ret;
}

int Storage::clear() {
  int ret = 0;

  // Clear the NVS file system from flash
  ret = nvs_clear(&this->fs);
  if (ret == 0) {
    printk("NVS file system is cleared from flash\r\n");
  } else {
    printk("Failed to clear NVS file system from flash: -(%d)\r\n", ret);
  }

  return ret;
}
