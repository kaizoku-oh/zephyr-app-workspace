#ifndef SERIAL_H
#define SERIAL_H

#include <stdint.h>
#include <functional>

class Serial {

public:
  const struct device *device;
  std::function<void(uint8_t *, uint32_t)> callback;

  Serial(const struct device *device);
  ~Serial();

  void write(uint8_t *data, uint32_t length);
  void read(uint8_t *data, uint32_t *length);
  void onReceive(std::function<void(uint8_t *, uint32_t)> callback);

private:
};

#endif // SERIAL_H
