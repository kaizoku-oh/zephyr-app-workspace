#ifndef COAP_CLIENT_H
#define COAP_CLIENT_H

class CoapClient {

public:
  CoapClient();
  ~CoapClient();
  void get();
  void post();
  void put();

private:
  int _dutyCycle;

};

#endif // COAP_CLIENT_H
