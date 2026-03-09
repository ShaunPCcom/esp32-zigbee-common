/**
 * @file zigbee_ctrl.c
 * @brief Shared control cluster attribute handlers (restart and factory reset).
 */

#include "zigbee_ctrl.h"
#include "esp_system.h"
#include "esp_zigbee_core.h"
#include "esp_log.h"

static const char *TAG = "zigbee_ctrl";

static void restart_cb(uint8_t param)
{
    (void)param;
    esp_restart();
}

void zgb_ctrl_handle_restart(void)
{
    ESP_LOGI(TAG, "Restart requested via Zigbee, restarting in 1s...");
    /* Delay so the ZCL Write Attributes Response is sent before reset.
     * Without this delay Z2M may retry the write after reconnect -> double reboot. */
    esp_zb_scheduler_alarm(restart_cb, 0, 1000);
}

void zgb_ctrl_handle_factory_reset(uint8_t value, void (*project_reset_fn)(void))
{
    if (value != ZB_FACTORY_RESET_MAGIC) {
        ESP_LOGW(TAG, "Factory reset ignored: expected 0x%02X, got 0x%02X",
                 ZB_FACTORY_RESET_MAGIC, value);
        return;
    }
    if (project_reset_fn == NULL) {
        ESP_LOGE(TAG, "Factory reset: project_reset_fn is NULL, aborting");
        return;
    }
    ESP_LOGW(TAG, "Factory reset triggered via Zigbee (magic=0xFE)");
    project_reset_fn();
}
