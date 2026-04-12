# ESP32 Zigbee Common Components

Shared C++ components for ESP32-H2/C6 Zigbee projects.

## Overview

This repository provides reusable components that eliminate code duplication across multiple Zigbee projects (LED controller, LD2450 sensor, etc.). Components use "better C with classes" design - RAII, encapsulation, and type safety without complex C++ features.

## Components

### board_led
Status LED state machine for onboard WS2812/RGB LEDs. Provides consistent visual feedback across all projects:
- Pairing mode (blue blink)
- Joined (green flash)
- Error states (red patterns)

### zigbee_core
Zigbee stack lifecycle management:
- **ZigbeeApp**: Platform config, stack init, signal handler, steering retry
- **ButtonHandler**: Factory reset button with hold-time detection (3s network reset, 10s full reset)
- **zgp_stub.c**: Green Power stub (must remain C for linker compatibility)

### nvs_helpers
Typed NVS storage utilities with RAII handle management:
- Template methods: `save<T>()`, `load<T>()` for type safety
- Blob support for complex structures
- Automatic handle cleanup
- Replaces repetitive nvs_get/nvs_set boilerplate

### cli_framework
UART-based command-line interface framework:
- Command registration: `register_command(name, description, handler)`
- Input parsing and dispatch
- Consistent CLI experience across projects

### wifi_manager
WiFi AP/STA manager for ESP32-C6 web configuration (compiled only on C6):
- `wifi_manager_init(const char *hostname_prefix)` — prefix used for AP SSID and fallback hostname (e.g. `"ld2450"` → `"ld2450-A1B2C3"`)
- AP mode with captive DNS; STA mode with mDNS; APSTA for scan-while-provisioning
- Credentials and hostname persisted in NVS `"wifi_cfg"` namespace
- Coexistence with Zigbee 802.15.4 via `esp_coex_wifi_i154_enable()`

### ota_check
OTA update availability checker for ESP32-C6 (compiled only on C6):
- `ota_check_init(const ota_check_config_t *cfg)` — device-specific image type and NVS namespace
- Background periodic check with configurable interval (default 12 h)
- Compares running firmware version against OTA index JSON
- Settings (URL, interval) persisted in caller's NVS namespace

### web_server_base
Shared HTTP server infrastructure for ESP32-C6 web UI:
- SPIFFS mount + firmware-embedded asset sync on version change
- AP-mode captive portal setup page (device name parameterised)
- All WiFi endpoints: `/api/wifi-scan`, `POST /api/wifi`, `POST /api/wifi-reset`
- All OTA endpoints: status, check, trigger, upload, interval, index-url
- System endpoints: `/api/status`, restart, zb-reset, factory-reset
- Diagnostics: `GET /api/diag` (boot count, reset reason, last uptime, heap), `POST /api/diag/reset`
- Device endpoint registration: `web_server_base_register(uri, method, handler, is_websocket)`
- Calls `ota_check_init()` internally

### crash_diag
Crash diagnostics and remote telemetry (pure C):
- Monotonic boot counter persisted in NVS (`crash_diag` namespace)
- Reset reason captured via `esp_reset_reason()` at boot
- Last uptime before reset via `RTC_NOINIT_ATTR` LP RAM (survives software/panic/WDT resets, not power loss)
- Minimum free heap tracked since boot
- Call `crash_diag_init()` once in `app_main()` after `nvs_flash_init()`
- Call `crash_diag_get_data()` during cluster creation to seed ZCL attributes
- Call `crash_diag_update_uptime()` periodically (e.g. every sensor poll)
- Call `crash_diag_reset_boot_count()` to clear the NVS counter (CLI / ZCL attr write / Web UI)

## Integration

### Method 1: ESP-IDF Component Manager (Recommended)

Add to your project's `main/idf_component.yml`:

```yaml
dependencies:
  board_led:
    git: https://github.com/ShaunPCcom/esp32-common.git
    path: board_led
  zigbee_core:
    git: https://github.com/ShaunPCcom/esp32-common.git
    path: zigbee_core
  # Add other components as needed:
  # nvs_helpers, cli_framework, crash_diag
```

Components are automatically downloaded to `managed_components/` during build.

### Method 2: Local Development (EXTRA_COMPONENT_DIRS)

For local development with live editing:

```cmake
# CMakeLists.txt
set(EXTRA_COMPONENT_DIRS "/path/to/esp32-common")
```

### Usage in Code

Then use in your code:

```cpp
#include "board_led.hpp"
#include "nvs_helpers.hpp"

// Create instances
BoardLed status_led(GPIO_NUM_8);
NvsStore config("my_namespace");
```

## Design Principles

1. **RAII**: Resources (timers, handles, tasks) cleaned up automatically
2. **No magic numbers**: Projects pass defaults as constructor parameters
3. **Minimal dependencies**: Each component declares only what it needs
4. **ESP-IDF native**: Uses standard ESP-IDF APIs, no external libraries
5. **Simple C++**: constexpr, classes, templates for type safety - no exceptions, no STL

## Status

- **Phase 0 (2026-02-21)**: ✅ Repository structure created, component placeholders
- **Phase 1 (2026-02-21)**: ✅ BoardLed and ButtonHandler implemented and integrated
  - Published to GitHub: https://github.com/ShaunPCcom/esp32-common
  - In use by: zb-h2-LED-lighting project (v1.1.1+)
  - Integration: ESP-IDF Component Manager
- **Phase 2 (2026-03-07)**: ✅ crash_diag component added (ported from ld2450-zb-h2)
  - Pure C, no Zigbee dependency, reusable by any ESP32 Zigbee project
- **Phase 3 (2026-04-12)**: ✅ wifi_manager, ota_check, web_server_base added (C6 web platform)
  - crash_diag extended with `crash_diag_reset_boot_count()`
  - Shared web infrastructure for LD2450-C6 and LED-C6 variants

## Projects Using This

- **zb-h2-LED-lighting** (v1.1.1+): BoardLed, ButtonHandler via Component Manager
- **ld2450-zb-h2** (v1.1.2+): BoardLed, ButtonHandler via Component Manager; crash_diag (pending migration)
- **zb-h2-LED-lighting** (pending): crash_diag port in progress

## License

MIT License (same as parent projects)
