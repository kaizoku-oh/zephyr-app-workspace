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

// Thread handler function declaration
static void httpGetRequestThreadHandler();

// Delay value used inside thread loops to yield back to scheduler
static constexpr uint32_t HTTP_CLIENT_THREAD_SLEEP_TIME_MS = 100;

// Threads definition
K_THREAD_DEFINE(httpGetRequestThread, 1024, httpGetRequestThreadHandler, NULL, NULL, NULL, 7, 0, 0);

static void httpGetRequestThreadHandler() {
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

  while (true) {
    k_msleep(HTTP_CLIENT_THREAD_SLEEP_TIME_MS);
  }
}
*/

#ifndef HTTP_CLIENT_H
#define HTTP_CLIENT_H

#include <stdint.h>
#include <functional>

#include <zephyr/net/net_ip.h>

static constexpr uint32_t HTTP_CLIENT_RESPONSE_BUFFER_SIZE = 512;

class HttpClient {

public:
  std::function<void(uint8_t *, uint32_t)> callback;

  HttpClient(char *server, uint16_t port);
  ~HttpClient();
  int get(const char *endpoint, std::function<void(uint8_t *, uint32_t)> callback);
  int post(const char *endpoint,
           const char *data,
           uint32_t length,
           std::function<void(uint8_t *, uint32_t)> callback);

private:
  int sock;
  char *server;
  uint16_t port;
  struct sockaddr socketAddress;
  uint8_t httpResponseBuffer[HTTP_CLIENT_RESPONSE_BUFFER_SIZE];

};

#endif // HTTP_CLIENT_H