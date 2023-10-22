#ifndef MY_CLASS_H
#define MY_CLASS_H

class MyClass {

public:
  MyClass();
  MyClass(int attribute);
  int getMyAttribute();
  void setMyAttribute(int attribute);
  ~MyClass();

private:
  int _myAttribute;

};

#endif // MY_CLASS_H
