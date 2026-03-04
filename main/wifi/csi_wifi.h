#include "esp_wifi.h"
#include "esp_wifi_types_generic.h"
#include <cstdint>
#include <esp_event_base.h>

namespace csi_wifi {
    extern bool wifi_connected;

    extern QueueHandle_t xCsiQueue;

    void wifi_csi_cb(void *ctx, wifi_csi_info_t *csi_data);

    void wifi_event_handler(void *arg, esp_event_base_t event_base,
                            int32_t event_id, void *event_data);

    void ip_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id,
                          void *event_data);

    void wifi_init_sta(void);

    void csi_init(void);

    esp_err_t wifi_ping_router_start();

    esp_err_t wifi_ping_router_stop();
} // namespace csi_wifi
