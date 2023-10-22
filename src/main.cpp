#include <stdlib.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>

#include "MyClass.h"

#define MAIN_THREAD_SLEEP_TIME_MS (1000)

int main(void) {

  {
    printk("============================== myObject1 ==============================\r\n");
    MyClass myObject1;

    printk("sizeof(myObject1) = %d bytes\r\n", sizeof(myObject1));

    myObject1.setMyAttribute(5);
    printk("myObject1.getMyAttribute() = %d\r\n", myObject1.getMyAttribute());

    myObject1.setMyAttribute(6);
    printk("myObject1.getMyAttribute() = %d\r\n", myObject1.getMyAttribute());
  }

  {
    printk("============================== myObject2 ==============================\r\n");
    MyClass myObject2(55);

    printk("sizeof(myObject2) = %d bytes\r\n", sizeof(myObject2));

    printk("myObject2.getMyAttribute() = %d\r\n", myObject2.getMyAttribute());

    myObject2.setMyAttribute(66);
    printk("myObject2.getMyAttribute() = %d\r\n", myObject2.getMyAttribute());
  }

  printk("=======================================================================\r\n");

  while (true) {
    printk("Running in main thread...\r\n");
    k_msleep(MAIN_THREAD_SLEEP_TIME_MS);
  }

  return EXIT_SUCCESS;
}
