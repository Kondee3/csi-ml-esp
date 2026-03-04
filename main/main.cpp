#include "csi_wifi.h"
#include "esp_task_wdt.h"
#include "flash.h"
#include "repl_console.h"
#include <esp_log.h>
#include <esp_sntp.h>
#include <nvs.h>
#include <nvs_flash.h>

static const char *TAG = "CSI";
using namespace std;

extern "C" void app_main(void) {
  esp_task_wdt_deinit();
  ESP_LOGI(TAG, "Starting WiFi CSI example");
  esp_err_t ret = nvs_flash_init();
  if (ret == ESP_ERR_NVS_NO_FREE_PAGES ||
      ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
    ESP_ERROR_CHECK(nvs_flash_erase());
    ret = nvs_flash_init();
  }
  ESP_ERROR_CHECK(ret);

  flash::init_storage();
  repl_console::setupREPL();
}
