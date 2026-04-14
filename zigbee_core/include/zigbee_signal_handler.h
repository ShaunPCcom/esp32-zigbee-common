/**
 * @file zigbee_signal_handler.h
 * @brief Shared Zigbee network lifecycle signal handler.
 *
 * Provides the esp_zb_app_signal_handler() implementation and all common
 * network lifecycle behaviour (steering retry, Device_annce on C6 reboot,
 * factory reset). Projects register callbacks for device-specific actions.
 *
 * Usage:
 *   1. Implement the callbacks you need (on_joined is required).
 *   2. Call zigbee_signal_handler_register() before the Zigbee stack starts.
 *   3. Remove any local definitions of esp_zb_app_signal_handler,
 *      zigbee_factory_reset, zigbee_full_factory_reset, and reboot_cb.
 */

#pragma once

#include <stdbool.h>
#include "esp_zigbee_core.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Project-specific callbacks for Zigbee network events.
 *
 * All fields are optional except on_joined and nvs_namespace.
 */
typedef struct {
    /**
     * Called at SKIP_STARTUP after BDB steering has been started.
     * Use for any stack-init-time setup that must run inside the Zigbee task
     * (e.g. sync ZCL attribute values from NVS, start renderer tasks).
     * Optional — pass NULL to skip.
     */
    void (*on_stack_init)(void);

    /**
     * Called when the device successfully joins or rejoins the network.
     * Fires on both fresh STEERING success and DEVICE_REBOOT while already
     * commissioned. Use for post-join state sync, reporting config, etc.
     * Required.
     */
    void (*on_joined)(void);

    /**
     * Called when the device leaves the network (ZDO SIGNAL_LEAVE).
     * Use for UI feedback, clearing output state, etc.
     * Optional — pass NULL to skip.
     */
    void (*on_left)(void);

    /**
     * Called from the default switch case for any signal not handled by the
     * common handler. Useful for project-specific signals (e.g.
     * ESP_ZB_NLME_STATUS_INDICATION on LD2450).
     * Optional — pass NULL to skip.
     */
    void (*on_unhandled_signal)(esp_zb_app_signal_t *signal_struct);

    /**
     * NVS namespace to erase during zigbee_full_factory_reset().
     * Example: "led_cfg" or "ld2450_cfg".
     * Required (must not be NULL).
     */
    const char *nvs_namespace;
} zigbee_signal_hooks_t;

/**
 * @brief Register project-specific lifecycle hooks.
 *
 * Must be called before the Zigbee stack processes its first signal —
 * typically at the start of zigbee_init().
 *
 * @param hooks  Hook table. The pointer must remain valid for the lifetime
 *               of the application (use a static struct).
 */
void zigbee_signal_handler_register(const zigbee_signal_hooks_t *hooks);

/**
 * @brief Returns true if the device is currently joined to a Zigbee network.
 */
bool zigbee_is_network_joined(void);

/**
 * @brief Network-only reset — leaves network but keeps NVS config.
 *
 * Calls esp_zb_factory_reset() and restarts the device.
 */
void zigbee_factory_reset(void);

/**
 * @brief Full factory reset — erases Zigbee network state AND NVS config.
 *
 * Erases the project's NVS namespace (registered via hooks->nvs_namespace),
 * then calls esp_zb_factory_reset() and restarts the device.
 */
void zigbee_full_factory_reset(void);

/**
 * @brief Scheduler-alarm callback that calls esp_restart().
 *
 * Exposed so attribute handlers can schedule a deferred reboot via
 * esp_zb_scheduler_alarm(reboot_cb, 0, delay_ms).
 */
void reboot_cb(uint8_t param);

/**
 * @brief Current network join state.
 *
 * Exposed for callers that need direct boolean access. Prefer
 * zigbee_is_network_joined() for new code.
 */
extern bool s_network_joined;

#ifdef __cplusplus
}
#endif
