#include "csi_command.h"

#include <cstdint>
#include <cstring>

#include "argtable3/argtable3.h"
#include "cJSON.h"
#include "ml/cnn/cnn.h"
#include "csi_wifi.h"
#include "esp_console.h"
#include "ml/forest/forest.h"
#include "ml/nn/nn.h"

namespace csi_command {
const char *mode = "collect";
bool benchmark = false;
static struct {
  arg_str *new_mode;
  arg_int *amount;
  arg_lit *benchmark;
  struct arg_end *end;
} run_args;

// New task to process CSI data
void print_data(int8_t *buf) {
  cJSON *root = cJSON_CreateObject();
  cJSON *elements = cJSON_CreateArray();
  for (int i = 0; i < 112; i++)
    cJSON_AddItemToArray(elements, cJSON_CreateNumber(buf[i]));
  cJSON_AddItemToObject(root, "data", elements);
  char *data = cJSON_Print(elements);
  printf("%s\n", data);
  cJSON_Delete(root);
  cJSON_free(data);
}

void csi_processing_task(void *pvParameters) {
  int8_t buf[112];

  int count = reinterpret_cast<int>(pvParameters);
  while (count > 0) {
    if (xQueueReceive(csi_wifi::xCsiQueue, &buf, portMAX_DELAY)) {
      count--;
      if (!strcmp(mode, "collect")) {
        print_data(buf);
      }
      if (!strcmp(mode, "cnn")) {
        cnn::convolutional_neural_networks(buf);
      } else if (!strcmp(mode, "nn")) {
        nn::neural_networks(buf);
      } else if (!strcmp(mode, "forest")) {
        forest::tree(buf);
      }
    }
  }
  csi_wifi::wifi_ping_router_stop();
  xQueueReset(csi_wifi::xCsiQueue);
  vTaskDelete(NULL);
}

static int run_task(int argc, char **argv) {
  if (!csi_wifi::xCsiQueue) {
    csi_wifi::xCsiQueue = xQueueCreate(8, sizeof(int8_t[112]));
  }
  int nerrors = arg_parse(argc, argv, reinterpret_cast<void **>(&run_args));
  if (nerrors != 0) {
    arg_print_errors(stderr, run_args.end, argv[0]);
    return 1;
  }
  // Init WiFi connection
  if (!csi_wifi::wifi_connected)
    csi_wifi::wifi_init_sta();
  else
    csi_wifi::wifi_ping_router_start();
  mode = run_args.new_mode->sval[0];
  benchmark = run_args.benchmark->count > 0;
  xTaskCreate(csi_processing_task, "csi_proc", 16384,
              reinterpret_cast<void **>(run_args.amount->ival[0]), 10, NULL);

  return 0;
}

void register_csi_cmd() {
  run_args.new_mode =
      arg_str0("m", "mode", "<mode>",
               "Mode of operation: <collect> <forest> <nn> <cnn>");
  run_args.amount =
      arg_int0("a", "amount", "<amount>", "Amount of csi packets");
  run_args.benchmark = arg_lit0("b", "benchmark", "Run benchmark");
  run_args.end = arg_end(3);
  esp_console_cmd_t cmd = {.command = "csi",
                           .help = "Choose mode and amount",
                           .hint = NULL,
                           .func = run_task,
                           .argtable = &run_args};
  ESP_ERROR_CHECK(esp_console_cmd_register(&cmd));
}
} // namespace csi_command
