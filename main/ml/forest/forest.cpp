#include "ap.h"
#include "csi_command.h"
#include "dataanalysis.h"
#include "esp_log.h"
#include "features.h"
#include "freertos/FreeRTOS.h"
#include <esp_heap_caps.h>
#include <esp_system.h>
#include <esp_timer.h>
using namespace alglib;

namespace forest {
bool merged_loaded;
real_2d_array xy;
decisionforest forest_model;

void print_compact_info() {
  // Pamięć RAM (internal)
  size_t total_ram = heap_caps_get_total_size(MALLOC_CAP_INTERNAL);
  size_t free_ram = heap_caps_get_free_size(MALLOC_CAP_INTERNAL);
  float used_ram_percent = 100.0f * (total_ram - free_ram) / total_ram;

  // Pamięć PSRAM (jeśli dostępna)
  size_t total_psram = heap_caps_get_total_size(MALLOC_CAP_SPIRAM);
  size_t free_psram = heap_caps_get_free_size(MALLOC_CAP_SPIRAM);
  float used_psram_percent =
      total_psram > 0 ? 100.0f * (total_psram - free_psram) / total_psram
                      : 0.0f;

  printf("=== Pamięć systemu ===\n");
  printf("RAM:   %u / %u bajtów (%.2f%% użycia)\n",
         (unsigned)(total_ram - free_ram), (unsigned)total_ram,
         used_ram_percent);
  if (total_psram > 0)
    printf("PSRAM: %u / %u bajtów (%.2f%% użycia)\n",
           (unsigned)(total_psram - free_psram), (unsigned)total_psram,
           used_psram_percent);
  else
    printf("PSRAM: niedostępna\n");
}


void tree(int8_t *values) {
  int64_t start = esp_timer_get_time();
  real_1d_array y = "[]";
  std::vector<float> features;
  features.reserve(21);
  features::extractFeatures(values, features);
  real_1d_array z;
  features::vector_to_r1da(features, z);
  dfprocess(forest_model, z, y);
  ae_int_t i = dfclassify(forest_model, z);
  if (csi_command::benchmark)
    print_compact_info();

  int64_t elapsed = esp_timer_get_time() - start;
  const char *class_names[] = {"siedzenie", "leżenie", "stanie"};

  ESP_LOGI("RESULT", "Pred: %s (%.4f) si: %.4f le: %.4f st: %.4f t: %.3fms",
           class_names[int(i)], y.operator[](int(i)), y.operator[](int(0)),
           y.operator[](int(1)), y.operator[](int(2)), elapsed / 1000.0);
}
} // namespace forest
