#include "cnn_model.cc"
#include "cnn_normalization.cc"
#include "esp_log.h"
#include "math.h"
#include "secrets.h"
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/micro/micro_mutable_op_resolver.h"
#include "tensorflow/lite/micro/tflite_bridge/micro_error_reporter.h"
#include "tensorflow/lite/schema/schema_generated.h"
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vector>
#include "esp_heap_caps.h"
#include <esp_system.h>
#include <esp_timer.h>
#include "features.h"
#include "csi_command.h"
#include <vector>

namespace cnn {
#define TENSOR_ARENA_SIZE (200000)
    static bool model_loaded = false;

    static tflite::MicroInterpreter *interpreter = nullptr;
    static uint8_t *tensor_arena = nullptr;

    void init_tensor_arena() {
        if (!tensor_arena) {
            tensor_arena = (uint8_t *) heap_caps_malloc(TENSOR_ARENA_SIZE, MALLOC_CAP_SPIRAM);
            if (!tensor_arena) {
                ESP_LOGE("TFLM", "Failed to allocate tensor arena (%d bytes) in PSRAM", TENSOR_ARENA_SIZE);
            } else {
                ESP_LOGI("TFLM", "Tensor arena allocated in PSRAM at %p", tensor_arena);
            }
        }
    }

    void print_compact_info() {
        size_t total_ram = heap_caps_get_total_size(MALLOC_CAP_INTERNAL);
        size_t free_ram = heap_caps_get_free_size(MALLOC_CAP_INTERNAL);
        float used_ram_percent = 100.0f * (total_ram - free_ram) / total_ram;

        size_t total_psram = heap_caps_get_total_size(MALLOC_CAP_SPIRAM);
        size_t free_psram = heap_caps_get_free_size(MALLOC_CAP_SPIRAM);
        float used_psram_percent = total_psram > 0 ? 100.0f * (total_psram - free_psram) / total_psram : 0.0f;

        printf("=== System memory ===\n");
        printf("RAM:   %u / %u bytes (%.2f%% used)\n", (unsigned) (total_ram - free_ram), (unsigned) total_ram,
               used_ram_percent);
        if (total_psram > 0)
            printf("PSRAM: %u / %u bytes (%.2f%% used)\n", (unsigned) (total_psram - free_psram),
                   (unsigned) total_psram, used_psram_percent);
        else
            printf("PSRAM: not accessible\n");
    }

    void convolutional_neural_networks(int8_t *values) {
        if (!model_loaded) {
            int64_t start = esp_timer_get_time();
            init_tensor_arena();
            static tflite::MicroErrorReporter error_reporter;
            const tflite::Model *model = tflite::GetModel(csi_model_cnn_tflite);

            if (model->version() != TFLITE_SCHEMA_VERSION) {
                ESP_LOGE("MODEL", "Model schema version mismatch!");
                return;
            }

            static tflite::MicroMutableOpResolver<9> resolver;
            resolver.AddExpandDims();
            resolver.AddConv2D();
            resolver.AddMul();
            resolver.AddAdd();
            resolver.AddReshape();
            resolver.AddMaxPool2D();
            resolver.AddMean();
            resolver.AddFullyConnected();
            resolver.AddSoftmax();
            static tflite::MicroInterpreter static_interpreter(
                model, resolver, tensor_arena, TENSOR_ARENA_SIZE);
            interpreter = &static_interpreter;
            TfLiteStatus allocate_status = interpreter->AllocateTensors();

            if (allocate_status != kTfLiteOk) {
                ESP_LOGE("MODEL", "AllocateTensors() failed");
                return;
            }
            model_loaded = true;
            int64_t end = esp_timer_get_time();
            int64_t elapsed = end - start;
            printf("Model load took: %lld microseconds (%.3f ms)\n", elapsed, elapsed / 1000.0);
        }
        int64_t start = esp_timer_get_time();
        TfLiteTensor *input = interpreter->input(0);
        std::vector<float> normalized;
        normalized.reserve(112);

        for (int i = 0; i < 112; i++) {
            normalized.emplace_back((values[i] - cnn_means[i]) / cnn_scales[i]);

        }

        features::preprocess_data_cnn(normalized);
        for (int i = 0; i < 112; i++) {
            input->data.f[i] = normalized[i];
        }

        if (interpreter->Invoke() != kTfLiteOk) {
            ESP_LOGE("MODEL", "Invoke failed");
            return;
        }
        TfLiteTensor *output = interpreter->output(0);
        float prob_desk = output->data.f[0];
        float prob_bed = output->data.f[1];
        float prob_stay = output->data.f[2];



        if (csi_command::benchmark)
            print_compact_info();
        int predicted_class = 0;
        float max_prob = prob_desk;

        if (prob_bed > max_prob) {
            predicted_class = 1;
            max_prob = prob_bed;
        }
        if (prob_stay > max_prob) {
            predicted_class = 2;
            max_prob = prob_stay;
        }
        int64_t elapsed = esp_timer_get_time() - start;

        const char *class_names[] = {"Desk", "Bed", "Stay"};
        ESP_LOGI("RESULT", "Pred: %s (%.4f) de: %.4f be: %.4f st: %.4f t: %.3f ms",
                 class_names[predicted_class], max_prob, prob_desk, prob_bed, prob_stay, elapsed / 1000.0);

    }
} // namespace cnn
