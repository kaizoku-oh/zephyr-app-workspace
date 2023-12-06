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
