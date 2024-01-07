// Lib C
#include <string.h>
#include <assert.h>

// Zephyr includes
#include <zephyr/net/net_ip.h>
#include <zephyr/net/socket.h>
#include <zephyr/net/http/client.h>
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(HttpClient);

// User C++ class headers
#include "HttpClient.h"

static void responseCallback(http_response *response,
                                 enum http_final_call finalData,
                                 void *userData);

HttpClient::HttpClient(char *server, uint16_t port) {
  assert(server);
  assert(port);

  // 1. Initialize attributes
  this->sock = 0;
  this->server = server;
  this->port = port;
  memset((void *)&this->socketAddress, 0x00, sizeof(this->socketAddress));
  memset((void *)&this->responseBuffer, 0x00, sizeof(this->responseBuffer));
}

HttpClient::~HttpClient() {
  // Destructor is automatically called when the object goes out of scope or is explicitly deleted
}

int HttpClient::get(const char *endpoint, std::function<void(HttpResponse *)> callback) {
  int ret = 0;
  struct http_request request = {0};

  assert(endpoint);
  assert(callback);

  // 0. Create socket
  memset((void *)&this->socketAddress, 0x00, sizeof(this->socketAddress));
  net_sin(&this->socketAddress)->sin_family = AF_INET;
  net_sin(&this->socketAddress)->sin_port = htons(port);
  inet_pton(AF_INET, server, &net_sin(&this->socketAddress)->sin_addr);
  this->sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

  if (this->sock < 0) {
    LOG_ERR("Failed to create HTTP socket (%d)\r\n", -errno);
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
  request.response = responseCallback;
  request.recv_buf = this->responseBuffer;
  request.recv_buf_len = sizeof(this->responseBuffer);
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
                     std::function<void(HttpResponse *)> callback) {
  int ret = 0;
  struct http_request request = {0};

  assert(endpoint);
  assert(data);
  assert(length);
  assert(callback);

  // 0. Create socket
  memset((void *)&this->socketAddress, 0x00, sizeof(this->socketAddress));
  net_sin(&this->socketAddress)->sin_family = AF_INET;
  net_sin(&this->socketAddress)->sin_port = htons(port);
  inet_pton(AF_INET, server, &net_sin(&this->socketAddress)->sin_addr);
  this->sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

  if (this->sock < 0) {
    LOG_ERR("Failed to create HTTP socket (%d)\r\n", -errno);
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
  request.response = responseCallback;
  request.payload = data;
  request.payload_len = length;
  request.recv_buf = this->responseBuffer;
  request.recv_buf_len = sizeof(this->responseBuffer);
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

static void responseCallback(http_response *response, enum http_final_call finalData, void *userData) {
  HttpClient *clientInstance  = static_cast<HttpClient *>(userData);
  HttpResponse httpResponse = {0};

  assert(userData);
  assert(response);
  assert(clientInstance);

  if (response->body_found) {
    httpResponse.header = response->recv_buf;
    httpResponse.headerLength = response->data_len - response->body_frag_len;
    httpResponse.body = response->body_frag_start;
    httpResponse.bodyLength = response->body_frag_len;
    httpResponse.totalSize = response->content_length;
    httpResponse.isComplete = (finalData == HTTP_DATA_FINAL);
  } else {
    httpResponse.header = response->recv_buf;
    httpResponse.headerLength = response->data_len;
  }

  if (clientInstance->callback) {
    clientInstance->callback(&httpResponse);
  }
}
