// SPDX-License-Identifier: MIT
#pragma once

#include <stdbool.h>
#include <stdint.h>

/**
 * OTA update availability checker (ESP32-C6 only).
 *
 * Fetches the OTA index on a configurable interval and on demand.
 * Results are read by the web server to drive the update UI.
 */

typedef struct {
    uint16_t    image_type;      /**< OTA image type for this device variant, e.g. 0x0003 */
    uint32_t    current_version; /**< Running firmware version hex, e.g. FIRMWARE_VERSION */
    const char *nvs_namespace;   /**< NVS namespace for persisting interval and URL */
} ota_check_config_t;

/**
 * Initialise the check module: create background task and start periodic
 * timer. The first check runs 15 seconds after init (Wi-Fi settling time).
 * Call once from web_server_start() after Wi-Fi connects.
 * @param cfg  Device-specific config (image type and NVS namespace). Must remain
 *             valid for the lifetime of the application.
 */
void ota_check_init(const ota_check_config_t *cfg);

/**
 * Trigger an immediate check synchronously (blocking ~2-3 s).
 * Called from the HTTP handler task — safe to block there.
 */
void ota_check_trigger(void);

/** True if the latest index version is newer than the running firmware. */
bool ota_check_available(void);

/**
 * Version string of the latest available firmware, e.g. "2.2.4".
 * Empty string if no update is available or no check has completed.
 * Pointer remains valid until the next call to ota_check_trigger().
 */
const char *ota_check_latest_version(void);

/** Persisted check interval in hours (1-168). Default: 12. */
uint16_t ota_check_get_interval_hours(void);

/** Update and persist the check interval, then restart the timer. */
void ota_check_set_interval_hours(uint16_t hours);

/**
 * OTA index URL used for both background checks and Wi-Fi OTA transport.
 * Returns the persisted override if set, otherwise the built-in default URL.
 * Pointer is stable until ota_check_set_index_url() is called.
 */
const char *ota_check_get_index_url(void);

/**
 * Persist a custom OTA index URL and notify the OTA component.
 * Pass NULL or empty string to clear the override and revert to the default.
 */
void ota_check_set_index_url(const char *url);
