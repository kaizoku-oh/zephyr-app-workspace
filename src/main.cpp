#include <stdlib.h>
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/sys/printk.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/uart.h>

#include "Button.h"
#include "Led.h"
#include "Temperature.h"

#define MAIN_THREAD_SLEEP_TIME_MS (100)

static void serial_callback(const struct device *dev, void *userData);

uint8_t rxBuffer[8] = {0};

int main(void) {
  const struct gpio_dt_spec buttonGpio = GPIO_DT_SPEC_GET_OR(DT_ALIAS(sw0), gpios, {0});

  const struct gpio_dt_spec greenLedGpio = GPIO_DT_SPEC_GET_OR(DT_ALIAS(led0), gpios, {0});
  const struct gpio_dt_spec blueLedGpio = GPIO_DT_SPEC_GET_OR(DT_ALIAS(led1), gpios, {0});
  const struct gpio_dt_spec redLedGpio = GPIO_DT_SPEC_GET_OR(DT_ALIAS(led2), gpios, {0});

  const struct device *temperatureDevice = DEVICE_DT_GET(DT_NODELABEL(die_temp));


  /* ******************************************************************************************* */
  const struct device *serialDevice = DEVICE_DT_GET(DT_NODELABEL(usart2));

  if (!device_is_ready(serialDevice)) {
    printk("Unable to get UART device\r\n");
    return -EIO;
  }

  int ret = uart_irq_callback_user_data_set(serialDevice, serial_callback, NULL);
  if (ret < 0) {
    if (ret == -ENOTSUP) {
      printk("Interrupt-driven UART API support not enabled\r\n");
    } else if (ret == -ENOSYS) {
      printk("UART device does not support interrupt-driven API\r\n");
    } else {
      printk("Error setting UART callback: %d\r\n", ret);
    }
    return 0;
  }

  uart_irq_rx_enable(serialDevice);

  uart_poll_out(serialDevice, 'B');
  uart_poll_out(serialDevice, 'a');
  uart_poll_out(serialDevice, 'y');
  uart_poll_out(serialDevice, 'r');
  uart_poll_out(serialDevice, 'e');
  uart_poll_out(serialDevice, 'm');
  uart_poll_out(serialDevice, '\r');
  uart_poll_out(serialDevice, '\n');
  /* ******************************************************************************************* */

  Button button(&buttonGpio);

  Led greenLed(&greenLedGpio);
  Led blueLed(&blueLedGpio);
  Led redLed(&redLedGpio);

  Temperature temperature(temperatureDevice);

  while (true) {
    if (button.isPressed()) {
      printk("Button is pressed\r\n");
      printk("CPU temperature: %.1f °C\r\n", temperature.read());

      printk("rxBuffer[%d] = {", sizeof(rxBuffer));
      for (size_t index = 0; index < sizeof(rxBuffer); index++) {
        printk("'%c'%s", rxBuffer[index], (index == (sizeof(rxBuffer) -1)) ? "}\r\n" : ", ");
      }

      redLed.toggle();
      greenLed.toggle();
      blueLed.toggle();
    }
    k_msleep(MAIN_THREAD_SLEEP_TIME_MS);
  }

  return EXIT_SUCCESS;
}

static void serial_callback(const struct device *dev, void *userData) {
  uint8_t rxByte = 0;
  static uint8_t index = 0;

  if (!uart_irq_update(dev)) {
    return;
  }

  if (!uart_irq_rx_ready(dev)) {
    return;
  }

  while (uart_fifo_read(dev, &rxByte, sizeof(rxByte))) {
    if (index >= sizeof(rxBuffer)) {
      index = 0;
    }
    rxBuffer[index++] = rxByte;
  }

  printk("serial_callback()\r\n");
}