#ifndef ZIGBEE_BUTTON_HPP
#define ZIGBEE_BUTTON_HPP

#include <stdint.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

/**
 * @brief Button handler with hold-time detection and callback-based reset actions.
 *
 * Polls GPIO at 100ms intervals to detect button press and hold duration.
 * Invokes callbacks based on hold time thresholds (network reset vs full factory reset).
 * Provides optional LED feedback during hold (amber/red alternating blink).
 *
 * RAII design: Task automatically starts on construction, stops on destruction.
 */
class ButtonHandler {
public:
    using Callback = void(*)();  // Function pointer for reset callbacks

    /**
     * @brief Construct button handler and configure GPIO.
     *
     * @param gpio GPIO pin number (configured as input with pull-up)
     * @param network_reset_ms Hold time threshold for network reset (e.g., 3000ms)
     * @param full_reset_ms Hold time threshold for full factory reset (e.g., 10000ms)
     */
    ButtonHandler(uint8_t gpio, uint32_t network_reset_ms, uint32_t full_reset_ms);

    /**
     * @brief Destructor - stops task if running.
     */
    ~ButtonHandler();

    /**
     * @brief Start the button polling task.
     *
     * Safe to call multiple times (checks if task already running).
     * Task runs at priority 5 with 2KB stack.
     */
    void start();

    /**
     * @brief Stop the button polling task.
     *
     * Safe to call multiple times (checks if task exists before deleting).
     */
    void stop();

    /**
     * @brief Set callback for network reset action (triggered at network_reset_ms threshold).
     *
     * @param cb Function pointer to call when button held for network_reset_ms (e.g., 3s)
     */
    void set_network_reset_callback(Callback cb);

    /**
     * @brief Set callback for full factory reset action (triggered at full_reset_ms threshold).
     *
     * @param cb Function pointer to call when button held for full_reset_ms (e.g., 10s)
     */
    void set_full_reset_callback(Callback cb);

    /**
     * @brief Set callback for LED feedback during button hold.
     *
     * @param cb Function pointer to call with state codes:
     *           0 = off (restore previous state)
     *           1 = amber/not-joined state (1s-3s hold)
     *           2 = red/error state (3s+ hold)
     *
     * Called periodically during hold to provide visual feedback.
     * If nullptr, no LED feedback is provided.
     */
    void set_led_callback(void(*cb)(int state));

private:
    uint8_t m_gpio;
    uint32_t m_network_reset_ms;
    uint32_t m_full_reset_ms;
    TaskHandle_t m_task_handle;
    Callback m_network_reset_cb;
    Callback m_full_reset_cb;
    void(*m_led_cb)(int state);

    /**
     * @brief Static wrapper for FreeRTOS task creation.
     *
     * Casts arg back to ButtonHandler* and calls run().
     */
    static void task_func(void* arg);

    /**
     * @brief Main polling loop (runs in FreeRTOS task context).
     *
     * Polls GPIO every 100ms, detects hold time, triggers callbacks on release.
     */
    void run();
};

#endif // ZIGBEE_BUTTON_HPP
