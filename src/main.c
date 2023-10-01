#include <stdlib.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>

#define MAIN_THREAD_SLEEP_TIME_MS (1000)

int main(void)
{
  while (true) {
    printk("Running in main thread...\r\n");
    k_msleep(MAIN_THREAD_SLEEP_TIME_MS);
  }

  return EXIT_SUCCESS;
}
