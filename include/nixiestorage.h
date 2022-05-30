#pragma once

#include <memory>

#include "esp_log.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "nvs_handle.hpp"

/**
 * @brief 存储管理
 * USAGE: NixieStorage::instance().open("namespace")->nvsHandler->setItem(...);
 *                                                              ->getItem(...);
 * 
 */

class NixieStorage final
{
public:
    static NixieStorage& instance() {
        static NixieStorage instance;
        return instance;
    }
    NixieStorage(const NixieStorage&) = delete;
    NixieStorage& operator= (const NixieStorage) = delete;
private:
    NixieStorage();
public:
    NixieStorage& open(const char * ns);
public:
    std::unique_ptr<nvs::NVSHandle> nvsHandler;
private:
    const char *TAG = "NixieStorage";
    esp_err_t err;
};