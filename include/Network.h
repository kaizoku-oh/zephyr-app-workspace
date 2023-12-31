/*
Usage example:

// Lib C includes
#include <stdbool.h>

// Zephyr includes
#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>

// User C++ class headers
#include "Network.h"

// Thread handler function declaration
static void networkThreadHandler();

// Delay value used inside thread loops to yield back to scheduler
static constexpr uint32_t NETWORK_THREAD_SLEEP_TIME_MS = 100;

// Threads definition
K_THREAD_DEFINE(networkThread, 1024, networkThreadHandler, NULL, NULL, NULL, 7, 0, 0);

static void networkThreadHandler() {
  // Get the singleton instance of Network
  Network& network = Network::getInstance();

  // Set up the lambda callback for IP address notification
  network.onGotIP([](const char *ipAddress) {
    printk("Got IP address: %s\r\n", ipAddress);
  });

  // Start the network and wait for an IP address
  network.start();

  while (true) {
    k_msleep(NETWORK_THREAD_SLEEP_TIME_MS);
  }
}
*/

#ifndef NETWORK_H
#define NETWORK_H

#include <stdint.h>
#include <functional>
#include <zephyr/net/net_if.h>
#include <zephyr/net/net_mgmt.h>

class Network {
public:
  std::function<void(const char *)> callback;

  // Static method to access the singleton instance
  static Network& getInstance();

  void start();
  void onGotIP(std::function<void(const char *)> callback);

private:
  // Private constructor to prevent direct instantiation
  Network();
  ~Network();

  // Static member to hold the singleton instance
  static Network instance;
  struct net_mgmt_event_callback _mgmtEventCb;
  struct net_if *_netIface;
};

#endif // NETWORK_H
