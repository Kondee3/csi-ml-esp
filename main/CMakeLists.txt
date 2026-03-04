set(CMAKE_CXX_STANDARD 23) 
idf_component_register(
  SRC_DIRS "." "ml" "ml/cnn" "ml/nn" "ml/features" "ml/forest" "storage" "wifi" "commands"
  INCLUDE_DIRS "." "ml"  "ml/cnn" "ml/nn" "ml/features" "ml/forest" "storage" "wifi" "commands"
  PRIV_REQUIRES spi_flash esp_wifi nvs_flash esp_event freertos esp_netif console fatfs app_update
  REQUIRES alglib cjson esp_timer
)

littlefs_create_partition_image(littlefs ../flash_data FLASH_IN_PROJECT)

target_compile_options(${COMPONENT_LIB} PRIVATE 
    -Wno-missing-field-initializers
    -Wno-unused-function 
    -Wno-shadow
)
