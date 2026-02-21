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

## Integration

Add to your project's top-level `CMakeLists.txt`:

```cmake
set(EXTRA_COMPONENT_DIRS "/home/shaun/src/esp32-zigbee-common")
```

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

- **Phase 0 (2026-02-21)**: Repository structure created, empty placeholders
- **Phase 1 (planned)**: Implement BoardLed, NvsStore, ButtonHandler classes
- **Phase 2+ (planned)**: Full migration of LED and LD2450 projects

## License

MIT License (same as parent projects)
