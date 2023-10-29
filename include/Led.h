#ifndef LED_H
#define LED_H

class Led {

public:
  Led(const struct gpio_dt_spec *gpio);
  ~Led();
  void on();
  void off();
  void toggle();
  void blink(int dutyCycle);

private:
  int _dutyCycle;
  const struct gpio_dt_spec *_device;

};

#endif // LED_H