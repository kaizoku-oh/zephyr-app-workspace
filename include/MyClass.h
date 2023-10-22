#ifndef MY_CLASS_H
#define MY_CLASS_H

class MyClass {

public:
  MyClass();
  MyClass(int attribute);
  ~MyClass();
  int getMyAttribute();
  void setMyAttribute(int attribute);

private:
  int _myAttribute;

};

#endif // MY_CLASS_H
