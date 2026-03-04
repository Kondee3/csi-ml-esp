#include "csi_command.h"
#include "csi_wifi.h"
#include "esp_console.h"
#include <cstring>

namespace repl_console {
#define MOUNT_PATH "/littlefs"
// #define MOUNT_PATH "/data"
#define HISTORY_PATH MOUNT_PATH "/history.txt"

void setupREPL() {
  esp_console_repl_t *repl = NULL;
  esp_console_repl_config_t repl_config = ESP_CONSOLE_REPL_CONFIG_DEFAULT();
  repl_config.prompt = "esp_csi >";
  repl_config.max_cmdline_length = 64;
  repl_config.history_save_path = HISTORY_PATH;

  esp_console_dev_usb_serial_jtag_config_t jtag_config =
      ESP_CONSOLE_DEV_USB_SERIAL_JTAG_CONFIG_DEFAULT();
  esp_console_new_repl_usb_serial_jtag(&jtag_config, &repl_config, &repl);
  csi_command::register_csi_cmd();
  ESP_ERROR_CHECK(esp_console_start_repl(repl));
}
} // namespace repl_console
