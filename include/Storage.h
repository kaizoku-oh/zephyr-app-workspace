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
  // Private constructor to prevent direct instantiation
  Storage();
  ~Storage();

  // Static member to hold the singleton instance
  static Storage instance;

  struct nvs_fs fs;
};

#endif // STORAGE_H
