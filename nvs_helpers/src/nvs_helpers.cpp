/**
 * @file nvs_helpers.cpp
 * @brief Implementation of type-safe NVS wrapper
 */

#include "nvs_helpers.hpp"
#include "esp_log.h"
#include "nvs_flash.h"
#include <cstring>

static const char* TAG = "NvsStore";

NvsStore::NvsStore(const char* namespace_name)
    : m_namespace(namespace_name)
{
    // Validate namespace is accessible by attempting to open/close
    nvs_handle_t handle;
    esp_err_t err = nvs_open(m_namespace, NVS_READWRITE, &handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to validate namespace '%s': %s",
                 m_namespace, esp_err_to_name(err));
        // Constructor cannot return error, but log it
        // Subsequent operations will fail gracefully
    } else {
        nvs_close(handle);
        ESP_LOGD(TAG, "NVS namespace '%s' validated", m_namespace);
    }
}

esp_err_t NvsStore::open_handle(nvs_open_mode_t mode, nvs_handle_t* handle) const
{
    esp_err_t err = nvs_open(m_namespace, mode, handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "nvs_open failed for namespace '%s': %s",
                 m_namespace, esp_err_to_name(err));
    }
    return err;
}

esp_err_t NvsStore::save_blob(const char* key, const void* data, size_t len)
{
    if (!data || len == 0) {
        return ESP_ERR_INVALID_ARG;
    }

    nvs_handle_t handle;
    esp_err_t err = open_handle(NVS_READWRITE, &handle);
    if (err != ESP_OK) return err;

    err = nvs_set_blob(handle, key, data, len);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "nvs_set_blob failed for key '%s': %s",
                 key, esp_err_to_name(err));
    }

    if (err == ESP_OK) {
        err = nvs_commit(handle);
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "nvs_commit failed for key '%s': %s",
                     key, esp_err_to_name(err));
        }
    }

    nvs_close(handle);
    return err;
}

esp_err_t NvsStore::load_blob(const char* key, void* data, size_t* len)
{
    if (!data || !len) {
        return ESP_ERR_INVALID_ARG;
    }

    nvs_handle_t handle;
    esp_err_t err = open_handle(NVS_READONLY, &handle);
    if (err != ESP_OK) return err;

    // First get required size
    size_t required_size = 0;
    err = nvs_get_blob(handle, key, nullptr, &required_size);
    if (err != ESP_OK) {
        if (err != ESP_ERR_NVS_NOT_FOUND) {
            ESP_LOGE(TAG, "nvs_get_blob size query failed for key '%s': %s",
                     key, esp_err_to_name(err));
        }
        nvs_close(handle);
        return err;
    }

    // Check if provided buffer is large enough
    if (*len < required_size) {
        ESP_LOGE(TAG, "Buffer too small for key '%s': need %zu, have %zu",
                 key, required_size, *len);
        *len = required_size;  // Return required size
        nvs_close(handle);
        return ESP_ERR_NVS_INVALID_LENGTH;
    }

    // Read actual data
    err = nvs_get_blob(handle, key, data, len);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "nvs_get_blob failed for key '%s': %s",
                 key, esp_err_to_name(err));
    }

    nvs_close(handle);
    return err;
}

bool NvsStore::exists(const char* key)
{
    nvs_handle_t handle;
    esp_err_t err = open_handle(NVS_READONLY, &handle);
    if (err != ESP_OK) return false;

    // Use nvs_get_blob with NULL data pointer to check existence
    size_t len = 0;
    err = nvs_get_blob(handle, key, nullptr, &len);

    // Key exists if we get ESP_OK (blob) or if any typed get succeeds
    // Try blob first, then fallback to checking if any primitive exists
    if (err == ESP_OK) {
        nvs_close(handle);
        return true;
    }

    // For primitive types, try a representative check (u8)
    // If key exists as any type, at least one will succeed
    uint8_t dummy;
    err = nvs_get_u8(handle, key, &dummy);
    if (err == ESP_OK) {
        nvs_close(handle);
        return true;
    }

    // Try other common types
    uint16_t dummy16;
    err = nvs_get_u16(handle, key, &dummy16);
    if (err == ESP_OK) {
        nvs_close(handle);
        return true;
    }

    uint32_t dummy32;
    err = nvs_get_u32(handle, key, &dummy32);
    if (err == ESP_OK) {
        nvs_close(handle);
        return true;
    }

    nvs_close(handle);
    return false;
}

esp_err_t NvsStore::erase(const char* key)
{
    nvs_handle_t handle;
    esp_err_t err = open_handle(NVS_READWRITE, &handle);
    if (err != ESP_OK) return err;

    err = nvs_erase_key(handle, key);
    if (err != ESP_OK && err != ESP_ERR_NVS_NOT_FOUND) {
        ESP_LOGE(TAG, "nvs_erase_key failed for key '%s': %s",
                 key, esp_err_to_name(err));
    }

    if (err == ESP_OK) {
        esp_err_t commit_err = nvs_commit(handle);
        if (commit_err != ESP_OK) {
            ESP_LOGE(TAG, "nvs_commit failed after erase for key '%s': %s",
                     key, esp_err_to_name(commit_err));
            err = commit_err;
        }
    }

    nvs_close(handle);
    return err;
}
