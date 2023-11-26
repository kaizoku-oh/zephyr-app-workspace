#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/net/coap_client.h>

#include "CoapClient.h"

static void response_callback(int16_t code,
                              size_t offset,
                              const uint8_t *payload,
                              size_t length,
                              bool lastBlock,
                              void *userData);

CoapClient::CoapClient() {
  int ret = 0;
  struct coap_client client = {0};

  ret = coap_client_init(&client, NULL);
  if (ret < 0) {
    printk("Error: Failed to initialize CoAP client\r\n");
    return;
  }
}

CoapClient::~CoapClient() {
  // Destructor is automatically called when the object goes out of scope or is explicitly deleted
}

void CoapClient::get() {
  int ret = 0;
  struct coap_client client = {0};
  struct coap_client_request request;

  request.method = COAP_METHOD_GET;
  request.confirmable = true;
  request.path = "test";
  request.fmt = COAP_CONTENT_FORMAT_TEXT_PLAIN;
  request.cb = response_callback;
  request.payload = NULL;
  request.len = 0;
  ret = coap_client_req(&client, 0, NULL, &request, -1);
  if (ret < 0) {
    printk("Error: Failed to send CoAP GET request\r\n");
    return;
  }
}

void CoapClient::post() {
}

void CoapClient::put() {
}

static void response_callback(int16_t code,
                              size_t offset,
                              const uint8_t *payload,
                              size_t length,
                              bool lastBlock,
                              void *userData) {
  if (code >= 0) {
    printk("CoAP response from server %d\r\n", code);
    if (lastBlock) {
      printk("Last packet received\r\n");
    }
  } else {
    printk("Error in sending request %d\r\n", code);
  }
}
