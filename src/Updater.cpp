// Lib C includes
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>

// Zephyr includes
#include <app_version.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/sys/reboot.h>
#include <zephyr/zbus/zbus.h>
#include <zephyr/dfu/mcuboot.h>
#include <zephyr/dfu/flash_img.h>
#include <zephyr/storage/flash_map.h>
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(Updater);

// User C++ class headers
#include "EventManager.h"
#include "HttpClient.h"

// Function declaration of thread handlers
static void UpdaterThreadHandler();

// Function declaration of event actions
static void onNetworkAvailableAction();
static void startOtaUpdateAction();

// Download firmware image from remote HTTP server
static void downloadImage(const char *url, const char *endpoint);

// Confirm the currently run image
static void confirmImage();

// ZBUS subscribers definition
ZBUS_SUBSCRIBER_DEFINE(UpdaterSubscriber, 4);

// Add a subscriber observer to ZBUS events channel
ZBUS_CHAN_ADD_OBS(eventsChannel, UpdaterSubscriber, 4);

// Thread definition
K_THREAD_DEFINE(UpdaterThread, 4096, UpdaterThreadHandler, NULL, NULL, NULL, 7, 0, 0);

static const event_action_pair_t eventActionList[] {
  {EVENT_BUTTON_PRESSED,    startOtaUpdateAction    },
  {EVENT_NETWORK_AVAILABLE, onNetworkAvailableAction},
};

static volatile bool networkIsAvailable = false;
static struct flash_img_context flashContext = {0};
static size_t totalDownloadSize = 0;
static size_t currentDownloadedSize = 0;

static void UpdaterThreadHandler() {
  int ret = 0;
  event_t event = {.id = EVENT_INITIAL_VALUE};

  confirmImage();

  while (true) {
    ret = waitForEvent(&UpdaterSubscriber, &event, K_FOREVER);
    if (ret == 0) {
      processEvent(&event, eventActionList, EVENT_ACTION_LIST_SIZE(eventActionList));
    }
  }
}

static void onNetworkAvailableAction() {
  LOG_INF("Network is now available");
  networkIsAvailable = true;
}

static void startOtaUpdateAction() {
  if (networkIsAvailable) {
    downloadImage("192.168.1.22", "/zephyr.signed.bin");
    if (boot_request_upgrade(BOOT_UPGRADE_TEST)) {
      LOG_ERR("Failed to mark the image in slot 1 as pending");
      return;
    }
    LOG_INF("You need to reboot your system to apply the new update");
  } else {
    LOG_WRN("Network is not available, cannot start update");
  }
}

static void downloadImage(const char *url, const char *endpoint) {
  int ret = 0;
  assert(url);
  assert(endpoint);

  HttpClient client((char *)url);

  // Initialize context needed for writing the image to the flash
  ret = flash_img_init(&flashContext);
  if (ret < 0) {
    LOG_ERR("Flash context init error: %d", ret);
    return;
  }

  // Download image
  client.get(endpoint, [](HttpResponse *response) {
    int ret = 0;
    size_t totalSizeWrittenToFlash = 0;

    // Get the total firmware size
    if (totalDownloadSize == 0) {
      totalDownloadSize = response->totalSize;
      LOG_INF("File size to download: %d bytes", totalDownloadSize);
    }

    // Write the downloaded chunk to flash
    ret = flash_img_buffered_write(&flashContext,
                                   response->body,
                                   response->bodyLength,
                                   (response->isComplete));
    if (ret < 0) {
      LOG_ERR("Flash write error: %d", ret);
      return;
    }
    k_msleep(10);

    // Increase currently downloaded size each time we download a chunk
    currentDownloadedSize += response->bodyLength;
    printk("\rDownloading: %d/%d bytes", currentDownloadedSize, totalDownloadSize);

    // Verify that we received all the chunks
    if (response->isComplete) {
        totalSizeWrittenToFlash = flash_img_bytes_written(&flashContext);
        LOG_INF("\r\nFile size downloaded: %d bytes", currentDownloadedSize);
        LOG_INF("File size written to flash: %d bytes", currentDownloadedSize);
        if ((currentDownloadedSize == totalDownloadSize) &&
            (totalDownloadSize == totalSizeWrittenToFlash)) {
          LOG_INF("Download completed successfully");
      } else {
        LOG_ERR("The size written to flash is different than the one downloaded");
      }
    }
  });
}

static void confirmImage() {
  int ret = 0;
  bool imageOk = false;
  struct mcuboot_img_header header = {0};

  if (boot_read_bank_header(FIXED_PARTITION_ID(slot0_partition), &header, sizeof(header)) != 0) {
    LOG_ERR("Failed to read slot0_partition header");
    return;
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
      return;
    }

    LOG_INF("Marked image as OK");
    ret = boot_erase_img_bank(FIXED_PARTITION_ID(slot1_partition));
    if (ret) {
      LOG_ERR("Failed to erase second slot: %d", ret);
      return;
    }
  }
}

