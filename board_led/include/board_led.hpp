/**
 * @file board_led.hpp
 * @brief Status indication via onboard WS2812 LED using RAII C++ class
 *
 * This component provides visual status feedback for ESP32 Zigbee devices via an
 * onboard WS2812B RGB LED. Uses RMT TX peripheral for precise timing control.
 *
 * Features:
 * - RAII resource management (constructor initializes, destructor cleans up)
 * - WS2812B protocol via ESP-IDF 5.x RMT TX API
 * - Five states: OFF, NOT_JOINED (amber blink), PAIRING (blue blink),
 *   JOINED (green solid 5s), ERROR (red blink 5s)
 * - Non-blocking operation using esp_timer
 */

#ifndef BOARD_LED_HPP
#define BOARD_LED_HPP

#include <stdint.h>
#include "driver/rmt_tx.h"
#include "esp_timer.h"

/**
 * @brief Board LED controller class with RAII resource management
 *
 * Manages onboard WS2812B LED for Zigbee device status indication.
 * Resources (RMT channel, timers) are automatically cleaned up on destruction.
 */
class BoardLed {
public:
    /**
     * @brief LED status states
     */
    enum class State {
        OFF,         ///< LED off
        NOT_JOINED,  ///< Amber blink ~2 Hz (not connected to Zigbee network)
        PAIRING,     ///< Blue blink ~2 Hz (pairing mode active)
        JOINED,      ///< Green solid 5s then OFF (successfully joined network)
        ERROR        ///< Red blink ~5 Hz for 5s then PAIRING (error occurred)
    };

    /**
     * @brief Construct and initialize board LED controller
     *
     * Allocates RMT TX channel, creates byte encoder for WS2812B timing,
     * and creates ESP timers for blink/timeout functionality.
     *
     * @param gpio GPIO pin connected to WS2812B LED data line
     *
     * @note Uses ESP_ERROR_CHECK for unrecoverable initialization failures
     * @note No exceptions thrown (ESP-IDF does not support C++ exceptions)
     */
    explicit BoardLed(uint8_t gpio);

    /**
     * @brief Destructor - cleanup RMT and timer resources
     *
     * Stops timers, deletes timer handles, disables and deletes RMT channel.
     * Ensures no resource leaks.
     */
    ~BoardLed();

    // Non-copyable (RMT handles are unique resources)
    BoardLed(const BoardLed&) = delete;
    BoardLed& operator=(const BoardLed&) = delete;

    /**
     * @brief Set LED status state
     *
     * Changes LED state and starts appropriate blink/timeout behavior.
     *
     * @param state Desired LED state
     */
    void set_state(State state);

private:
    /**
     * @brief Apply RGB color to WS2812B LED
     *
     * @param r Red component (0-255)
     * @param g Green component (0-255)
     * @param b Blue component (0-255)
     */
    void apply_color(uint8_t r, uint8_t g, uint8_t b);

    /**
     * @brief Turn off LED (set to black)
     */
    void clear();

    /**
     * @brief Blink timer callback (static wrapper for member function)
     */
    static void blink_timer_cb(void* arg);

    /**
     * @brief Timeout timer callback (static wrapper for member function)
     */
    static void timeout_timer_cb(void* arg);

    /**
     * @brief Blink timer handler (member function)
     */
    void on_blink();

    /**
     * @brief Timeout timer handler (member function)
     */
    void on_timeout();

    // RMT resources
    rmt_channel_handle_t m_rmt_chan;   ///< RMT TX channel handle
    rmt_encoder_handle_t m_bytes_enc;  ///< WS2812B bytes encoder handle

    // Timing resources
    esp_timer_handle_t m_blink_timer;   ///< Periodic blink timer
    esp_timer_handle_t m_timeout_timer; ///< One-shot timeout timer

    // State tracking
    State m_state;      ///< Current LED state
    bool  m_blink_on;   ///< Blink phase (true = LED on, false = LED off)
};

#endif // BOARD_LED_HPP
