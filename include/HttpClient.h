#ifndef HTTP_CLIENT_H
#define HTTP_CLIENT_H

#include <stdint.h>
#include <functional>

#include <zephyr/net/net_ip.h>

static constexpr uint32_t HTTP_CLIENT_RESPONSE_BUFFER_SIZE = 512;

class HttpClient {

public:
  std::function<void(uint8_t *, uint32_t)> callback;

  HttpClient(const char *server, const uint16_t port);
  ~HttpClient();
  int get(const char *endpoint, std::function<void(uint8_t *, uint32_t)> callback);

private:
  int socket;
  const char *server;
  const uint16_t port;
  struct sockaddr socketAddress;
  uint8_t httpResponseBuffer[HTTP_CLIENT_RESPONSE_BUFFER_SIZE];

};

#endif // HTTP_CLIENT_H