#ifndef APP_TEMPLATE_H
#define APP_TEMPLATE_H

class AppTemplate {

public:
  AppTemplate();
  AppTemplate(int configuration);
  ~AppTemplate();
  void run();

private:
  int _myConfiguration;

};

#endif // APP_TEMPLATE_H
