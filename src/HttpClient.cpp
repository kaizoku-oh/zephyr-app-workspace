// Zephyr includes
#include <zephyr/net/net_ip.h>
#include <zephyr/net/socket.h>
#include <zephyr/net/http/client.h>
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(HttpClient);

// User C++ class headers
#include "HttpClient.h"

static void httpResponseCallback(struct http_response *response,
                                 enum http_final_call finalData,
                                 void *userData);

HttpClient::HttpClient(char *server, uint16_t port) {
  // 1. Initialize attributes
  this->sock = 0;
  this->server = server;
  this->port = port;
}

HttpClient::~HttpClient() {
  // Destructor is automatically called when the object goes out of scope or is explicitly deleted
}

int HttpClient::get(const char *endpoint, std::function<void(uint8_t *, uint32_t)> callback) {
  int ret = 0;
  struct http_request request = {0};

  // 0. Create socket
  memset((void *)&this->socketAddress, 0, sizeof(this->socketAddress));
  net_sin(&this->socketAddress)->sin_family = AF_INET;
  net_sin(&this->socketAddress)->sin_port = htons(port);
  inet_pton(AF_INET, server, &net_sin(&this->socketAddress)->sin_addr);
  this->sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

  if (this->sock < 0) {
    LOG_ERR("Failed to create HTTP socket (%d)\r\n", -errno);
  }

  if (callback == nullptr) {
    LOG_ERR("Failed to register callback\r\n");
    ret = -errno;
    return ret;
  }

  this->callback = callback;

  // 1. Open TCP connection
  ret = connect(this->sock, &this->socketAddress, sizeof(this->socketAddress));
  if (ret < 0) {
    LOG_ERR("Cannot connect to remote (%d)", -errno);
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
  ret = http_client_req(this->sock, &request, 5000, (void *)&this->callback);
  if (ret < 0) {
    LOG_ERR("Error sending GET request\r\n");
    ret = -errno;
    return ret;
  }

  // 3. Close TCP connection
  close(this->sock);

  return ret;
}

int HttpClient::post(const char *endpoint,
                     const char *data,
                     uint32_t length,
                     std::function<void(uint8_t *, uint32_t)> callback) {
  int ret = 0;
  struct http_request request = {0};

  // 0. Create socket
  memset((void *)&this->socketAddress, 0, sizeof(this->socketAddress));
  net_sin(&this->socketAddress)->sin_family = AF_INET;
  net_sin(&this->socketAddress)->sin_port = htons(port);
  inet_pton(AF_INET, server, &net_sin(&this->socketAddress)->sin_addr);
  this->sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

  if (this->sock < 0) {
    LOG_ERR("Failed to create HTTP socket (%d)\r\n", -errno);
  }

  if (callback == nullptr) {
    LOG_ERR("Failed to register callback\r\n");
    ret = -errno;
    return ret;
  }

  this->callback = callback;

  // 1. Open TCP connection
  ret = connect(this->sock, &this->socketAddress, sizeof(this->socketAddress));
  if (ret < 0) {
    LOG_ERR("Cannot connect to remote (%d)", -errno);
    ret = -errno;
    return ret;
  }

  // 2. Send POST request
  request.method = HTTP_POST;
  request.host = this->server;
  request.url = endpoint;
  request.protocol = "HTTP/1.1";
  request.response = httpResponseCallback;
  request.payload = data;
  request.payload_len = length;
  request.recv_buf = this->httpResponseBuffer;
  request.recv_buf_len = sizeof(this->httpResponseBuffer);
  ret = http_client_req(this->sock, &request, 5000, (void *)this);
  if (ret < 0) {
    LOG_ERR("Error sending POST request\r\n");
    ret = -errno;
    return ret;
  }

  // 3. Close TCP connection
  close(this->sock);

  return ret;
}

static void httpResponseCallback(struct http_response *response,
                                 enum http_final_call finalData,
                                 void *userData) {
  HttpClient *httpClientInstance = nullptr;

  if (userData == nullptr) {
    LOG_ERR("Invalid callback parameters\r\n");
    return;
  }

  httpClientInstance = static_cast<HttpClient *>(userData);

  if (finalData == HTTP_DATA_MORE) {
    LOG_DBG("Partial data received (%zd bytes)", response->data_len);
  } else if (finalData == HTTP_DATA_FINAL) {
    LOG_DBG("All the data received (%zd bytes)", response->data_len);
  }
  LOG_DBG("Response status %s", response->http_status);

  if (httpClientInstance->callback) {
    httpClientInstance->callback(response->recv_buf, response->data_len);
  }
}
