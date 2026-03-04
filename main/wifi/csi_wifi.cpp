#include "esp_sntp.h"
#include "esp_wifi.h"

#include "csi_wifi.h"
#include "esp_log.h"
#include "esp_wifi_types_generic.h"
#include "secrets.h"
#include <cJSON.h>
#include <cstring>
#include <ping/ping_sock.h>
namespace csi_wifi
{
  static const char *TAG = "CSI";
  const uint8_t PING_PER_SECOND = 60;
  bool wifi_connected = false;
  static bool wifi_initialized = false;
  static esp_ping_handle_t ping_handle = NULL;
  QueueHandle_t xCsiQueue;
   esp_err_t wifi_ping_router_stop() {
    esp_ping_stop(ping_handle);
    return ESP_OK;
  }
   esp_err_t wifi_ping_router_start() {
    esp_ping_start(ping_handle);
    return ESP_OK;
  }

  static esp_err_t wifi_ping_router_setup()
  {
    esp_ping_config_t ping_config = ESP_PING_DEFAULT_CONFIG();
    ping_config.count = 0;
    ping_config.interval_ms = 1000 / PING_PER_SECOND;
    ping_config.task_stack_size = 3072;
    ping_config.data_size = 1;

    esp_netif_ip_info_t local_ip;
    esp_netif_get_ip_info(esp_netif_get_handle_from_ifkey("WIFI_STA_DEF"), &local_ip);
    ping_config.target_addr.u_addr.ip4.addr = ip4_addr_get_u32(&local_ip.gw);
    ping_config.target_addr.type = ESP_IPADDR_TYPE_V4;

    esp_ping_callbacks_t cbs = {0};
    esp_ping_new_session(&ping_config, &cbs, &ping_handle);
    return ESP_OK;
  }
  void print_csi_config(wifi_csi_info_t *csi_data)
  {
    printf("stbc: %d\n", csi_data->rx_ctrl.stbc);
    printf("channel_mode: %d\n", csi_data->rx_ctrl.cwb);
    printf("signal_mode: %d\n", csi_data->rx_ctrl.sig_mode);
    printf("signal_len: %d\n", csi_data->len);
  }
  void sliceAndConcat(const int8_t *values, int8_t *out)
  {

    // najpierw ht2 = values[200:256]
    memcpy(out, values + 200, 56);
    // potem ht1 = values[130:186]
    memcpy(out + 56, values + 130, 56);
  }
  void wifi_csi_cb(void *ctx, wifi_csi_info_t *csi_data)
  {
    if (!csi_data || !csi_data->buf)
    {
      return;
    }
    int8_t sliced[112];
    sliceAndConcat(csi_data->buf, sliced);

    xQueueSend(xCsiQueue, &sliced, 0);
  }
  // WiFi event handler
  void wifi_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
  {
    if (event_id == WIFI_EVENT_STA_START)
    {
      ESP_LOGI(TAG, "WiFi started, connecting...");
      esp_wifi_connect();
    }
    else if (event_id == WIFI_EVENT_STA_CONNECTED)
    {
      ESP_LOGI(TAG, "Connected to WiFi network");
      wifi_connected = true;
    }
    else if (event_id == WIFI_EVENT_STA_DISCONNECTED)
    {
      wifi_initialized = false;
      wifi_connected = false;
    }
  }

  // IP event handler
  void ip_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id,
                        void *event_data)
  {
    if (event_id == IP_EVENT_STA_GOT_IP)
    {
      ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
      ESP_LOGI(TAG, "Got IP: " IPSTR, IP2STR(&event->ip_info.ip));

      csi_init();
      wifi_ping_router_setup();
      wifi_ping_router_start();
    }
  }

  // Initialize WiFi in STA mode
  void wifi_init_sta(void)
  {
    if (!wifi_initialized)
    {

      ESP_ERROR_CHECK(esp_netif_init());
      esp_err_t err = esp_event_loop_create_default();
      if (err != ESP_OK && err != ESP_ERR_INVALID_STATE)
      {
        ESP_ERROR_CHECK(err);
      }
      static esp_netif_t *sta_netif = nullptr;
      if (sta_netif == nullptr)
      {
        sta_netif = esp_netif_create_default_wifi_sta();
      }
      wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
      ESP_ERROR_CHECK(esp_wifi_init(&cfg));
      ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID,
                                                 &wifi_event_handler, NULL));
      ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP,
                                                 &ip_event_handler, NULL));

      // Configure WiFi
      // WiFi configuration - secrets.h.template -> main/secrets.h
      wifi_config_t wifi_config = {
          .sta =
              {
                  .ssid = WIFI_SSID,
                  .password = WIFI_PASSWORD,
                  .pmf_cfg = {.capable = true, .required = false},

              },
      };

      ESP_LOGI(TAG, "Connecting to SSID: %s", wifi_config.sta.ssid);

      ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
      ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
      esp_wifi_set_bandwidth(WIFI_IF_STA, WIFI_BW20);
      ESP_ERROR_CHECK(esp_wifi_start());
      wifi_initialized = true;
    }
  }

  // Configure CSI
  void csi_init(void)
  {
    // HT, 20MHz, stbc (if tx is using it)
    wifi_csi_config_t csi_config = {
        .lltf_en = true,
        .htltf_en = true,
        .stbc_htltf2_en = false,
    };

    ESP_ERROR_CHECK(esp_wifi_set_csi_config(&csi_config));
    ESP_ERROR_CHECK(esp_wifi_set_csi_rx_cb(wifi_csi_cb, NULL));
    ESP_ERROR_CHECK(esp_wifi_set_csi(true));
    ESP_LOGI(TAG, "CSI initialized and enabled");
    ESP_LOGI(TAG, "CSI capture started");
  }
}
