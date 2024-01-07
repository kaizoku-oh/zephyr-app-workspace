// Lib C includes
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
LOG_MODULE_REGISTER(ServiceUpdater);

// User C++ class headers
#include "EventManager.h"
#include "HttpClient.h"

// Function declaration of thread handlers
static void serviceUpdaterThreadHandler();

// ZBUS subscribers definition
ZBUS_SUBSCRIBER_DEFINE(serviceUpdaterSubscriber, 4);

// Add a subscriber observer to ZBUS events channel
ZBUS_CHAN_ADD_OBS(eventsChannel, serviceUpdaterSubscriber, 4);

// Thread definition
K_THREAD_DEFINE(serviceUpdaterThread, 4096, serviceUpdaterThreadHandler, NULL, NULL, NULL, 7, 0, 0);

static volatile bool networkIsAvailable = false;
static size_t totalDownloadSize = 0;
static size_t currentDownloadedSize = 0;
static struct flash_img_context flashContext = {0};
static struct flash_img_check flashImageCheck = {0};
static uint8_t fileHash[32] = {0};

static void serviceUpdaterThreadHandler() {
  int ret = 0;
  bool imageOk = false;

  // Initialize local variable to hold the event
  event_t event = {.id = EVENT_INITIAL_VALUE};

  // Used to figure out on which channel the event came from
  const struct zbus_channel *channel = NULL;

  // Create an HTTP client as a local object
  HttpClient client((char *)"192.168.1.22");

  // Read the bootloader major version and the application version
  struct mcuboot_img_header header = {0};

  if (boot_read_bank_header(FIXED_PARTITION_ID(slot0_partition), &header, sizeof(header)) != 0) {
    LOG_ERR("Failed to read slot0_partition header");
    while (true);
  }
  LOG_INF("Bootloader version: %d.x.y", header.mcuboot_version);
  LOG_INF("Application version: %d.%d.%d",
          header.h.v1.sem_ver.major,
          header.h.v1.sem_ver.minor,
          header.h.v1.sem_ver.revision);

  // On boot verify if current image is confirmed, if not confirm it
  imageOk = boot_is_img_confirmed();
  LOG_INF("Image is%s confirmed OK", imageOk ? "" : " not");
  if (!imageOk) {
    ret = boot_write_img_confirmed();
    if (ret < 0) {
      LOG_ERR("Couldn't confirm this image: %d", ret);
      while (true);
    }

    LOG_DBG("Marked image as OK");
    ret = boot_erase_img_bank(FIXED_PARTITION_ID(slot1_partition));
    if (ret) {
      LOG_ERR("Failed to erase second slot: %d", ret);
      while (true);
    }
  }

  while (true) {

    // Wait forever for an event
    ret = zbus_sub_wait(&serviceUpdaterSubscriber, &channel, K_FOREVER);

    // Check if notification is received
    if (ret == 0) {

      // Make sure the event came on the right channel
      if (&eventsChannel == channel) {

        // Read the event
        ret = zbus_chan_read(&eventsChannel, &event, K_NO_WAIT);

        if (ret == 0) {

          LOG_DBG("Subscriber <%s> received event <%s> on <%s>\r\n",
                  serviceUpdaterSubscriber.name,
                  EVENT_ID_TO_STRING(event.id),
                  channel->name);

          // Make sure the event is the one we are interested in
          switch (event.id) {

            case EVENT_NETWORK_AVAILABLE: {
              networkIsAvailable = true;
              break;
            }

            case EVENT_BUTTON_PRESSED: {
              if (networkIsAvailable) {
                LOG_INF("Started checking for updates...");

                // Initialize context needed for writing the image to the flash
                flash_img_init(&flashContext);

                // Download image
                client.get("/zephyr.signed.bin", [](HttpResponse *response) {
                  int ret = 0;
                  size_t totalSizeWrittenToFlash = 0;

                  if (totalDownloadSize == 0) {
                    totalDownloadSize = response->totalSize;
                    LOG_INF("File size to download: %d bytes", totalDownloadSize);
                  }

                  ret = flash_img_buffered_write(&flashContext,
                                                 response->body,
                                                 response->bodyLength,
                                                 (response->isComplete));
                  if (ret < 0) {
                    LOG_ERR("Flash write error: %d", ret);
                    return;
                  }
                  k_msleep(10);

                  currentDownloadedSize += response->bodyLength;
                  printk("\rDownloading: %d/%d bytes", currentDownloadedSize, totalDownloadSize);

                  if (response->isComplete) {
                    printk("\r\n");
                    totalSizeWrittenToFlash = flash_img_bytes_written(&flashContext);
                    LOG_INF("File size downloaded: %d bytes", currentDownloadedSize);
                    LOG_INF("File size written to flash: %d bytes", currentDownloadedSize);
                    LOG_INF("Image size: %d kb", totalDownloadSize / 1024);
                    if ((currentDownloadedSize == totalDownloadSize) &&
                        (totalDownloadSize == totalSizeWrittenToFlash)) {
                      LOG_INF("Download completed successfully");
                    } else {
                      LOG_ERR("The size written to flash is different than the one downloaded");
                    }
                  }
                });

                // // Verify the hash of the stored firmware
                // flashImageCheck.match = fileHash;
                // flashImageCheck.clen = totalDownloadSize;
                // if (flash_img_check(&flashContext, &flashImageCheck, FIXED_PARTITION_ID(slot1_partition))) {
                //   LOG_ERR("Firmware - flash validation has failed");
                //   return;
                // }

                // Request mcuboot to upgrade
                if (boot_request_upgrade(BOOT_UPGRADE_TEST)) {
                  LOG_ERR("Failed to mark the image in slot 1 as pending");
                  return;
                }

              } else {
                LOG_WRN("Cannot update because network is not available");
              }
              break;
            }

            default: {
              // I'm not interested in this event
              LOG_DBG("<%s> is not interested in this event: <%s>",
                      serviceUpdaterSubscriber.name,
                      EVENT_ID_TO_STRING(event.id));
              break;
            }
          }
        } else {
          // Something wrong happened while reading event from channel
          LOG_ERR("Something wrong happened while reading from channel: %d", ret);
        }
      } else {
        // I'm not interested in this channel
        LOG_WRN("<%s> is not interested in this channel: <%s>",
                serviceUpdaterSubscriber.name,
                channel->name);
      }
    } else {
      // Something wrong happened while waiting for event
      LOG_ERR("Something wrong happened while waiting for event: %d", ret);
    }
  }
}
