// Lib C includes
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

// Zephyr includes
#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/zbus/zbus.h>
#include <zephyr/net/net_ip.h>
#include <zephyr/net/socket.h>
#include <zephyr/net/net_mgmt.h>
#include <zephyr/net/http/client.h>
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(main);

// User C++ class headers
#include "EventManager.h"
#include "Network.h"
#include "HttpClient.h"
#include "Button.h"

#if defined(CONFIG_NET_SOCKETS_SOCKOPT_TLS)
#define CA_CERTIFICATE_TAG 1
#include <zephyr/net/tls_credentials.h>
#include "ca_certificate.h"
#endif // CONFIG_NET_SOCKETS_SOCKOPT_TLS

static event_t event = {.id = EVENT_INITIAL_VALUE};

static volatile bool networkIsAvailable = false;

static void start_http_client();

int main(void) {

  // Connect to network
  Network& network = Network::getInstance();

  network.onGotIP([](const char *ipAddress) {
    event.id = EVENT_NETWORK_AVAILABLE;
    // publishEvent(&event, K_NO_WAIT);
    networkIsAvailable = true;
  });

  network.start();

  // Monitor button press
  const struct gpio_dt_spec buttonGpio = GPIO_DT_SPEC_GET_OR(DT_ALIAS(sw0), gpios, {0});
  Button button(&buttonGpio);

  while (!networkIsAvailable);

  start_http_client();

  while (true) {
    if (button.isPressed()) {
      event.id = EVENT_BUTTON_PRESSED;
      publishEvent(&event, K_NO_WAIT);
    }
    k_msleep(100);
  }

  return EXIT_SUCCESS;
}

static void start_http_client() {
  int ret = -1;
  int sock = 0;
  int resolve_attempts = 10;
  char ip_addr_str[INET_ADDRSTRLEN] {0};
  struct sockaddr_in *ipv4 = NULL;
  struct zsock_addrinfo *addr = NULL;
  struct zsock_addrinfo hints = {0};
#if defined(CONFIG_NET_SOCKETS_SOCKOPT_TLS)
  sec_tag_t sec_tag_opt[] = { CA_CERTIFICATE_TAG };
  int protocol = IPPROTO_TLS_1_2;

  ret = tls_credential_add(CA_CERTIFICATE_TAG,
                           TLS_CREDENTIAL_CA_CERTIFICATE,
                           ca_certificate,
                           sizeof(ca_certificate));
  if (ret < 0) {
    LOG_ERR("Failed to register public certificate: %d", ret);
    return;
  }
#else
  int protocol = IPPROTO_TCP;
#endif // CONFIG_NET_SOCKETS_SOCKOPT_TLS

  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;

  // Try to resolve DNS
  while (resolve_attempts--) {
    ret = getaddrinfo("google.com", "443", &hints, &addr);
    if (ret == 0) {
      break;
    }
    k_sleep(K_MSEC(1));
  }

  if (ret != 0) {
    LOG_ERR("Could not resolve dns: %d", ret);
    return;
  }

  // Extract and print IP address
  ipv4 = (struct sockaddr_in *)addr->ai_addr;
  inet_ntop(AF_INET, &(ipv4->sin_addr), ip_addr_str, INET_ADDRSTRLEN);
  LOG_INF("IPv4 address for google.com: %s", ip_addr_str);

  sock = socket(addr->ai_family, SOCK_STREAM, protocol);
  if (sock < 0) {
    LOG_ERR("Failed to create TCP socket");
    return;
  }

#if defined(CONFIG_NET_SOCKETS_SOCKOPT_TLS)
  // Set TLS connection options
  ret = setsockopt(sock, SOL_TLS, TLS_SEC_TAG_LIST, sec_tag_opt, sizeof(sec_tag_opt));
  if (ret < 0) {
    close(sock);
    LOG_ERR("Failed to set TLS_TAG option");
    return;
  }
  ret = setsockopt(sock, SOL_TLS, TLS_HOSTNAME, "google.com", sizeof("google.com"));
  if (ret < 0) {
    close(sock);
    LOG_ERR("Failed to set TLS_HOSTNAME option");
    return;
  }
#endif // CONFIG_NET_SOCKETS_SOCKOPT_TLS

  // Connect to remote address
  ret = connect(sock, addr->ai_addr, addr->ai_addrlen);
  if (ret < 0) {
    close(sock);
    LOG_ERR("Failed to connect to server");
    return;
  }

  zsock_freeaddrinfo(addr);
}
