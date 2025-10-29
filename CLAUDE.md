# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## About This Keyboard

**Keychron K11 Max - ANSI Layout with Encoder and RGB**

- 69-key split space keyboard
- STM32F401 processor
- Wireless: Bluetooth (3 devices) + 2.4GHz (1 device)
- RGB Matrix: SNLED27351 driver (per-key RGB)
- Rotary encoder (volume control)
- DIP switch for Mac/Windows mode
- VIA support enabled
- 5 layers: MAC_BASE, WIN_BASE, MAC_FN1, WIN_FN1, FN2

Keyboard path: `keyboards/keychron/k11_max/ansi_encoder/rgb`
Keymap location: `keyboards/keychron/k11_max/ansi_encoder/rgb/keymaps/via/keymap.c`

Official QMK docs: https://docs.qmk.fm

## Quick Start Commands

All commands below are for the K11 Max ANSI RGB keyboard:

```bash
# Build firmware with your custom keymap
qmk compile -kb keychron/k11_max/ansi_encoder/rgb -km via

# Build and flash to keyboard (put keyboard in bootloader mode first)
qmk flash -kb keychron/k11_max/ansi_encoder/rgb -km via

# Or using Make syntax (shorter after first use)
make keychron/k11_max/ansi_encoder/rgb:via
make keychron/k11_max/ansi_encoder/rgb:via:flash

# Clean build artifacts (use when builds fail)
qmk clean

# Set as default keyboard (then just use "qmk compile")
qmk config user.keyboard=keychron/k11_max/ansi_encoder/rgb
qmk config user.keymap=via

# After setting defaults:
qmk compile
qmk flash
```

## Entering Bootloader Mode

To flash firmware, put the keyboard in bootloader mode:
1. Unplug USB cable
2. Hold **ESC** (top-left key)
3. Plug in USB cable while holding ESC
4. Release ESC after 2 seconds
5. Run `qmk flash` command

Bootloader: STM32-DFU

## Codebase Architecture

### Directory Structure

```
keyboards/keychron/          # Keychron keyboard implementations
  ├── common/                # Shared Keychron modules
  │   ├── wireless/          # Wireless (Bluetooth/2.4G) support
  │   ├── rgb/               # RGB lighting effects
  │   ├── snap_click/        # Snap click feature
  │   ├── debounce/          # Custom debounce implementations
  │   ├── language/          # Language support (Mac/Win layouts)
  │   └── keychron_common.mk # Common build rules
  ├── k11_max/               # K11 Max keyboard
  │   ├── ansi_encoder/      # ANSI layout with encoder
  │   ├── iso_encoder/       # ISO layout with encoder
  │   ├── jis_encoder/       # JIS layout with encoder
  │   ├── config.h           # Hardware configuration
  │   └── rules.mk           # Build rules
  └── [other models]/        # Other Keychron models

quantum/                     # QMK core features (keycodes, layers, etc.)
tmk_core/                   # TMK core (matrix scanning, USB, etc.)
drivers/                    # Hardware drivers (LED, sensors, etc.)
builddefs/                  # Build system definitions
platforms/                  # Platform-specific code (AVR, ARM, ChibiOS)
docs/                       # Official documentation
```

### Keyboard Configuration Hierarchy

Keyboards follow a hierarchical structure with inheritance:

1. **Keyboard root** (`keyboards/keychron/k11_max/`)
   - `config.h` - Base hardware configuration (encoder, SPI, wireless pins)
   - `rules.mk` - Includes wireless.mk and keychron_common.mk
   - `info.json` - Keyboard metadata
   - `k11_max.c` - Keyboard-level code

2. **Variant level** (`keyboards/keychron/k11_max/ansi_encoder/rgb/`)
   - `config.h` - Variant-specific overrides
   - `info.json` - USB PID, RGB matrix config
   - `rules.mk` - Usually intentionally blank

3. **Keymap level** (`keyboards/keychron/k11_max/ansi_encoder/rgb/keymaps/via/`)
   - `keymap.c` - Layer definitions and key assignments
   - `rules.mk` - Enable VIA support or other keymap features

### Key Architecture Patterns

#### Wireless Support
- Keychron wireless keyboards use a custom `LK_WIRELESS_ENABLE` feature
- Bluetooth module: LKBT51 (communication via SPI)
- Supports both Bluetooth (up to 3 devices) and 2.4GHz (1 device)
- Configuration in `keyboards/keychron/common/wireless/`:
  - `wireless.c` - Main wireless logic
  - `lkbt51.c` - Bluetooth module communication
  - `battery.c` - Battery management
  - `lpm.c` - Low power mode
  - `transport.c` - HID report transport layer

#### Keychron Common Features
All Keychron keyboards include (`keychron_common.mk`):
- Factory test mode (`FACTORY_TEST_ENABLE`)
- Adaptive NKRO (`APDAPTIVE_NKRO_ENABLE`)
- Raw HID interface for configuration tools
- Language-specific layouts (Mac/Windows mode switching)

#### RGB Matrix
- Driver: SNLED27351 (SPI communication)
- Per-key RGB or mixed (per-key + underglow)
- Animations defined in keyboard's `info.json`
- LED shutdown pin controlled in hardware config

#### Custom Features
- **Snap Click** - Enhanced typing feedback (optional, enabled via `SNAP_CLICK_ENABLE`)
- **Custom Debounce** - Optimized debouncing for wireless keyboards
- **Factory Test** - Manufacturing test mode (Fn+J+Z during power-on)

### Build System

The QMK build system uses GNU Make with Python CLI:
1. Top-level `Makefile` parses `<keyboard>:<keymap>:<target>` format
2. Calls `qmk` CLI to resolve keyboard paths and generate build rules
3. Includes `builddefs/build_keyboard.mk` for actual compilation
4. Uses `builddefs/common_features.mk` to enable/disable features

Output files go to `.build/` directory:
- `.bin` - Binary firmware (for DFU bootloaders)
- `.hex` - Intel HEX format
- `.uf2` - USB Flashing Format (for RP2040)

## Working with Keyboards

### Creating a New Keymap

```bash
# Create a new keymap from default
qmk new-keymap -kb keychron/k11_max/ansi_encoder/rgb

# Or manually copy
cp -r keyboards/keychron/k11_max/ansi_encoder/rgb/keymaps/default keyboards/keychron/k11_max/ansi_encoder/rgb/keymaps/<name>
```

### Keymap Structure

```c
// keymap.c
const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    [0] = LAYOUT_<variant>(  // Base layer
        KC_ESC,  KC_1,    KC_2,    ...
        KC_TAB,  KC_Q,    KC_W,    ...
        ...
    ),
    [1] = LAYOUT_<variant>(  // Function layer
        _______,  KC_F1,   KC_F2,   ...
        ...
    ),
};
```

Important macros:
- `_______` - Transparent (passes through to lower layer)
- `XXXXXXX` - Blocked (no action)
- `MO(n)` - Momentarily activate layer n
- `LT(n, kc)` - Layer tap (hold for layer n, tap for keycode)

### Testing Changes

```bash
# Run unit tests
qmk pytest

# Or using make
make test:all

# Test specific keyboard build without flashing
qmk compile -kb keychron/k11_max/ansi_encoder/rgb -km via
```

## Wireless-Specific Development

### Enabling Wireless Features

In `rules.mk`:
```make
include keyboards/keychron/common/wireless/wireless.mk
```

This automatically defines:
- `LK_WIRELESS_ENABLE`
- `WIRELESS_CONFIG_ENABLE`
- `NO_USB_STARTUP_CHECK`

### Hardware Pin Configuration

In keyboard's `config.h`:
```c
// Mode selection
#define P2P4_MODE_SELECT_PIN A10
#define BT_MODE_SELECT_PIN A9

// Bluetooth module
#define LKBT51_RESET_PIN C4
#define LKBT51_INT_INPUT_PIN B1

// Power sensing
#define USB_POWER_SENSE_PIN B0
#define BAT_CHARGING_PIN B13
#define BAT_LOW_LED_PIN B12

// Host device counts
#define BT_HOST_DEVICES_COUNT 3
#define P24G_HOST_DEVICES_COUNT 1
```

### Battery and Power Management

- Battery level displayed via LED animation
- Low power mode (LPM) automatically engages when idle
- Backlight timeout: 40s disconnected, 600s connected
- Configure in `DISCONNECTED_BACKLIGHT_DISABLE_TIMEOUT` / `CONNECTED_BACKLIGHT_DISABLE_TIMEOUT`

## Code Style

- Indentation: 4 spaces (NOT tabs)
- C language standard for firmware code
- Python for CLI tools (formatted with yapf)
- Follow existing patterns in keyboard files
- Use `clang-format` for C/C++ code formatting (config in `.clang-format`)

## Git Workflow

The main branch is `master`. This fork is on branch `wireless_playground`.

When making commits, be descriptive:
```bash
git add keyboards/keychron/k11_max/
git commit -m "Added K11 Max JIS variant"
```

## Common Issues

### Build Errors
- Ensure submodules are initialized: `qmk git-submodule --sync`
- Clean build artifacts: `qmk clean`
- Check Python environment: `qmk doctor`

### USB Detection
- Wireless keyboards use `NO_USB_STARTUP_CHECK` to boot without USB
- `KEEP_USB_CONNECTION_IN_WIRELESS_MODE` allows simultaneous USB + BT

### Encoder Issues
- Set `ENCODER_DEFAULT_POS` to `0x3` for Keychron encoders
- Add `ENCODER_MAP_KEY_DELAY 2` for proper registration

### RGB Matrix Not Working
- Check `LED_DRIVER_SHUTDOWN_PIN` is set correctly
- Verify SPI pins configuration (`SPI_SCK_PIN`, `SPI_MOSI_PIN`, `SPI_MISO_PIN`)
- Ensure `SNLED23751_SPI_DIVISOR 16` is defined for timing

## File Locations for Quick Reference

- Keyboard implementations: `keyboards/keychron/`
- Core QMK features: `quantum/`
- Build system: `builddefs/`
- Documentation: `docs/` or https://docs.qmk.fm
- QMK CLI: `qmk <command> --help`

## Useful QMK Features

- **VIA**: Real-time keymap editing without reflashing (`VIA_ENABLE = yes`)
- **Tap Dance**: Multiple functions on single key with taps
- **Combo**: Trigger action by pressing multiple keys simultaneously
- **Leader Key**: Vim-style key sequences
- **Auto Shift**: Hold key for shifted version
- **Layers**: Up to 32 layers supported
