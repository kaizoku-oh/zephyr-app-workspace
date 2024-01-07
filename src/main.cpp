// Lib C includes
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

// Zephyr includes
#include <zephyr/kernel.h>
#include <zephyr/zbus/zbus.h>
#include <zephyr/fs/nvs.h>
#include <zephyr/dfu/mcuboot.h>
#include <zephyr/dfu/flash_img.h>
#include <zephyr/drivers/flash.h>
#include <zephyr/storage/flash_map.h>
#include <zephyr/net/http/client.h>
#include <app_version.h>
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(main);

// User C++ class headers
#include "EventManager.h"
#include "Network.h"
#include "HttpClient.h"

static struct flash_img_context flash_ctx = {0};
static struct flash_img_check fic = {0};
static uint8_t file_hash[32] = {0};
static size_t current_downloaded_size = 0;
static size_t total_download_size = 0;
static size_t body_len = 0;
static int ret = 0;
static int type = 0;
static int downloaded = 0;
static int download_progress = 0;
static uint8_t *body_data = NULL;
static volatile uint32_t calls = 0;

static volatile bool networkAvailable = false;

int main(void) {

  // On boot verify if current image is confirmed if not confirm it
  bool image_ok = false;

  image_ok = boot_is_img_confirmed();
  LOG_INF("Image is%s confirmed OK", image_ok ? "" : " not");
  if (!image_ok) {
    ret = boot_write_img_confirmed();
    if (ret < 0) {
      LOG_ERR("Couldn't confirm this image: %d", ret);
      return ret;
    }

    LOG_DBG("Marked image as OK");
    ret = boot_erase_img_bank(FIXED_PARTITION_ID(slot1_partition));
    if (ret) {
      LOG_ERR("Failed to erase second slot: %d", ret);
      return ret;
    }
  }

  // Get the singleton instance of Network
  Network& network = Network::getInstance();

  // Set up the lambda callback for IP address notification
  network.onGotIP([](const char *ipAddress) {
    networkAvailable = true;
  });

  // Start the network and wait for an IP address
  network.start();

  // Wait for network to be available
  while (!networkAvailable);

  // Read the Bootloader major version and the application version
  struct mcuboot_img_header header = {0};

  if (boot_read_bank_header(FIXED_PARTITION_ID(slot0_partition), &header, sizeof(header)) != 0) {
    LOG_ERR("Failed to read slot0_partition header");
    return 0;
  }

  LOG_INF("Bootloader version: %d.x.y", header.mcuboot_version);
  LOG_INF("Application version: %d.%d.%d",
          header.h.v1.sem_ver.major,
          header.h.v1.sem_ver.minor,
          header.h.v1.sem_ver.revision);

  // // Initialize context needed for writing the image to the flash
  // flash_img_init(&flash_ctx);

  // printk("\r\n");

  // // Download image
  // HttpClient client((char *)"192.168.1.22", 80);
  // client.get("/zephyr.signed.bin", [](struct http_response *response, enum http_final_call final_data) {
  //   if (total_download_size == 0) {
  //     total_download_size = response->content_length;
  //   }

  //   // LOG_INF("Chunk length: %d", response->data_len);
  //   // LOG_INF("Callback called %d times", ++calls);

  //   if (response->body_found) {
  //     body_data = response->body_frag_start;
  //     body_len = response->body_frag_len;

  //     current_downloaded_size += body_len;

  //     printk("\rDownloading: %d/%d bytes", current_downloaded_size, total_download_size);
  //     k_msleep(10);

  //     // LOG_HEXDUMP_INF(body_data, body_len, "");

  //     ret = flash_img_buffered_write(&flash_ctx, body_data, body_len, final_data == HTTP_DATA_FINAL);
  //     if (ret < 0) {
  //       LOG_ERR("Flash write error: %d", ret);
  //       return;
  //     }
  //   }

  //   current_downloaded_size = flash_img_bytes_written(&flash_ctx);

  //   if (final_data == HTTP_DATA_FINAL) {
  //     printk("\r\n");
  //     LOG_INF("Download complete");
  //     LOG_INF("Image size: %d kb", total_download_size / 1024);
  //   }
  // });

  // // Verify the hash of the stored firmware
  // fic.match = file_hash;
  // fic.clen = current_downloaded_size;
  // if (flash_img_check(&flash_ctx, &fic, FIXED_PARTITION_ID(slot1_partition))) {
  //   LOG_ERR("Firmware - flash validation has failed");
  //   return 0;
  // }

  // // Request mcuboot to upgrade
  // if (boot_request_upgrade(BOOT_UPGRADE_TEST)) {
  //   LOG_ERR("Failed to mark the image in slot 1 as pending");
  //   return 0;
  // }

  while (true) {
    k_msleep(1000);
  }

  return EXIT_SUCCESS;
}
