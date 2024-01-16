// Lib C includes
#include <stdlib.h>
#include <stdbool.h>

// Zephyr includes
#include <zephyr/kernel.h>

int main(void) {

  while (true) {
    k_msleep(100);
  }

  return EXIT_SUCCESS;
}
