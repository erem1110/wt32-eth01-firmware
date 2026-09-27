# WT32-ETH01 16-Relay Standalone Controller Firmware

Production-grade, standalone ESP32 firmware for the **WT32-ETH01** board targeting ESP32 Arduino Core 2.x. This firmware provides a high-reliability, 100% non-blocking controller for 16 active-low relays and 16 configurable inputs via I2C PCF8574 expanders, complete with an embedded responsive Single-Page Application (SPA) Web UI, real-time WebSocket dashboard, Home Assistant MQTT auto-discovery, emergency interlocking, and persistent NVS storage.

---

## 1. Hardware Architecture & Pin Mapping

### WT32-ETH01 Ethernet PHY (LAN8720)
| Signal / Function | ESP32 Pin | Note |
|---|---|---|
| RMII MDC | GPIO 23 | Management Data Clock |
| RMII MDIO | GPIO 18 | Management Data I/O |
| RMII CLK (REF_CLK) | GPIO 0 | 50 MHz Input (`ETH_CLOCK_GPIO0_IN`) |
| PHY Power / Oscillator Enable | GPIO 16 | External Crystal Oscillator Power Enable |
| PHY I2C/SMI Address | 1 | LAN8720 PHY Address |

### I2C Bus
| Signal | ESP32 Pin | Bus Speed |
|---|---|---|
| SDA | GPIO 14 | 100 kHz standard mode |
| SCL | GPIO 15 | 100 kHz standard mode |

### PCF8574 I/O Expanders
| Expander | I2C Address | Function | Polarity / Notes |
|---|---|---|---|
| `pcf_out1` | `0x21` | Relays R2, R4, R6, R8, R10, R12, R14, R16 | Active-LOW (Pin LOW = Relay ON) |
| `pcf_out2` | `0x20` | Relays R1, R3, R5, R7, R9, R11, R13, R15 | Active-LOW (Pin LOW = Relay ON) |
| `pcf_in1` | `0x22` | Digital Inputs 1 to 8 (P0..P7) | Default Active-LOW (internal pullup) |
| `pcf_in2` | `0x23` | Digital Inputs 9 to 16 (P0..P7) | Default Active-LOW (internal pullup) |

### Detailed Relay Mapping Table
| Relay | PCF8574 Chip | Pin | Physical Bit |
|---|---|---|---|
| **R1** | `out2` (`0x20`) | P7 | Bit 7 |
| **R2** | `out1` (`0x21`) | P0 | Bit 0 |
| **R3** | `out2` (`0x20`) | P6 | Bit 6 |
| **R4** | `out1` (`0x21`) | P1 | Bit 1 |
| **R5** | `out2` (`0x20`) | P5 | Bit 5 |
| **R6** | `out1` (`0x21`) | P2 | Bit 2 |
| **R7** | `out2` (`0x20`) | P4 | Bit 4 |
| **R8** | `out1` (`0x21`) | P3 | Bit 3 |
| **R9** | `out2` (`0x20`) | P3 | Bit 3 |
| **R10** | `out1` (`0x21`) | P4 | Bit 4 |
| **R11** | `out2` (`0x20`) | P2 | Bit 2 |
| **R12** | `out1` (`0x21`) | P5 | Bit 5 |
| **R13** | `out2` (`0x20`) | P1 | Bit 1 |
| **R14** | `out1` (`0x21`) | P6 | Bit 6 |
| **R15** | `out2` (`0x20`) | P0 | Bit 0 |
| **R16** | `out1` (`0x21`) | P7 | Bit 7 |

---

## 2. NVS Storage Schema (`Preferences`)

Configuration and states persist across reboot using ESP32 Preferences (NVS):

| Namespace | Key | Type | Description |
|---|---|---|---|
| `sys_sec` | `configured` | bool | `true` if first-run setup completed |
| | `user` | string | Administrator username |
| | `salt` | string | 16-byte random hex salt (32 chars) |
| | `hash` | string | Salted SHA-256 password hash (64 hex chars) |
| | `timeout` | uint32 | Web session timeout in seconds |
| | `lock_att` | uint8 | Max failed login attempts before lockout |
| | `lock_time` | uint32 | Lockout duration in seconds |
| `sys_net` | `dhcp` | bool | DHCP enable/disable |
| | `ip` / `gw` / `mask` / `dns` | string | Static network IP configuration |
| | `host` | string | Controller network hostname |
| `sys_mqtt` | `enabled` | bool | MQTT client enable/disable |
| | `host` / `port` | string / uint16 | Broker address & port |
| | `user` / `pass` | string / string | MQTT credentials |
| | `cid` / `topic` | string / string | Client ID & base topic structure |
| | `ha_disc` | bool | Home Assistant Auto-Discovery enable |
| `sys_time` | `ntp` | string | SNTP server hostname (`pool.ntp.org`) |
| | `gmt` / `dst` | int32 / int32 | Timezone GMT offset and DST offset (seconds) |
| `sys_emg` | `locked` | bool | **Persisted Emergency Lock** across reboot |
| | `trig_mask` | uint16 | Bitmask of inputs triggering emergency |
| | `off_all` | bool | Turn off all relays (vs selected target mask) |
| | `tgt_mask` | uint16 | Target relays affected by emergency |
| | `lock_rel` | bool | Enable relay command blocking during emergency |
| | `man_rst` | bool | Require 2-step manual confirmation from web |
| | `auto_s` | uint32 | Auto-reset countdown seconds (0 = disabled) |
| | `latch` | bool | Latch emergency state until explicitly cleared |
| `sys_gdef` | `deb` / `long` / `dbl` | uint16 | Global default debounce, long-press, double-click |
| | `tout` / `tact` | uint16 / uint8 | Default signal timeout and timeout action |
| `sys_relay` | `r_cfg_<0..15>` | blob | Binary `RelayConfig` struct per relay |
| | `r_last_<0..15>` | bool | Last known state for `RESTORE_PREVIOUS` mode |
| `sys_input` | `in_cfg_<0..15>` | blob | Binary `InputConfig` struct per input |
| `sys_scene` | `sc_cfg_<0..7>` | blob | Binary `SceneConfig` struct per scene |

---

## 3. Subsystem Architecture Highlights

- **Relay Interlock & Safety Choke Point**:
  All sources of relay switching (`SRC_WEB`, `SRC_MQTT`, `SRC_INPUT`, `SRC_SCENE`, `SRC_TIMER`) must pass through `RelaySubsystem::turnRelayOn()`. This choke point enforces:
  1. Rejection if Emergency lock is active.
  2. Strict Mutual Exclusion: Within any interlock group (1..8), at most one relay is ON. Turning on a relay immediately turns off any other relay in the same group. Scenes (such as "All ON") cannot violate this invariant.
  3. Non-blocking ON/OFF delays.
  4. Auto-Off Timer Epoch: Generation counter prevents stale timer expiration if a relay was turned off and back on before the timer fired.

- **Non-blocking Input State Machine**:
  Each of the 16 inputs runs an independent per-instance state machine:
  - Debounce filtering (default 50 ms).
  - Short-press fires **only** after the double-click window (default 350 ms) expires and no long-press was confirmed.
  - Long-press triggers once the duration exceeds the threshold (default 1500 ms) while held.
  - Double-click triggers immediately upon a second press within the window.
  - Signal timeout warns or triggers auto-release/emergency if held continuously.
  - Polarity and startup behaviors (`IGNORE`, `PROCESS`, `WAIT_CHANGE`) configurable per input.

- **Emergency Subsystem**:
  - Activated by designated physical inputs, web UI button, or MQTT command.
  - Immediately shuts off relays, cancels all auto-off timers, locks relays against new ON commands, and displays a prominent red banner on the dashboard.
  - Persists lock state to NVS so emergency state survives power cycles.
  - Unlocked via two-step confirmation dialog on the Web UI.

- **Web UI & WebSocket**:
  - Embedded in PROGMEM (zero external CDN or internet dependencies).
  - Real-time updates via WebSockets on port 81 with graceful 2-second light-polling fallback on port 80.
  - Modern, responsive desktop/tablet/mobile layout.
  - Visible "Trusted LAN Operation Only" disclaimer.

- **Home Assistant MQTT Integration**:
  - Publishes MQTT Discovery payloads for all 16 relays (`switch`), 16 inputs (`binary_sensor`), and Emergency status.
  - Supports status availability LWT (`<base>/status`).

---

## 4. Build & Installation

### Option A: PlatformIO (Recommended)
1. Open this folder in VS Code with PlatformIO or use CLI:
   ```bash
   pio run
   ```
2. Upload to WT32-ETH01 via USB-to-UART adapter (3.3V, TX->RX0, RX->TX0, GND->GND, IO0 pulled LOW during boot to enter flashing mode):
   ```bash
   pio run --target upload
   ```

### Option B: Arduino IDE (v2.x)
1. Install **ESP32 Arduino Core** (version `2.0.14` or any `2.x`).
2. Install required libraries from Library Manager:
   - `PubSubClient` (by Nick O'Leary)
   - `WebSockets` (by Markus Sattler)
   - `ArduinoJson` (v6 or v7)
3. Select Board: **ESP32 Dev Module** or **WT32-ETH01**.
4. Open `wt32-eth01-firmware.ino` and click **Upload**.

---

## 5. First-Run Setup

1. Connect the WT32-ETH01 to your LAN via Ethernet.
2. The board obtains an IP address via DHCP (logged to Serial @ 115200 baud).
3. Open `http://<device-ip>/` in your browser.
4. The First-Run Setup Wizard prompts you to set an Administrator username, password, and session timeout.
5. After setup, log in to access the Dashboard.
