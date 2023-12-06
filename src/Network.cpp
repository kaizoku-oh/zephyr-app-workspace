#include <zephyr/sys/printk.h>
#include <zephyr/net/net_core.h>
#include <zephyr/net/net_context.h>

#include "Network.h"

static void netMgmtCallback(struct net_mgmt_event_callback *cb, uint32_t event, struct net_if *iface);

// Define the static member
Network Network::instance;

Network& Network::getInstance() {
  // Return the singleton instance
  return instance;
}

Network::Network() {
  net_mgmt_init_event_callback(&this->_mgmtEventCb, netMgmtCallback, NET_EVENT_IPV4_ADDR_ADD);
  net_mgmt_add_event_callback(&this->_mgmtEventCb);
  this->_netIface = net_if_get_default();
}

Network::~Network() {
}

void Network::start() {
  net_dhcpv4_start(this->_netIface);
}

void Network::onGotIP(std::function<void(const char *)> callback) {
  if (callback == nullptr) {
    printk("Failed to register callback\r\n");
    return;
  }

  this->callback = callback;
}

static void netMgmtCallback(struct net_mgmt_event_callback *cb, uint32_t event, struct net_if *iface) {
  char buffer[NET_IPV4_ADDR_LEN] = {0};
  char *ipAddress;

  if (event == NET_EVENT_IPV4_ADDR_ADD) {
    if (iface->config.ip.ipv4->unicast[0].addr_type == NET_ADDR_DHCP) {
      ipAddress = net_addr_ntop(AF_INET,
                                &iface->config.ip.ipv4->unicast[0].address.in_addr,
                                buffer,
                                sizeof(buffer));
      printk("Address: %s\r\n", ipAddress);
      printk("Subnet: %s\r\n", net_addr_ntop(AF_INET,
                                             &iface->config.ip.ipv4->netmask,
                                             buffer,
                                             sizeof(buffer)));
      printk("Router: %s\r\n", net_addr_ntop(AF_INET,
                                             &iface->config.ip.ipv4->gw,
                                             buffer,
                                             sizeof(buffer)));

      // Notify the callback if it's set
      if (Network::getInstance().callback) {
        Network::getInstance().callback(ipAddress);
      }
    }
  }
}
