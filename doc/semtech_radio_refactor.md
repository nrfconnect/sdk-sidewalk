# Semtech Radio Refactor

The Sidewalk Semtech radio refactor is based on "LBM_Zephy" LoRa Basics Modem integration in Zephyr OS.
Project URL: https://github.com/Lora-net/LBM_Zephyr

## Semtech DTS and Sidewalk Driver Structure Mapping

This document describes the complete mapping between Semtech radio DTS (Device Tree Source) configurations and the Sidewalk driver structures for SX126X radio modules. The implementation properly handles both required and optional DTS properties with appropriate default values.

### DTS Properties Analysis

#### Required Properties (✅ Mapped)
| DTS Property | Driver Structure Field | Status | Description |
|--------------|----------------------|--------|-------------|
| `reset-gpios` | `gpio_power` | ✅ Mapped | GPIO for radio reset signal |
| `busy-gpios` | `gpio_radio_busy` | ✅ Mapped | GPIO for radio busy signal |
| `reg-mode` | `regulator_mode` | ✅ Mapped | Power regulator mode (LDO/DCDC) |
| `tcxo-voltage` | `tcxo.voltage` | ✅ Mapped | TCXO supply voltage |
| `tcxo-wakeup-time` | `tcxo.timeout` | ✅ Mapped | TCXO wakeup time in ms |

#### Optional Properties (✅ Mapped with Defaults)
| DTS Property | Driver Structure Field | Default | Status | Description |
|--------------|----------------------|---------|--------|-------------|
| `dio1-gpios` | `gpio_int1` | `INVALID_DT_GPIO` | ✅ Mapped | GPIO for DIO1 interrupt signal |
| `spi-max-frequency` | `bus_selector.speed_hz` | `8MHz` | ✅ Mapped | SPI bus frequency |
| `cs-gpios` (from arduino_spi) | `bus_selector.client_selector` | `INVALID_DT_GPIO` | ✅ Mapped | SPI chip select GPIO |
| `rx-boosted` | `rx_boost` | `false` | ✅ Mapped | RX boosted mode configuration |

#### Optional Properties (❌ Not Implemented in Driver Structure)
| DTS Property | Reason | Description |
|--------------|--------|-------------|
| `dio2-gpios` | No field in driver structure | DIO2 as generic IRQ line |
| `dio2-as-rf-switch` | No field in driver structure | DIO2 as RF switch control |
| `dio3-gpios` | No field in driver structure | DIO3 as generic IRQ line |
| `dio3-as-tcxo-control` | No field in driver structure | DIO3 as TCXO control |
| `xtal-capacitor-value-xta` | No field in driver structure | XTA trimming capacitor value |
| `xtal-capacitor-value-xtb` | No field in driver structure | XTB trimming capacitor value |
| `tx-power-offset` | No field in driver structure | TX power offset configuration |

### Driver Structure Fields Mapping

```c
typedef struct {
    uint8_t id;                                    // ✅ Set to SEMTECH_ID_SX1262
    uint8_t regulator_mode;                        // ✅ From DTS reg-mode (required)
    bool rx_boost;                                 // ✅ From DTS rx-boosted (optional, default: false)
    int8_t lna_gain;                              // ✅ Hardcoded (RADIO_RX_LNA_GAIN)
    const radio_sx126x_get_pa_cfg_t pa_cfg_callback; // ✅ Callback function
    const radio_sx126x_get_mfg_trim_val_t trim_cap_val_callback; // ✅ Callback function
    const radio_sx126x_get_dio3_cfg_t dio3_cfg_callback; // ❌ Not used
    const struct sid_pal_serial_bus_factory *bus_factory; // ✅ SPI factory

    uint32_t gpio_power;                          // ✅ From DTS reset-gpios (required)
    uint32_t gpio_int1;                           // ✅ From DTS dio1-gpios (optional)
    uint32_t gpio_radio_busy;                     // ✅ From DTS busy-gpios (required)
    uint32_t gpio_rf_sw_ena;                      // ❌ Not from DTS (GPIO_UNUSED_PIN)
    uint32_t gpio_tx_bypass;                      // ❌ Not from DTS (GPIO_UNUSED_PIN)

    struct sid_pal_serial_bus_client bus_selector; // ✅ From DTS cs-gpios + spi-max-frequency
    radio_sx126x_tcxo_t tcxo;                     // ✅ From DTS tcxo-voltage + tcxo-wakeup-time
    sid_pal_radio_state_transition_timings_t state_timings; // ✅ Hardcoded
    struct { uint8_t *p; size_t size; } internal_buffer; // ✅ Hardcoded buffer
    radio_sx126x_regional_config_t regional_config; // ✅ Regional parameters
} radio_sx126x_device_config_t;
```

### TCXO Configuration Logic

```c
.tcxo = {
    .ctrl = DT_PROP_OR(DT_NODELABEL(lora_semtech_sx126xmb2xxs), tcxo_wakeup_time, 0) > 0 ? 
        SX126X_TCXO_CTRL_DIO3 : SX126X_TCXO_CTRL_NONE,
    .voltage = DT_PROP_OR(DT_NODELABEL(lora_semtech_sx126xmb2xxs), tcxo_voltage, SX126X_TCXO_CTRL_1_8V),
    .timeout = DT_PROP_OR(DT_NODELABEL(lora_semtech_sx126xmb2xxs), tcxo_wakeup_time, 0),
},
```

## Implementation Details

### DTS Property Reading Strategy

The driver uses `DT_PROP_OR()` macro to read DTS properties with fallback values:

```c
// Example: Read regulator mode with fallback to DCDC
.regulator_mode = DT_PROP_OR(DT_NODELABEL(lora_semtech_sx126xmb2xxs), reg_mode, RADIO_SX126X_REGULATOR_DCDC)
```

### GPIO Configuration

GPIO pins are registered using `sid_gpio_utils_register_gpio()` with `GPIO_DT_SPEC_GET_OR()`:

```c
radio_sx1262_cfg.gpio_power = sid_gpio_utils_register_gpio(
    (struct gpio_dt_spec)GPIO_DT_SPEC_GET_OR(
        DT_NODELABEL(lora_semtech_sx126xmb2xxs), reset_gpios, INVALID_DT_GPIO));
```

### Default Values for Optional Properties

#### SX126X Defaults
```c
#define SX126X_DEFAULT_XTAL_CAP_XTA 0x12
#define SX126X_DEFAULT_XTAL_CAP_XTB 0x12
#define SX126X_DEFAULT_TX_POWER_OFFSET 0
#define SX126X_DEFAULT_RX_BOOSTED false
```

## Missing Features and Recommendations

### SX126X Missing Features
1. **DIO2/DIO3 GPIO support**: Add fields to driver structure for DIO2/DIO3 configuration
2. **XTAL capacitor values**: Add fields for XTA/XTB capacitor configuration
3. **TX power offset**: Add field for board-specific TX power offset
4. **DIO2 as RF switch**: Add support for DIO2 RF switch control

## Conclusion

The current implementation successfully maps the core DTS properties to the driver structures with proper handling of optional properties and default values. The implementation provides a clean separation between hardware configuration (DTS) and driver behavior while maintaining backward compatibility.

**Key improvements made**:
- ✅ Proper handling of optional DTS properties with defaults
- ✅ TCXO configuration with proper logic
- ✅ RX boosted mode support
- ✅ Comprehensive documentation of all properties

**Next steps**:
- Implement missing driver structure fields for advanced features
- Add validation and error handling
- Add unit tests for DTS property parsing
- Implement TX power calibration and RSSI calibration features 
