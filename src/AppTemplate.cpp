#include <zephyr/sys/printk.h>
#include "AppTemplate.h"

AppTemplate::AppTemplate() {
  printk("Object is created with AppTemplate() empty constructor!\r\n");
}

AppTemplate::AppTemplate(int configuration) {
  _myConfiguration = configuration;
  printk("Object is created with AppTemplate(int configuration) constructor!\r\n");
}

AppTemplate::~AppTemplate() {
  // Destructor is automatically called when object go out of scope or is explicitly deleted
  printk("Object is destroyed!\r\n");
}

void AppTemplate::run() {
}
