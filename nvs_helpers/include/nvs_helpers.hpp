/**
 * @file nvs_helpers.hpp
 * @brief Type-safe RAII wrapper for ESP-IDF NVS (Non-Volatile Storage)
 *
 * Eliminates boilerplate nvs_open/commit/close patterns with compile-time
 * type safety for primitive types and runtime safety for blobs.
 *
 * Example usage:
 * @code
 * NvsStore config("led_cfg");
 * uint16_t count = 100;
 * config.save("led_cnt_1", count);  // Type-safe, no manual handle management
 *
 * uint16_t loaded;
 * if (config.load("led_cnt_1", loaded) == ESP_OK) {
 *     // Use loaded value
 * }
 * @endcode
 */

#pragma once

#include "esp_err.h"
#include "nvs.h"
#include <cstddef>
#include <cstdint>
#include <type_traits>

/**
 * @brief Type-safe NVS storage wrapper with RAII handle management
 *
 * Automatically handles nvs_open/commit/close lifecycle on a per-operation basis.
 * Supports all ESP-IDF integer types with compile-time type checking.
 */
class NvsStore {
public:
    /**
     * @brief Construct NVS store for given namespace
     * @param namespace_name NVS namespace identifier (max 15 chars)
     *
     * Validates namespace accessibility. Does not keep handle open.
     */
    explicit NvsStore(const char* namespace_name);

    /**
     * @brief Destructor (trivial - handles closed per operation)
     */
    ~NvsStore() = default;

    // Disable copy/move to prevent namespace name lifetime issues
    NvsStore(const NvsStore&) = delete;
    NvsStore& operator=(const NvsStore&) = delete;

    /**
     * @brief Save value to NVS with type safety
     * @tparam T Integer type (uint8_t, uint16_t, uint32_t, uint64_t, int8_t, int16_t, int32_t, int64_t)
     * @param key NVS key name
     * @param value Value to save
     * @return ESP_OK on success, error code otherwise
     *
     * Opens handle, sets value, commits, and closes in single operation.
     */
    template<typename T>
    esp_err_t save(const char* key, const T& value);

    /**
     * @brief Load value from NVS with type safety
     * @tparam T Integer type (must match type used in save)
     * @param key NVS key name
     * @param value Output parameter for loaded value
     * @return ESP_OK on success, ESP_ERR_NVS_NOT_FOUND if key missing, error code otherwise
     *
     * Opens handle, gets value, closes handle.
     */
    template<typename T>
    esp_err_t load(const char* key, T& value);

    /**
     * @brief Save arbitrary binary blob (e.g., structs)
     * @param key NVS key name
     * @param data Pointer to data
     * @param len Length of data in bytes
     * @return ESP_OK on success, error code otherwise
     *
     * Use for non-primitive types. Caller responsible for serialization.
     */
    esp_err_t save_blob(const char* key, const void* data, size_t len);

    /**
     * @brief Load arbitrary binary blob
     * @param key NVS key name
     * @param data Output buffer
     * @param len Input: buffer size, Output: actual data size
     * @return ESP_OK on success, ESP_ERR_NVS_NOT_FOUND if key missing
     *
     * Caller must allocate buffer. If len too small, returns ESP_ERR_NVS_INVALID_LENGTH.
     */
    esp_err_t load_blob(const char* key, void* data, size_t* len);

    /**
     * @brief Check if key exists in namespace
     * @param key NVS key name
     * @return true if key exists, false otherwise
     */
    bool exists(const char* key);

    /**
     * @brief Erase key from namespace
     * @param key NVS key name
     * @return ESP_OK on success, ESP_ERR_NVS_NOT_FOUND if key missing
     */
    esp_err_t erase(const char* key);

private:
    const char* m_namespace;  ///< NVS namespace name (must outlive this object)

    /**
     * @brief Open NVS handle for read or write
     * @param mode NVS_READONLY or NVS_READWRITE
     * @param handle Output handle
     * @return ESP_OK on success, error code otherwise
     */
    esp_err_t open_handle(nvs_open_mode_t mode, nvs_handle_t* handle) const;
};

// Template implementation (must be in header for templates)

template<typename T>
esp_err_t NvsStore::save(const char* key, const T& value) {
    nvs_handle_t handle;
    esp_err_t err = open_handle(NVS_READWRITE, &handle);
    if (err != ESP_OK) return err;

    // Type-specific save via template specialization (see .cpp for explicit instantiations)
    if constexpr (std::is_same_v<T, uint8_t>) {
        err = nvs_set_u8(handle, key, value);
    } else if constexpr (std::is_same_v<T, uint16_t>) {
        err = nvs_set_u16(handle, key, value);
    } else if constexpr (std::is_same_v<T, uint32_t>) {
        err = nvs_set_u32(handle, key, value);
    } else if constexpr (std::is_same_v<T, uint64_t>) {
        err = nvs_set_u64(handle, key, value);
    } else if constexpr (std::is_same_v<T, int8_t>) {
        err = nvs_set_i8(handle, key, value);
    } else if constexpr (std::is_same_v<T, int16_t>) {
        err = nvs_set_i16(handle, key, value);
    } else if constexpr (std::is_same_v<T, int32_t>) {
        err = nvs_set_i32(handle, key, value);
    } else if constexpr (std::is_same_v<T, int64_t>) {
        err = nvs_set_i64(handle, key, value);
    } else {
        static_assert(std::is_same_v<T, uint8_t>,
            "NvsStore::save only supports integer types (u8/16/32/64, i8/16/32/64)");
    }

    if (err == ESP_OK) {
        err = nvs_commit(handle);
    }

    nvs_close(handle);
    return err;
}

template<typename T>
esp_err_t NvsStore::load(const char* key, T& value) {
    nvs_handle_t handle;
    esp_err_t err = open_handle(NVS_READONLY, &handle);
    if (err != ESP_OK) return err;

    // Type-specific load via template specialization
    if constexpr (std::is_same_v<T, uint8_t>) {
        err = nvs_get_u8(handle, key, &value);
    } else if constexpr (std::is_same_v<T, uint16_t>) {
        err = nvs_get_u16(handle, key, &value);
    } else if constexpr (std::is_same_v<T, uint32_t>) {
        err = nvs_get_u32(handle, key, &value);
    } else if constexpr (std::is_same_v<T, uint64_t>) {
        err = nvs_get_u64(handle, key, &value);
    } else if constexpr (std::is_same_v<T, int8_t>) {
        err = nvs_get_i8(handle, key, &value);
    } else if constexpr (std::is_same_v<T, int16_t>) {
        err = nvs_get_i16(handle, key, &value);
    } else if constexpr (std::is_same_v<T, int32_t>) {
        err = nvs_get_i32(handle, key, &value);
    } else if constexpr (std::is_same_v<T, int64_t>) {
        err = nvs_get_i64(handle, key, &value);
    } else {
        static_assert(std::is_same_v<T, uint8_t>,
            "NvsStore::load only supports integer types (u8/16/32/64, i8/16/32/64)");
    }

    nvs_close(handle);
    return err;
}
