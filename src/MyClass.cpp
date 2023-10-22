#include <zephyr/sys/printk.h>
#include "MyClass.h"

MyClass::MyClass() {
  printk("Object is created with MyClass() empty constructor!\r\n");
}

MyClass::MyClass(int attribute) {
  _myAttribute = attribute;
  printk("Object is created with MyClass(int attribute) constructor!\r\n");
}

MyClass::~MyClass() {
  // Destructor is automatically called when object go out of scope or is explicitly deleted
  printk("Object is destroyed!\r\n");
}

int MyClass::getMyAttribute() {
  return _myAttribute;
}

void MyClass::setMyAttribute(int attribute) {
  _myAttribute = attribute;
}