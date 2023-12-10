#include <zephyr/sys/printk.h>

#include <zephyr/net/net_ip.h>
#include <zephyr/net/socket.h>
#include <zephyr/net/http/client.h>

#include "HttpClient.h"

static void httpResponseCallback(struct http_response *response,
                                 enum http_final_call finalData,
                                 void *userData);

HttpClient::HttpClient(const char *server, const uint16_t port) {
  int ret = 0;
  socklen_t length = 0;

  // 1. Initialize attributes
  this->socket = 0;
  this->server = server;
  this->port = port;
  memset(this->socketAddress, 0, sizeof(this->socketAddress));

  // 2. Create socket
  net_sin(this->socketAddress)->sin_family = AF_INET;
  net_sin(this->socketAddress)->sin_port = htons(port);
  inet_pton(AF_INET, server, &net_sin(this->socketAddress)->sin_addr);
  this->socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

  if (*this->socket < 0) {
    printk("Failed to create HTTP socket (%d)\r\n", -errno);
  }
}

HttpClient::~HttpClient() {
  // Destructor is automatically called when the object goes out of scope or is explicitly deleted
}

int HttpClient::get(const char *endpoint, std::function<void(uint8_t *, uint32_t)> callback) {
  int ret = 0;
  struct http_request request = {0};

  if (callback == nullptr) {
    printk("Failed to register callback\r\n");
    ret = -errno;
    return ret;
  }

  this->callback = callback;

  // 1. Open TCP connection
  ret = connect(this->socket, this->socketAddress, sizeof(this->socketAddress));
  if (ret < 0) {
    printk("Cannot connect to remote (%d)", -errno);
    ret = -errno;
    return ret;
  }

  // 2. Send GET request
  request.method = HTTP_GET;
  request.url = endpoint;
  request.host = this->server;
  request.protocol = "HTTP/1.1";
  request.response = httpResponseCallback;
  request.recv_buf = this->httpResponseBuffer;
  request.recv_buf_len = sizeof(this->httpResponseBuffer);
  ret = http_client_req(this->socket, &request, 3000, (void *)this->callback);
  if (ret < 0) {
    printk("Error sending GET request\r\n");
    ret = -errno;
    return ret;
  }

  // 3. Close TCP connection
  close(this->socket);

  return ret;
}

static void httpResponseCallback(struct http_response *response,
                                 enum http_final_call finalData,
                                 void *userData) {
  HttpClient *httpClientInstance = nullptr;

  if (userData == nullptr) {
    printk("Invalid callback parameters\r\n");
    return;
  }

  httpClientInstance = static_cast<HttpClient *>(userData);

  if (finalData == HTTP_DATA_MORE) {
    printk("Partial data received (%zd bytes)", response->data_len);
  } else if (finalData == HTTP_DATA_FINAL) {
    printk("All the data received (%zd bytes)", response->data_len);
  }
  printk("Response status %s", response->http_status);

  if (httpClientInstance->callback) {
    httpClientInstance->callback(response->recv_buf, response->data_len);
  }
}
