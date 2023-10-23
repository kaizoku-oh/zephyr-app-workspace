#ifndef TEMPERATURE_H
#define TEMPERATURE_H

class Temperature {

public:
  Temperature(const struct device *device);
  ~Temperature();
  double read();

private:
  const struct device *_device;

};

#endif // TEMPERATURE_H
