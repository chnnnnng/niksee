#include "nixiestorage.h"

NixieStorage::NixieStorage()
{
    // Initialize NVS
    err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        // NVS partition was truncated and needs to be erased
        // Retry nvs_flash_init
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK( err );
}

NixieStorage& NixieStorage::open(const char * ns)
{ 
    nvsHandler = nvs::open_nvs_handle(ns, NVS_READWRITE, &err);
    if (err != ESP_OK) {
        ESP_LOGI(TAG,"Error (%s) opening NVS handle!\n", esp_err_to_name(err));
    } else {
        ESP_LOGI(TAG,"Succeed opening NVS namespace %s.",ns);
    }
    return *this;
}
