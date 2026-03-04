#include <esp_littlefs.h>
#include <esp_log.h>

#include "ap.h"
#include "csi_command.h"
#include "dataanalysis.h"
#include "esp_vfs_fat.h"
#include "ml/forest/forest.h"
#include "wear_levelling.h"
#include <fstream>
#include <iostream>

namespace flash {
#define MOUNT_PATH "/data"
static const char *TAG = "CSI";
void prepare_flash() {
  esp_vfs_littlefs_conf_t conf = {
      .base_path = "/littlefs",
      .partition_label = "littlefs",
      .format_if_mount_failed = true,
      .dont_mount = false,
  };
  esp_err_t ret = esp_vfs_littlefs_register(&conf);

  if (ret != ESP_OK) {
    if (ret == ESP_FAIL) {
      ESP_LOGE(TAG, "Failed to mount or format filesystem");
    } else if (ret == ESP_ERR_NOT_FOUND) {
      ESP_LOGE(TAG, "Failed to find LittleFS partition");
    } else {
      ESP_LOGE(TAG, "Failed to initialize LittleFS (%s)", esp_err_to_name(ret));
    }
    return;
  }

  size_t total = 0, used = 0;
  ret = esp_littlefs_info(conf.partition_label, &total, &used);
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "Failed to get LittleFS partition information (%s)",
             esp_err_to_name(ret));
    esp_littlefs_format(conf.partition_label);
  } else {
    ESP_LOGI(TAG, "Partition size: total: %d, used: %d", total, used);
  }
}
void init_storage() {
  prepare_flash();
  std::ifstream model("/littlefs/model.bin");
  alglib::dfunserialize(model, forest::forest_model);
  model.close();
}
} // namespace flash
