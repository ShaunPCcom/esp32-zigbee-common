/**
 * @file zigbee_ctrl.h
 * @brief Shared control cluster (0xFC00) attribute IDs and write handlers.
 *
 * Both LED and LD2450 projects expose cluster 0xFC00 with a restart attribute
 * (0x00F0) and factory reset attribute (0x00F1). This module provides:
 *   - Shared attribute ID constants
 *   - Handler functions for project's zigbee_attr_handler.c to call
 *
 * Usage in zigbee_attr_handler.c:
 *   #include "zigbee_ctrl.h"
 *   // inside cluster 0xFC00 handler:
 *   case ZB_ATTR_RESTART:
 *       zgb_ctrl_handle_restart();
 *       return ESP_OK;
 *   case ZB_ATTR_FACTORY_RESET:
 *       zgb_ctrl_handle_factory_reset(*(uint8_t *)value, zigbee_full_factory_reset);
 *       return ESP_OK;
 */

#ifndef ZIGBEE_CTRL_H
#define ZIGBEE_CTRL_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/* Attribute IDs in custom cluster 0xFC00 */
#define ZB_ATTR_RESTART          0x00F0  /* U8, write-only — any value triggers 1s-delayed restart */
#define ZB_ATTR_FACTORY_RESET    0x00F1  /* U8, write-only — 0xFE triggers factory reset */
#define ZB_FACTORY_RESET_MAGIC   0xFE    /* Magic value required to trigger factory reset */

/**
 * @brief Schedule a soft restart in 1 second.
 *
 * Uses esp_zb_scheduler_alarm so the ZCL Write Attributes Response is
 * transmitted before the device reboots. Safe to call from any Zigbee
 * attribute write callback.
 */
void zgb_ctrl_handle_restart(void);

/**
 * @brief Trigger factory reset if value matches magic byte.
 *
 * @param value        Value written to attribute 0x00F1.
 * @param project_reset_fn  Project-specific reset function (e.g. zigbee_full_factory_reset).
 *                     Called only when value == ZB_FACTORY_RESET_MAGIC (0xFE).
 *                     This function is responsible for erasing NVS and calling
 *                     esp_zb_factory_reset() or equivalent.
 *
 * Any value other than 0xFE is silently ignored.
 */
void zgb_ctrl_handle_factory_reset(uint8_t value, void (*project_reset_fn)(void));

#ifdef __cplusplus
}
#endif

#endif /* ZIGBEE_CTRL_H */
