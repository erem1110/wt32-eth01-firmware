# Advanced Controller Features Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Implement 4 advanced features for WT32-ETH01 16-channel controller: (1) Virtual Energy (kWh) & Relay Cycle Odometer, (2) Astro-Clock (Sunrise/Sunset), (3) IF-THEN Logic Engine, and (4) Web UI Enhancements & PWA Mobile App.

**Architecture:**
- **Energy & Odometer:** Extends `RelayConfig` with rated wattage & icon/room; tracks cycle count & cumulative on-seconds in NVS periodically without flash wear; computes kWh & health %.
- **Astro-Clock:** NOAA astronomical solar calculation algorithm built into firmware with latitude/longitude/timezone; integrated into `ScheduleSubsystem` with sunrise/sunset triggers.
- **Logic Engine:** Dedicated `LogicEngine` module managing 8 IF-THEN rules with edge-triggered execution on input/relay/time events.
- **Web UI & PWA:** Serves `/manifest.json`, adds PWA meta tags, adds Device Icons & Room filters, adds Energy & Health tab, adds Logic Rules tab, adds Sunrise/Sunset schedules.

**Tech Stack:** ESP32 Arduino framework, FreeRTOS, ArduinoJson, Preferences (NVS), HTML5/CSS3/JavaScript (Embedded WebSockets).

---

## Tasks

### Task 1: Data Structures and Types Extension (`types.h`, `config.h`)
- [ ] Add `DeviceIcon` enum (Light, Socket, Gate, Boiler, Pump, Fan/AC, General) to `types.h`.
- [ ] Update `RelayConfig`: add `uint16_t rated_power_w`, `uint8_t device_icon`, `char room[20]`.
- [ ] Add `RelayEnergyStats` struct (`uint32_t total_cycles`, `uint32_t total_on_seconds`).
- [ ] Update `TimeConfig`: add `float latitude`, `float longitude`.
- [ ] Add `ScheduleTriggerType` enum (`TRIGGER_CLOCK`, `TRIGGER_SUNRISE`, `TRIGGER_SUNSET`) and offset in `ScheduleItem`.
- [ ] Add `LogicRule` struct and enums for conditions, operators, and actions.

### Task 2: Astro-Clock Solar Calculation Module (`astro_clock.h`, `astro_clock.cpp`)
- [ ] Create `astro_clock.h` and `astro_clock.cpp` with compact NOAA solar calculations.
- [ ] Compute sunrise and sunset minutes of day given date, lat, lon, and GMT offset.
- [ ] Add `isNight()` helper method.
- [ ] Integrate into `ScheduleSubsystem` so schedules trigger on sunrise/sunset ± offset.

### Task 3: Virtual Energy Meter & Relay Odometer Subsystem
- [ ] Add energy stats tracking in `relay_subsystem.h` and `relay_subsystem.cpp`.
- [ ] Add NVS storage methods in `storage.h` and `storage.cpp` for energy stats with wear-leveling (periodic save every 5 min or on state change).
- [ ] Add methods to retrieve and reset energy stats per relay.

### Task 4: IF-THEN Logic Engine Subsystem (`logic_engine.h`, `logic_engine.cpp`)
- [ ] Create `logic_engine.h` and `logic_engine.cpp` with support for up to 8 rules.
- [ ] Implement condition evaluation (`COND_INPUT`, `COND_RELAY`, `COND_TIME`, `COND_SUN_STATE`) with `OP_AND`/`OP_OR`.
- [ ] Implement action execution (`ACT_RELAY_ON`, `ACT_RELAY_OFF`, `ACT_RELAY_TOGGLE`, `ACT_ACTIVATE_SCENE`).
- [ ] Edge-trigger execution (transition from false to true) to prevent spamming actions every cycle.
- [ ] Integrate into `wt32-eth01-firmware.ino` loop and event hooks.

### Task 5: Web Server API Endpoints (`web_server.h`, `web_server.cpp`)
- [ ] Implement `/manifest.json` endpoint for PWA.
- [ ] Implement `/api/energy/status` and `/api/energy/reset` endpoints.
- [ ] Implement `/api/logic/config` GET and POST endpoints.
- [ ] Update `/api/time/config` to support latitude/longitude.
- [ ] Update `/api/schedule/config` to support sunrise/sunset triggers.
- [ ] Update `/api/relay/config` to support power, icon, and room.

### Task 6: Web UI Dashboard, Energy & Logic Tabs (`web_assets.h`)
- [ ] Add PWA headers & manifest link.
- [ ] Add Room filter buttons & Device Icons to Dashboard relays.
- [ ] Add **Energy & Health** tab with total power/energy stats, cycle counter, health progress bar, and reset buttons.
- [ ] Add **Logic Rules** tab with interactive rule builder (IF condition 1 AND/OR condition 2 THEN action).
- [ ] Update Schedules tab with Sunrise/Sunset options and solar info card.

### Task 7: Compilation, OTA Flashing & Verification
- [ ] Compile firmware with PlatformIO.
- [ ] Upload binary to WT32-ETH01 via OTA (`ota_updater.py`).
- [ ] Verify HTTP APIs, WebSockets, and PWA live on the board at `192.168.11.164`.
