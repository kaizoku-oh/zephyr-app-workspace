#ifndef BUTTON_H
#define BUTTON_H

#include <stdbool.h>

class Button {

public:
  Button(const struct gpio_dt_spec *gpio);
  ~Button();
  bool isPressed();

private:
  const struct gpio_dt_spec *_device;

};

#endif // BUTTON_H
