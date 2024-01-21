/*
Usage example:

// Lib C includes
#include <stdint.h>
#include <stdbool.h>

// Zephyr includes
#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>

// User C++ class headers
#include "HttpClient.h"

// Create an HTTP client as a local object
HttpClient client((char *)"10.42.0.1", 1880);

// Send GET request and handle response in a lambda callback
client.get("/data", [](uint8_t *response, uint32_t length) {
  uint32_t index = 0;

  // Skip headers by looking for the start of the json message
  for (index = 0; index < length; index++) {
    if (response[index] == '{') {
      break;
    }
  }
  printk("\r\nResponse(%d bytes): %.*s\r\n", length-index, length-index, &response[index]);
});
}
*/

#ifndef HTTP_CLIENT_H
#define HTTP_CLIENT_H

#include <stdint.h>
#include <stdbool.h>
#include <functional>

#include <zephyr/net/net_ip.h>
#include <zephyr/net/http/client.h>

static constexpr uint32_t HTTP_CLIENT_RESPONSE_BUFFER_SIZE = 512;

typedef struct {
  uint8_t *header;
  uint32_t headerLength;
  uint8_t *body;
  uint32_t bodyLength;
  uint32_t totalSize;
  bool isComplete;
} HttpResponse;

class HttpClient {

public:
  std::function<void(HttpResponse *response)> callback;

  HttpClient(char *server, uint16_t port = 80);
  ~HttpClient();
  int get(const char *endpoint, std::function<void(HttpResponse *)> callback);
  int post(const char *endpoint,
           const char *data,
           uint32_t length,
           std::function<void(HttpResponse *)> callback);

private:
  int sock;
  char *server;
  uint16_t port;
  struct sockaddr socketAddress;
  uint8_t responseBuffer[HTTP_CLIENT_RESPONSE_BUFFER_SIZE];

};

#endif // HTTP_CLIENT_H