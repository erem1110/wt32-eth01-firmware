#pragma once

#include <Arduino.h>

const char INDEX_HTML[] PROGMEM = R"rawliteral(<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width,initial-scale=1.0">
<meta name="theme-color" content="#12161a">
<meta name="apple-mobile-web-app-capable" content="yes">
<meta name="apple-mobile-web-app-status-bar-style" content="black-translucent">
<meta name="apple-mobile-web-app-title" content="Relays">
<link rel="manifest" href="/manifest.json">
<title>WT32-ETH01 Relay Controller</title>

<style>
:root{--bg:#12161a;--card:#1b2127;--card-border:#2a333d;--text:#e4e9f0;--text-dim:#8b99a8;--primary:#2563eb;--primary-hover:#1d4ed8;--success:#10b981;--danger:#ef4444;--danger-hover:#dc2626;--warning:#f59e0b;--info:#06b6d4;--radius:8px;}
*{box-sizing:border-box;margin:0;padding:0;font-family:-apple-system,BlinkMacSystemFont,"Segoe UI",Roboto,Helvetica,Arial,sans-serif;}
body{background:var(--bg);color:var(--text);font-size:14px;line-height:1.5;min-height:100vh;display:flex;flex-direction:column;}
.trusted-lan-bar{background:#1e293b;color:#94a3b8;font-size:11px;padding:4px 12px;text-align:center;letter-spacing:0.5px;border-bottom:1px solid #334155;display:flex;justify-content:center;align-items:center;gap:8px;}
.trusted-lan-bar .badge{background:#334155;color:#f1f5f9;padding:1px 6px;border-radius:4px;font-weight:600;}
.emg-banner{background:var(--danger);color:#fff;padding:12px 16px;text-align:center;font-weight:700;display:none;align-items:center;justify-content:center;gap:16px;animation:pulse 1.8s infinite;}
@keyframes pulse{0%{opacity:1}50%{opacity:0.85}100%{opacity:1}}
header{background:var(--card);border-bottom:1px solid var(--card-border);padding:12px 20px;display:flex;align-items:center;justify-content:space-between;flex-wrap:wrap;gap:10px;}
.logo-group{display:flex;align-items:center;gap:10px;}
.logo-title{font-size:16px;font-weight:700;color:#fff;}
.header-meta{display:flex;align-items:center;gap:12px;font-size:12px;}
.status-pill{padding:3px 8px;border-radius:12px;font-weight:600;font-size:11px;}
.status-pill.online{background:rgba(16,185,129,0.2);color:var(--success);border:1px solid rgba(16,185,129,0.3);}
.status-pill.offline{background:rgba(239,68,68,0.2);color:var(--danger);border:1px solid rgba(239,68,68,0.3);}
.status-pill.polling{background:rgba(245,158,11,0.2);color:var(--warning);border:1px solid rgba(245,158,11,0.3);}
nav{background:#161c22;border-bottom:1px solid var(--card-border);display:flex;overflow-x:auto;padding:0 12px;scrollbar-width:none;}
nav::-webkit-scrollbar{display:none;}
nav button{background:none;border:none;color:var(--text-dim);padding:12px 14px;font-size:13px;font-weight:600;cursor:pointer;white-space:nowrap;border-bottom:2px solid transparent;transition:all 0.2s;}
nav button:hover{color:var(--text);}
nav button.active{color:var(--primary);border-bottom-color:var(--primary);}
main{flex:1;padding:16px 20px;max-width:1440px;margin:0 auto;width:100%;}
.tab-content{display:none;}
.tab-content.active{display:block;}
.grid-2{display:grid;grid-template-columns:repeat(auto-fit,minmax(320px,1fr));gap:16px;}
.grid-4{display:grid;grid-template-columns:repeat(auto-fill,minmax(280px,1fr));gap:14px;}
.card{background:var(--card);border:1px solid var(--card-border);border-radius:var(--radius);padding:16px;}
.card-title{font-size:14px;font-weight:700;color:#fff;margin-bottom:12px;display:flex;align-items:center;justify-content:space-between;}
.summary-bar{display:grid;grid-template-columns:repeat(auto-fit,minmax(140px,1fr));gap:10px;margin-bottom:16px;}
.summary-item{background:var(--card);border:1px solid var(--card-border);padding:10px 14px;border-radius:var(--radius);}
.summary-label{font-size:11px;color:var(--text-dim);text-transform:uppercase;letter-spacing:0.5px;}
.summary-value{font-size:16px;font-weight:700;color:#fff;margin-top:2px;}
.relay-card{background:#161d24;border:1px solid var(--card-border);border-radius:var(--radius);padding:14px;display:flex;flex-direction:column;justify-content:space-between;transition:border-color 0.2s;}
.relay-card.on{border-color:var(--success);box-shadow:0 0 10px rgba(16,185,129,0.15);}
.relay-card.locked{border-color:var(--danger);}
.relay-header{display:flex;justify-content:space-between;align-items:flex-start;margin-bottom:10px;}
.relay-name{font-weight:700;font-size:14px;color:#fff;}
.relay-num{font-size:11px;color:var(--text-dim);}
.relay-meta{display:flex;gap:6px;flex-wrap:wrap;margin-bottom:12px;}
.tag{font-size:10px;padding:2px 6px;border-radius:4px;background:#242c36;color:var(--text-dim);font-weight:600;}
.tag.warn{background:rgba(245,158,11,0.2);color:var(--warning);}
.tag.danger{background:rgba(239,68,68,0.2);color:var(--danger);}
.tag.success{background:rgba(16,185,129,0.2);color:var(--success);}
.relay-timer{font-size:11px;color:var(--warning);margin-bottom:10px;display:none;font-weight:600;}
.relay-actions{display:flex;gap:8px;}
.btn{background:var(--primary);color:#fff;border:none;padding:8px 14px;border-radius:6px;font-size:13px;font-weight:600;cursor:pointer;transition:background 0.2s;display:inline-flex;align-items:center;justify-content:center;gap:6px;}
.btn:hover{background:var(--primary-hover);}
.btn-secondary{background:#2a333d;color:var(--text);}
.btn-secondary:hover{background:#374350;}
.btn-danger{background:var(--danger);}
.btn-danger:hover{background:var(--danger-hover);}
.btn-success{background:var(--success);}
.btn-block{width:100%;}
.btn:disabled{opacity:0.5;cursor:not-allowed;}
.input-row{background:#161d24;border:1px solid var(--card-border);border-radius:var(--radius);padding:12px;display:flex;align-items:center;justify-content:space-between;gap:10px;}
.input-indicator{width:12px;height:12px;border-radius:50%;background:#374151;}
.input-indicator.active{background:var(--success);box-shadow:0 0 8px var(--success);}
.input-indicator.timeout{background:var(--danger);box-shadow:0 0 8px var(--danger);}
table{width:100%;border-collapse:collapse;font-size:13px;}
th,td{padding:8px 10px;text-align:left;border-bottom:1px solid var(--card-border);}
th{color:var(--text-dim);font-weight:600;font-size:11px;text-transform:uppercase;}
tr:hover{background:rgba(255,255,255,0.02);}
input,select{background:#12161a;border:1px solid var(--card-border);color:var(--text);padding:8px 10px;border-radius:6px;font-size:13px;width:100%;}
input:focus,select:focus{outline:none;border-color:var(--primary);}
.form-group{margin-bottom:12px;}
.form-label{display:block;font-size:12px;font-weight:600;color:var(--text-dim);margin-bottom:4px;}
.modal{position:fixed;top:0;left:0;width:100%;height:100%;background:rgba(0,0,0,0.7);display:none;align-items:center;justify-content:center;z-index:1000;padding:16px;}
.modal-content{background:var(--card);border:1px solid var(--card-border);border-radius:var(--radius);max-width:440px;width:100%;padding:20px;}
.modal-actions{display:flex;justify-content:flex-end;gap:10px;margin-top:16px;}
.log-box{max-height:480px;overflow-y:auto;background:#12161a;border:1px solid var(--card-border);border-radius:6px;padding:8px;font-family:monospace;font-size:12px;}
.log-line{padding:3px 6px;border-radius:4px;display:flex;gap:10px;}
.log-line:hover{background:#182028;}
.log-time{color:var(--text-dim);white-space:nowrap;}
.log-subsys{font-weight:700;width:80px;white-space:nowrap;}
.auth-box{max-width:380px;margin:60px auto;background:var(--card);border:1px solid var(--card-border);border-radius:var(--radius);padding:24px;}
.auth-title{font-size:18px;font-weight:700;color:#fff;text-align:center;margin-bottom:16px;}
.alert{padding:8px 12px;border-radius:6px;font-size:12px;margin-bottom:12px;display:none;}
.alert-danger{background:rgba(239,68,68,0.2);color:var(--danger);border:1px solid rgba(239,68,68,0.4);}
.alert-success{background:rgba(16,185,129,0.2);color:var(--success);border:1px solid rgba(16,185,129,0.4);}
@media(max-width:768px){header{padding:10px 14px;}main{padding:12px;}.grid-4{grid-template-columns:1fr;}}
</style>
</head>
<body>
<div class="trusted-lan-bar">
  <span class="badge">SECURITY NOTICE</span>
  <span>Operating on Trusted Local Network (HTTP Only) — No External Cloud or WAN Exposure</span>
</div>

<div id="emg-banner" class="emg-banner">
  <span id="emg-text">EMERGENCY ACTIVE: ALL RELAYS LOCKED</span>
  <button class="btn btn-secondary" onclick="openEmgResetModal()" style="color:#fff;border:1px solid #fff;">Reset Emergency</button>
</div>

<div id="auth-view" style="display:none;">
  <div class="auth-box">
    <div class="auth-title" id="auth-title">System Login</div>
    <div id="auth-alert" class="alert alert-danger"></div>
    <div class="form-group" id="setup-msg" style="display:none;color:var(--warning);font-size:12px;margin-bottom:12px;">
      Welcome! Please set your administrator username, password, and session timeout to initialize the controller.
    </div>
    <div class="form-group">
      <label class="form-label">Username</label>
      <input type="text" id="auth-user" autocomplete="username">
    </div>
    <div class="form-group">
      <label class="form-label">Password</label>
      <input type="password" id="auth-pass" autocomplete="current-password">
    </div>
    <div class="form-group" id="setup-timeout-group" style="display:none;">
      <label class="form-label">Session Timeout (seconds)</label>
      <input type="number" id="auth-timeout" value="900">
    </div>
    <button class="btn btn-block" id="auth-btn" onclick="submitAuth()">Sign In</button>
  </div>
</div>

<div id="app-view" style="display:none;">
  <header>
    <div class="logo-group">
      <span class="logo-title">WT32-ETH01 Controller</span>
      <span class="status-pill online" id="ws-pill">LIVE WS</span>
    </div>
    <div class="header-meta">
      <div id="header-time-pill" onclick="showTab('timers')" title="Click to open Time & Astro-Clock settings" style="cursor:pointer;background:#161d24;padding:3px 8px;border-radius:4px;border:1px solid var(--border);display:flex;align-items:center;gap:6px;">
        <span id="header-clock-icon">🕒</span>
        <span id="header-clock" style="font-weight:700;font-size:13px;color:#fff;">--:--:--</span>
        <span id="header-clock-sync" class="status-pill online" style="font-size:9px;padding:1px 5px;">SYNC</span>
      </div>
      <span id="header-ip" style="color:var(--text-dim)">IP: ---</span>
      <span id="header-temp" style="color:var(--info)">--.- °C</span>
      <button class="btn btn-secondary" style="padding:4px 8px;font-size:11px;" onclick="logout()">Logout</button>
    </div>
  </header>

  <nav>
    <button class="active" onclick="showTab('dashboard')">Dashboard</button>
    <button onclick="showTab('relays')">Relays</button>
    <button onclick="showTab('inputs')">Inputs</button>
    <button onclick="showTab('interlock')">Interlock</button>
    <button onclick="showTab('timers')">Timers & Astro</button>
    <button onclick="showTab('scenes')">Scenes</button>
    <button onclick="showTab('energy')">⚡ Energy & Health</button>
    <button onclick="showTab('logic')">🔀 Logic Rules</button>
    <button onclick="showTab('watchdog')">Ping Watchdog</button>
    <button onclick="showTab('network')">Network & Wi-Fi</button>
    <button onclick="showTab('mqtt')">MQTT</button>
    <button onclick="showTab('security')">Security</button>
    <button onclick="showTab('logs')">Event Log</button>
    <button onclick="showTab('sysinfo')">System Info</button>
    <button onclick="showTab('maintenance')">Maintenance</button>
  </nav>


  <main>
    <!-- DASHBOARD -->
    <div id="tab-dashboard" class="tab-content active">
      <div class="summary-bar">
        <div class="summary-item"><div class="summary-label">Relays Active</div><div class="summary-value" id="sum-relays">0 / 16</div></div>
        <div class="summary-item"><div class="summary-label">Live Power</div><div class="summary-value" id="sum-power">0 W</div></div>
        <div class="summary-item"><div class="summary-label">Sun Times</div><div class="summary-value" id="sum-sun" style="font-size:12px;">🌅 --:-- | 🌇 --:--</div></div>
        <div class="summary-item"><div class="summary-label">Inputs Active</div><div class="summary-value" id="sum-inputs">0 / 16</div></div>
        <div class="summary-item"><div class="summary-label">Ethernet</div><div class="summary-value" id="sum-eth" style="font-size:13px;">Connected</div></div>
        <div class="summary-item"><div class="summary-label">MQTT Status</div><div class="summary-value" id="sum-mqtt" style="font-size:13px;">Disabled</div></div>
        <div class="summary-item"><div class="summary-label">Uptime</div><div class="summary-value" id="sum-uptime" style="font-size:13px;">0s</div></div>
      </div>

      <div style="margin-bottom:12px;display:flex;justify-content:space-between;align-items:center;flex-wrap:wrap;gap:8px;">
        <div style="display:flex;align-items:center;gap:8px;">
          <h3 style="font-size:15px;color:#fff;">Relays (1-16)</h3>
          <div id="room-filter-bar" style="display:flex;gap:4px;flex-wrap:wrap;"></div>
        </div>
        <div style="display:flex;gap:8px;">
          <button class="btn btn-secondary" style="padding:4px 10px;font-size:11px;" onclick="allRelays(false)">All OFF</button>
          <button class="btn btn-secondary" style="padding:4px 10px;font-size:11px;" onclick="allRelays(true)">All ON</button>
        </div>
      </div>
      <div class="grid-4" id="relay-cards-grid" style="margin-bottom:20px;"></div>


      <h3 style="font-size:15px;color:#fff;margin-bottom:12px;">Inputs (1-16)</h3>
      <div class="grid-4" id="input-cards-grid" style="margin-bottom:20px;"></div>

      <div class="card">
        <div class="card-title">Recent Event Log <button class="btn btn-secondary" style="padding:2px 8px;font-size:11px;" onclick="showTab('logs')">View All</button></div>
        <div class="log-box" id="dashboard-recent-logs" style="max-height:160px;"></div>
      </div>
    </div>

    <!-- RELAY SETTINGS -->
    <div id="tab-relays" class="tab-content">
      <div class="card">
        <div class="card-title">Relay Configuration (1-16)</div>
        <div style="overflow-x:auto;">
          <table id="table-relay-cfg">
            <thead>
              <tr>
                <th>#</th><th>Icon</th><th>Name</th><th>Room</th><th>Power (W)</th><th>Enabled</th><th>Restore Mode</th><th>Auto-Off (s)</th>
                <th>On Delay (ms)</th><th>Off Delay (ms)</th><th>Interlock Grp</th><th>Manual</th><th>MQTT</th><th>Input</th><th>Log</th><th>Action</th>
              </tr>
            </thead>
            <tbody id="tbody-relay-cfg"></tbody>
          </table>
        </div>
      </div>
    </div>

    <!-- INPUT SETTINGS -->
    <div id="tab-inputs" class="tab-content">
      <div class="card" style="margin-bottom:16px;">
        <div class="card-title">Global Input Defaults</div>
        <div class="grid-4" style="margin-bottom:12px;">
          <div class="form-group"><label class="form-label">Debounce (ms)</label><input type="number" id="gdef-deb"></div>
          <div class="form-group"><label class="form-label">Long Press (ms)</label><input type="number" id="gdef-long"></div>
          <div class="form-group"><label class="form-label">Double Click Window (ms)</label><input type="number" id="gdef-dbl"></div>
          <div class="form-group"><label class="form-label">Signal Timeout (s, 0=Off)</label><input type="number" id="gdef-tout"></div>
          <div class="form-group"><label class="form-label">Timeout Action</label>
            <select id="gdef-tact"><option value="0">Warning Only</option><option value="1">Auto-Release</option><option value="2">Emergency</option></select>
          </div>
        </div>
        <div style="display:flex;gap:10px;">
          <button class="btn" onclick="saveGlobalDefaults()">Save Defaults</button>
          <button class="btn btn-secondary" onclick="applyDefaultsToAllPrompt()">Apply Defaults To All Inputs</button>
        </div>
      </div>

      <div class="card">
        <div class="card-title">
          <span>Input Assignment & Configuration (1-16)</span>
          <span style="font-size:12px;color:var(--text-dim);font-weight:400;">Any input can trigger any relay of your choice</span>
        </div>
        <div class="grid-2" id="inputs-cards-container"></div>
      </div>
    </div>

    <!-- INTERLOCK -->
    <div id="tab-interlock" class="tab-content">
      <div class="card" style="margin-bottom:16px;">
        <div class="card-title">Interlock Subsystem Rules</div>
        <p style="color:var(--text-dim);font-size:13px;line-height:1.6;">
          Interlock groups 1 through 8 strictly enforce that <strong>at most one relay in each group can be ON at any time</strong>.
          Turning any relay ON automatically turns OFF any other active relay in the same interlock group.
        </p>
      </div>

      <div class="card" style="margin-bottom:16px;">
        <div class="card-title">
          <span>Relay Interlock Matrix</span>
          <button class="btn btn-success" style="padding:4px 12px;font-size:12px;" onclick="saveInterlockMatrix()">Save Interlock Matrix</button>
        </div>
        <div style="display:grid;grid-template-columns:repeat(auto-fill,minmax(200px,1fr));gap:10px;" id="interlock-matrix-grid"></div>
      </div>

      <div class="card">
        <div class="card-title">Active Interlock Groups Status (1-8)</div>
        <div class="grid-4" id="interlock-groups-grid"></div>
      </div>
    </div>

    <!-- TIMERS & ASTRO -->
    <div id="tab-timers" class="tab-content">
      <!-- TIMEZONE & ASTRO CONFIGURATION -->
      <div class="card" style="margin-bottom:16px;">
        <div class="card-title">
          <span>🕒 System Time, Timezone & Astro-Clock Configuration</span>
          <div style="display:flex;align-items:center;gap:8px;">
            <button class="btn btn-primary" style="padding:4px 10px;font-size:11px;" onclick="syncBrowserTime()">📱 Sync with Browser Time</button>
            <button class="btn btn-secondary" style="padding:4px 10px;font-size:11px;" onclick="resyncNtp()">🌐 Sync NTP</button>
          </div>
        </div>

        <!-- Solar Overview Metrics Bar -->
        <div class="summary-bar" style="margin-bottom:16px;background:#0d1115;border:1px solid #1a222c;">
          <div class="summary-item"><div class="summary-label">Current Clock</div><div class="summary-value" id="time-current-display" style="color:var(--info);">--:--:--</div></div>
          <div class="summary-item"><div class="summary-label">🌅 Sunrise</div><div class="summary-value" id="astro-sunrise">--:--</div></div>
          <div class="summary-item"><div class="summary-label">🌇 Sunset</div><div class="summary-value" id="astro-sunset">--:--</div></div>
          <div class="summary-item"><div class="summary-label">☀️ Solar Noon</div><div class="summary-value" id="astro-noon">--:--</div></div>
          <div class="summary-item"><div class="summary-label">⏳ Day Length</div><div class="summary-value" id="astro-length">--:--</div></div>
          <div class="summary-item"><div class="summary-label">Phase</div><div class="summary-value" id="astro-phase"><span class="status-pill online">--</span></div></div>
        </div>

        <div class="grid-2" style="gap:16px;margin-bottom:16px;">
          <!-- Left: Timezone & Location -->
          <div style="background:#161d24;padding:12px;border-radius:6px;border:1px solid var(--border);">
            <div style="font-weight:700;font-size:13px;color:#fff;margin-bottom:10px;">🌍 Timezone & City Preset</div>
            <div class="form-group">
              <label class="form-label">Regional Preset (Auto-fills Coordinates & Offset)</label>
              <select id="time-preset" onchange="onPresetChange(this.value)">
                <option value="am_yerevan">🇦🇲 Armenia - Yerevan, Gyumri (UTC+4)</option>
                <option value="am_artsakh">🇦🇲 Artsakh - Stepanakert (UTC+4)</option>
                <option value="ge_tbilisi">🇬🇪 Georgia - Tbilisi (UTC+4)</option>
                <option value="ru_moscow">🇷🇺 Russia - Moscow (UTC+3)</option>
                <option value="ru_krasnodar">🇷🇺 Russia - Krasnodar / Sochi (UTC+3)</option>
                <option value="ae_dubai">🇦🇪 UAE - Dubai (UTC+4)</option>
                <option value="eu_berlin">🇩🇪 Germany / France / Italy (UTC+1)</option>
                <option value="uk_london">🇬🇧 UK - London (UTC+0)</option>
                <option value="us_ny">🇺🇸 USA - New York (UTC-5)</option>
                <option value="us_la">🇺🇸 USA - Los Angeles (UTC-8)</option>
                <option value="custom">⚙️ Custom Timezone & Coordinates</option>
              </select>
            </div>

            <div style="display:grid;grid-template-columns:1fr 1fr;gap:10px;">
              <div class="form-group">
                <label class="form-label">Latitude (°N)</label>
                <input type="number" id="time-lat" step="0.0001" placeholder="40.18">
              </div>
              <div class="form-group">
                <label class="form-label">Longitude (°E)</label>
                <input type="number" id="time-lon" step="0.0001" placeholder="44.51">
              </div>
            </div>
            <button class="btn btn-secondary btn-block" style="padding:4px;font-size:11px;margin-bottom:10px;" onclick="getBrowserGps()">
              📍 Get Coordinates from Phone / Browser GPS
            </button>

            <div style="display:grid;grid-template-columns:1fr 1fr;gap:10px;">
              <div class="form-group">
                <label class="form-label">Timezone (GMT Offset)</label>
                <select id="time-gmt">
                  <option value="-43200">UTC -12:00</option>
                  <option value="-39600">UTC -11:00</option>
                  <option value="-36000">UTC -10:00 (Hawaii)</option>
                  <option value="-32400">UTC -09:00 (Alaska)</option>
                  <option value="-28800">UTC -08:00 (PST)</option>
                  <option value="-25200">UTC -07:00 (MST)</option>
                  <option value="-21600">UTC -06:00 (CST)</option>
                  <option value="-18000">UTC -05:00 (EST)</option>
                  <option value="-14400">UTC -04:00</option>
                  <option value="-10800">UTC -03:00 (Argentina)</option>
                  <option value="-7200">UTC -02:00</option>
                  <option value="-3600">UTC -01:00</option>
                  <option value="0">UTC +00:00 (London, GMT)</option>
                  <option value="3600">UTC +01:00 (CET - Berlin, Paris)</option>
                  <option value="7200">UTC +02:00 (EET - Athens, Kyiv)</option>
                  <option value="10800">UTC +03:00 (MSK - Moscow)</option>
                  <option value="14400" selected>UTC +04:00 (AMT - Yerevan, Tbilisi, Dubai)</option>
                  <option value="16200">UTC +04:30 (Iran)</option>
                  <option value="18000">UTC +05:00 (Pakistan)</option>
                  <option value="19800">UTC +05:30 (India)</option>
                  <option value="21600">UTC +06:00</option>
                  <option value="25200">UTC +07:00 (Bangkok)</option>
                  <option value="28800">UTC +08:00 (Beijing, Singapore)</option>
                  <option value="32400">UTC +09:00 (Tokyo)</option>
                  <option value="36000">UTC +10:00 (Sydney)</option>
                  <option value="39600">UTC +11:00</option>
                  <option value="43200">UTC +12:00 (Auckland)</option>
                </select>
              </div>
              <div class="form-group">
                <label class="form-label">Daylight Saving (DST)</label>
                <select id="time-dst">
                  <option value="0">Disabled (0h)</option>
                  <option value="3600">+1 Hour Summer Time</option>
                </select>
              </div>
            </div>
          </div>

          <!-- Right: NTP Server & Offline Manual Set -->
          <div style="background:#161d24;padding:12px;border-radius:6px;border:1px solid var(--border);display:flex;flex-direction:column;justify-content:space-between;">
            <div>
              <div style="font-weight:700;font-size:13px;color:#fff;margin-bottom:10px;">🌐 Internet NTP Synchronization</div>
              <div class="form-group">
                <label class="form-label">NTP Server Hostname</label>
                <input type="text" id="time-ntp" placeholder="pool.ntp.org">
              </div>
              <p style="font-size:11px;color:var(--text-dim);line-height:1.5;">
                When connected to LAN with internet, the controller automatically synchronizes time via NTP every hour and calculates exact daily sunrise and sunset for your coordinates.
              </p>
            </div>

            <div style="margin-top:10px;padding-top:10px;border-top:1px solid var(--border);">
              <div style="font-weight:700;font-size:12px;color:var(--info);margin-bottom:6px;">✍️ Manual Clock Setting (Offline Mode)</div>
              <div style="display:flex;gap:8px;">
                <input type="datetime-local" id="time-manual-input" style="flex:1;font-size:12px;">
                <button class="btn btn-secondary" style="padding:4px 10px;font-size:11px;" onclick="applyManualTime()">Set Time</button>
              </div>
            </div>
          </div>
        </div>

        <div style="display:flex;justify-content:flex-end;gap:10px;">
          <button class="btn btn-success" style="padding:6px 18px;" onclick="saveTimeSettings()">Save Time & Location</button>
        </div>
      </div>

      <div class="card" style="margin-bottom:16px;">
        <div class="card-title">Relay Auto-Off & Delay Timers</div>
        <table id="table-timers">
          <thead>
            <tr><th>Relay</th><th>State</th><th>Auto-Off Setting</th><th>Active Auto-Off Remaining</th><th>On Delay</th><th>Off Delay</th></tr>
          </thead>
          <tbody id="tbody-timers"></tbody>
        </table>
      </div>

      <div class="card">
        <div class="card-title">
          <span>Scheduled Timers (NTP Automation)</span>
          <div style="display:flex;align-items:center;gap:12px;">
            <span style="font-size:12px;color:var(--text-dim);">Clock: <strong id="ntp-clock-display" style="color:var(--info);">--:--:--</strong></span>
            <button class="btn btn-secondary" style="padding:4px 8px;font-size:11px;" onclick="loadSchedules()">Refresh</button>
          </div>
        </div>
        <div id="schedules-grid"></div>
      </div>
    </div>

    <!-- SCENES -->
    <div id="tab-scenes" class="tab-content">
      <div class="grid-2" id="scenes-grid"></div>
    </div>

    <!-- ENERGY & HEALTH -->
    <div id="tab-energy" class="tab-content">
      <div class="summary-bar" style="margin-bottom:16px;">
        <div class="summary-item"><div class="summary-label">Total System Load</div><div class="summary-value" id="energy-total-power">0 W</div></div>
        <div class="summary-item"><div class="summary-label">Cumulative Energy</div><div class="summary-value" id="energy-total-kwh" style="color:var(--success);">0.000 kWh</div></div>
        <div class="summary-item"><div class="summary-label">Active Relays</div><div class="summary-value" id="energy-active-count">0 / 16</div></div>
        <div class="summary-item"><div class="summary-label">Actions</div><div class="summary-value"><button class="btn btn-danger" style="padding:4px 10px;font-size:11px;" onclick="resetAllEnergy()">Reset All Odometer</button></div></div>
      </div>

      <div class="card">
        <div class="card-title">
          <span>Relay Cycle Odometer & Energy Estimation</span>
          <button class="btn btn-secondary" style="padding:4px 8px;font-size:11px;" onclick="loadEnergyStats()">Refresh</button>
        </div>
        <div style="overflow-x:auto;">
          <table id="table-energy">
            <thead>
              <tr>
                <th>#</th><th>Relay</th><th>Room</th><th>Rated Power (W)</th><th>State</th>
                <th>Cycles</th><th>Health</th><th>Total On-Time</th><th>Energy (kWh)</th><th>Action</th>
              </tr>
            </thead>
            <tbody id="tbody-energy"></tbody>
          </table>
        </div>
      </div>
    </div>

    <!-- LOGIC RULES -->
    <div id="tab-logic" class="tab-content">
      <div class="card" style="margin-bottom:16px;">
        <div class="card-title">
          <span>Automation IF-THEN Rules Engine</span>
          <button class="btn btn-secondary" style="padding:4px 8px;font-size:11px;" onclick="loadLogicRules()">Refresh Rules</button>
        </div>
        <p style="color:var(--text-dim);font-size:13px;line-height:1.6;margin-bottom:8px;">
          Define up to 8 standalone event-driven automation rules. Evaluated in real-time every 50ms with edge-detection transitions.
          Combine physical inputs, relay states, and solar day/night events.
        </p>
      </div>

      <div id="logic-rules-grid" class="grid-2"></div>
    </div>


    <!-- NETWORK -->
    <div id="tab-network" class="tab-content">
      <div class="grid-2">
        <div class="card">
          <div class="card-title">Wired Ethernet (LAN8720)</div>
          <div id="net-alert" class="alert alert-success"></div>
          <div class="form-group"><label class="form-label">Hostname</label><input type="text" id="net-host"></div>
          <div class="form-group"><label class="form-label">Addressing Mode</label>
            <select id="net-dhcp" onchange="toggleNetFields()"><option value="1">DHCP (Automatic)</option><option value="0">Static IP</option></select>
          </div>
          <div id="net-static-group">
            <div class="form-group"><label class="form-label">Static IP Address</label><input type="text" id="net-ip"></div>
            <div class="form-group"><label class="form-label">Gateway</label><input type="text" id="net-gw"></div>
            <div class="form-group"><label class="form-label">Subnet Mask</label><input type="text" id="net-mask"></div>
            <div class="form-group"><label class="form-label">Primary DNS</label><input type="text" id="net-dns"></div>
          </div>
          <button class="btn" onclick="saveNetworkCfg()">Save Network Settings</button>
        </div>

        <div class="card">
          <div class="card-title">
            <span>Wi-Fi Redundancy & Hotspot</span>
            <span id="wifi-status-badge" class="status-pill offline">Idle</span>
          </div>
          <div id="wifi-alert" class="alert alert-success"></div>
          
          <h4 style="font-size:13px;color:var(--text-dim);margin:10px 0 6px;">Wi-Fi Hot-Standby Client (Failover)</h4>
          <div class="form-group"><label class="form-label">Wi-Fi Failover Mode</label>
            <select id="wifi-sta-en"><option value="0">Disabled</option><option value="1">Enabled (Hot-Standby if LAN drops)</option></select>
          </div>
          <div class="form-group">
            <div style="display:flex;justify-content:space-between;align-items:center;margin-bottom:4px;">
              <label class="form-label" style="margin:0;">Network SSID</label>
              <button class="btn btn-secondary" style="padding:2px 8px;font-size:11px;" onclick="scanWifiNetworks()">Scan Wi-Fi</button>
            </div>
            <input type="text" id="wifi-sta-ssid" placeholder="Enter or select SSID">
            <div id="wifi-scan-list" style="margin-top:6px;max-height:120px;overflow-y:auto;display:none;background:var(--bg-card);border:1px solid var(--border);border-radius:4px;padding:4px;"></div>
          </div>
          <div class="form-group"><label class="form-label">Network Password</label><input type="password" id="wifi-sta-pass" placeholder="Leave blank to keep unchanged"></div>

          <hr style="border:0;border-top:1px solid var(--border);margin:14px 0;">

          <h4 style="font-size:13px;color:var(--text-dim);margin:10px 0 6px;">Wi-Fi Setup Hotspot (Fallback AP)</h4>
          <div class="form-group"><label class="form-label">Fallback Hotspot AP</label>
            <select id="wifi-ap-fb"><option value="1">Enabled (Active if unconfigured or LAN lost)</option><option value="0">Disabled</option></select>
          </div>
          <div class="form-group"><label class="form-label">Hotspot AP SSID</label><input type="text" id="wifi-ap-ssid" placeholder="WT32-Relay-Setup"></div>
          <div class="form-group"><label class="form-label">Hotspot Password (min 8 chars, or blank for open)</label><input type="password" id="wifi-ap-pass" placeholder="Leave blank to keep unchanged"></div>

          <button class="btn btn-success" onclick="saveWifiCfg()">Save Wi-Fi Settings</button>
        </div>
      </div>
    </div>

    <!-- MQTT -->
    <div id="tab-mqtt" class="tab-content">
      <div class="card" style="max-width:540px;">
        <div class="card-title">MQTT & Home Assistant Configuration</div>
        <div id="mqtt-alert" class="alert alert-success"></div>
        <div class="form-group"><label class="form-label">MQTT Client State</label>
          <select id="mqtt-en"><option value="0">Disabled</option><option value="1">Enabled</option></select>
        </div>
        <div class="form-group"><label class="form-label">Connection Security</label>
          <select id="mqtt-ssl" onchange="toggleMqttSsl()">
            <option value="0">Standard TCP (Port 1883)</option>
            <option value="1">SSL / TLS Encrypted (MQTTS, Port 8883)</option>
          </select>
        </div>
        <div class="form-group" id="mqtt-ssl-mode-group" style="display:none;">
          <label class="form-label">SSL Certificate Verification</label>
          <select id="mqtt-ssl-mode">
            <option value="0">TLS Encrypted (No CA check / Any Cloud Broker / Self-Signed)</option>
            <option value="1">Let's Encrypt ISRG Root X1 (Strict Verify - HiveMQ Cloud, Public CA)</option>
            <option value="2">Amazon Root CA 1 (Strict Verify - AWS IoT Core)</option>
          </select>
        </div>
        <div class="form-group"><label class="form-label">Broker Host / IP</label><input type="text" id="mqtt-host"></div>
        <div class="form-group"><label class="form-label">Broker Port</label><input type="number" id="mqtt-port"></div>
        <div class="form-group"><label class="form-label">Username (optional)</label><input type="text" id="mqtt-user"></div>
        <div class="form-group"><label class="form-label">Password (optional)</label><input type="password" id="mqtt-pass"></div>
        <div class="form-group"><label class="form-label">Client ID</label><input type="text" id="mqtt-cid"></div>
        <div class="form-group"><label class="form-label">Base Topic</label><input type="text" id="mqtt-topic"></div>
        <div class="form-group"><label class="form-label">Home Assistant Auto-Discovery</label>
          <select id="mqtt-ha"><option value="1">Enabled (auto-publish discovery topics)</option><option value="0">Disabled</option></select>
        </div>
        <button class="btn" onclick="saveMqttCfg()">Save MQTT Settings</button>
      </div>
    </div>

    <!-- WATCHDOG -->
    <div id="tab-watchdog" class="tab-content">
      <div class="grid-2">
        <div class="card">
          <div class="card-title">Ping Watchdog (Router Auto-Reboot)</div>
          <div id="wd-alert" class="alert alert-success"></div>
          <p style="color:var(--text-dim);font-size:13px;line-height:1.5;margin-bottom:12px;">
            Continuously monitors Internet or router connectivity by pinging a remote host (e.g. <code>8.8.8.8</code> or <code>1.1.1.1</code>). If consecutive failures occur, it automatically power-cycles the designated router relay to restore connectivity.
          </p>

          <div class="form-group"><label class="form-label">Watchdog State</label>
            <select id="wd-en"><option value="0">Disabled</option><option value="1">Enabled (Active)</option></select>
          </div>
          <div class="grid-2" style="gap:10px;">
            <div class="form-group"><label class="form-label">Target Host / IP</label><input type="text" id="wd-host" placeholder="8.8.8.8"></div>
            <div class="form-group"><label class="form-label">Port</label><input type="number" id="wd-port" placeholder="53"></div>
          </div>
          <div class="grid-2" style="gap:10px;">
            <div class="form-group"><label class="form-label">Check Interval (sec)</label><input type="number" id="wd-interval" placeholder="60"></div>
            <div class="form-group"><label class="form-label">Failure Threshold</label><input type="number" id="wd-fails" placeholder="3"></div>
          </div>
          <div class="form-group"><label class="form-label">Target Router Relay to Power-Cycle</label>
            <select id="wd-relay"></select>
          </div>
          <div class="grid-2" style="gap:10px;">
            <div class="form-group"><label class="form-label">Power OFF Duration (sec)</label><input type="number" id="wd-off" placeholder="10"></div>
            <div class="form-group"><label class="form-label">Boot Cooldown (sec)</label><input type="number" id="wd-cooldown" placeholder="300"></div>
          </div>

          <div style="display:flex;gap:10px;margin-top:10px;">
            <button class="btn btn-primary" onclick="saveWatchdogCfg()">Save Watchdog Settings</button>
            <button class="btn btn-secondary" onclick="testWatchdogPing()">Test Ping Now</button>
          </div>
        </div>

        <div class="card">
          <div class="card-title">Live Watchdog Monitor</div>
          <div style="display:grid;grid-template-columns:1fr 1fr;gap:12px;margin-bottom:16px;">
            <div style="background:var(--bg);border:1px solid var(--border);border-radius:6px;padding:12px;">
              <div style="font-size:11px;color:var(--text-dim);text-transform:uppercase;">Watchdog Status</div>
              <div id="wd-mon-status" style="font-size:16px;font-weight:600;margin-top:4px;color:var(--info);">Idle</div>
            </div>
            <div style="background:var(--bg);border:1px solid var(--border);border-radius:6px;padding:12px;">
              <div style="font-size:11px;color:var(--text-dim);text-transform:uppercase;">Consecutive Fails</div>
              <div id="wd-mon-fails" style="font-size:16px;font-weight:600;margin-top:4px;">0 / 3</div>
            </div>
            <div style="background:var(--bg);border:1px solid var(--border);border-radius:6px;padding:12px;">
              <div style="font-size:11px;color:var(--text-dim);text-transform:uppercase;">Total Router Reboots</div>
              <div id="wd-mon-reboots" style="font-size:16px;font-weight:600;margin-top:4px;color:var(--warning);">0</div>
            </div>
            <div style="background:var(--bg);border:1px solid var(--border);border-radius:6px;padding:12px;">
              <div style="font-size:11px;color:var(--text-dim);text-transform:uppercase;">Target Destination</div>
              <div id="wd-mon-target" style="font-size:14px;font-weight:600;margin-top:4px;">8.8.8.8:53</div>
            </div>
          </div>
          <div id="wd-test-result" style="display:none;" class="alert"></div>
        </div>
      </div>
    </div>

    <!-- SECURITY -->
    <div id="tab-security" class="tab-content">
      <div class="grid-2">
        <div class="card">
          <div class="card-title">Change Password</div>
          <div id="sec-alert" class="alert alert-success"></div>
          <div class="form-group"><label class="form-label">Current Password</label><input type="password" id="sec-old-pass"></div>
          <div class="form-group"><label class="form-label">New Username</label><input type="text" id="sec-new-user"></div>
          <div class="form-group"><label class="form-label">New Password</label><input type="password" id="sec-new-pass"></div>
          <button class="btn" onclick="changePassword()">Update Credentials</button>
        </div>
        <div class="card">
          <div class="card-title">Security & Session Parameters</div>
          <div id="sec-pol-alert" class="alert alert-success"></div>
          <div class="form-group"><label class="form-label">Session Timeout (seconds)</label><input type="number" id="sec-timeout"></div>
          <div class="form-group"><label class="form-label">Max Failed Attempts (Lockout)</label><input type="number" id="sec-lock-att"></div>
          <div class="form-group"><label class="form-label">Lockout Duration (seconds)</label><input type="number" id="sec-lock-time"></div>
          <button class="btn" onclick="saveSecurityPolicy()">Save Policy</button>
        </div>
      </div>
    </div>

    <!-- EVENT LOG -->
    <div id="tab-logs" class="tab-content">
      <div class="card">
        <div class="card-title">
          <span>System Event Log (Ring Buffer)</span>
          <div style="display:flex;gap:8px;">
            <select id="log-filter" onchange="loadEventLogs()" style="width:130px;padding:4px 8px;font-size:12px;">
              <option value="-1">All Subsystems</option>
              <option value="0">System</option><option value="1">Relays</option><option value="2">Inputs</option>
              <option value="3">Emergency</option><option value="4">Network</option><option value="5">MQTT</option>
              <option value="6">Security</option>
            </select>
            <button class="btn btn-secondary" style="padding:4px 8px;font-size:11px;" onclick="loadEventLogs()">Refresh</button>
            <button class="btn btn-danger" style="padding:4px 8px;font-size:11px;" onclick="clearLogs()">Clear Log</button>
          </div>
        </div>
        <div class="log-box" id="full-log-box" style="height:500px;"></div>
      </div>
    </div>

    <!-- SYSTEM INFO -->
    <div id="tab-sysinfo" class="tab-content">
      <div class="card" style="max-width:640px;">
        <div class="card-title">Device & Hardware Information</div>
        <table>
          <tbody id="tbody-sysinfo"></tbody>
        </table>
      </div>
    </div>

    <!-- MAINTENANCE -->
    <div id="tab-maintenance" class="tab-content">
      <div class="grid-2">
        <div class="card">
          <div class="card-title">Firmware Over-The-Air (OTA) Update</div>
          <p style="color:var(--text-dim);font-size:13px;margin-bottom:12px;">Upload a new <code>firmware.bin</code> file directly over the network without any USB/UART wires.</p>
          <div id="ota-alert" class="alert alert-success" style="display:none;"></div>
          <div class="form-group">
            <input type="file" id="ota-file" accept=".bin" style="border:1px dashed var(--border);padding:10px;width:100%;cursor:pointer;">
          </div>
          <div id="ota-prog-wrap" style="display:none;margin-bottom:12px;">
            <div style="display:flex;justify-content:space-between;font-size:12px;margin-bottom:4px;">
              <span id="ota-status">Uploading...</span>
              <span id="ota-pct">0%</span>
            </div>
            <div style="background:#222;height:8px;border-radius:4px;overflow:hidden;">
              <div id="ota-bar" style="background:var(--primary);width:0%;height:100%;transition:width 0.2s;"></div>
            </div>
          </div>
          <button class="btn btn-warning" id="ota-btn" onclick="startOtaUpload()">Upload & Flash Firmware</button>
        </div>

        <div class="card">
          <div class="card-title">Configuration Backup & Restore</div>
          <p style="color:var(--text-dim);font-size:13px;margin-bottom:12px;">Export all relay, input, schedule, and network settings to a JSON file, or restore configuration from a backup.</p>
          <div id="bak-alert" class="alert alert-success" style="display:none;"></div>
          <div style="margin-bottom:14px;">
            <button class="btn" onclick="downloadBackup()">Download Backup (JSON)</button>
          </div>
          <div class="form-group">
            <label class="form-label">Restore from JSON file</label>
            <input type="file" id="restore-file" accept=".json" style="border:1px dashed var(--border);padding:10px;width:100%;cursor:pointer;">
          </div>
          <button class="btn btn-secondary" onclick="restoreBackup()">Restore Configuration</button>
        </div>

        <div class="card">
          <div class="card-title">Reboot Device</div>
          <p style="color:var(--text-dim);font-size:13px;margin-bottom:12px;">Restart the ESP32 microcontroller cleanly. Relay states configured with "Restore previous" will be resumed.</p>
          <button class="btn btn-secondary" onclick="confirmReboot()">Reboot Controller</button>
        </div>

        <div class="card">
          <div class="card-title" style="color:var(--danger)">Factory Reset</div>
          <p style="color:var(--text-dim);font-size:13px;margin-bottom:12px;">Erases all persistent settings, credentials, relay/input configurations, and emergency states in NVS. Returns the board to first-run setup mode.</p>
          <button class="btn btn-danger" onclick="confirmFactoryReset()">Factory Reset</button>
        </div>
      </div>
    </div>
  </main>
</div>

<!-- EMERGENCY RESET MODAL -->
<div id="emg-modal" class="modal">
  <div class="modal-content">
    <div class="card-title" style="color:var(--danger);">Reset Emergency State</div>
    <p style="font-size:13px;margin-bottom:12px;color:var(--text);">Step 1: Confirm that all physical conditions triggered by emergency have been safely resolved.</p>
    <div class="form-group">
      <label style="display:flex;align-items:center;gap:8px;font-size:13px;cursor:pointer;">
        <input type="checkbox" id="emg-chk-1" style="width:auto;"> I confirm the physical hazards are resolved.
      </label>
    </div>
    <div class="form-group">
      <label style="display:flex;align-items:center;gap:8px;font-size:13px;cursor:pointer;">
        <input type="checkbox" id="emg-chk-2" style="width:auto;"> I authorize unlocking relay outputs.
      </label>
    </div>
    <div class="modal-actions">
      <button class="btn btn-secondary" onclick="closeEmgModal()">Cancel</button>
      <button class="btn btn-danger" id="emg-reset-confirm-btn" onclick="submitEmgReset()">Confirm Reset</button>
    </div>
  </div>
</div>

<script>
let sessionToken = localStorage.getItem("wt32_session") || "";
let ws = null;
let pollTimer = null;
let sysState = null;
let relayConfigsCache = [];
let currentRoomFilter = "";
const DEVICE_ICONS = ["💡", "🔌", "🚪", "♨️", "🚰", "❄️", "⚡"];

function showTab(name) {
  document.querySelectorAll(".tab-content").forEach(el => el.classList.remove("active"));
  document.querySelectorAll("nav button").forEach(el => el.classList.remove("active"));
  const tab = document.getElementById("tab-" + name);
  if (tab) tab.classList.add("active");
  const btn = Array.from(document.querySelectorAll("nav button")).find(b => b.innerText.toLowerCase().includes(name.toLowerCase()));
  if (btn) btn.classList.add("active");

  if (name === 'relays') loadRelayConfigs();
  if (name === 'inputs') loadInputConfigs();
  if (name === 'interlock') loadInterlockMatrix();
  if (name === 'timers') { renderTimers(); loadSchedules(); loadTimeConfig(); }
  if (name === 'scenes') loadSceneConfigs();
  if (name === 'energy') loadEnergyStats();
  if (name === 'logic') loadLogicRules();
  if (name === 'network') { loadNetworkCfg(); loadWifiCfg(); }
  if (name === 'mqtt') loadMqttCfg();
  if (name === 'watchdog') loadWatchdogCfg();
  if (name === 'security') loadSecurityCfg();
  if (name === 'logs') loadEventLogs();
  if (name === 'sysinfo') loadSysInfo();
}

async function api(path, method="GET", body=null) {
  let url = path;
  if (sessionToken) {
    url += (url.includes('?') ? '&' : '?') + 'token=' + encodeURIComponent(sessionToken);
  }
  const headers = {
    "X-Session": sessionToken,
    "Authorization": "Bearer " + sessionToken
  };
  if (body) headers["Content-Type"] = "application/json";
  try {
    const res = await fetch(url, { method, headers, body: body ? JSON.stringify(body) : null });
    if (res.status === 401) {
      localStorage.removeItem("wt32_session");
      sessionToken = "";
      showAuth(false);
      return null;
    }
    return await res.json();
  } catch (e) {
    console.error("API error", e);
    return null;
  }
}

async function checkInit() {
  const data = await api("/api/status");
  if (!data) return;
  if (!data.configured) {
    showAuth(true);
  } else if (!data.authenticated) {
    showAuth(false);
  } else {
    showApp();
    handleStateUpdate(data);
    initWebSocket();
    loadTimeConfig();
  }
}

function showAuth(isSetup) {
  document.getElementById("app-view").style.display = "none";
  document.getElementById("auth-view").style.display = "block";
  document.getElementById("auth-title").innerText = isSetup ? "First-Run Setup" : "System Login";
  document.getElementById("setup-msg").style.display = isSetup ? "block" : "none";
  document.getElementById("setup-timeout-group").style.display = isSetup ? "block" : "none";
  document.getElementById("auth-btn").innerText = isSetup ? "Complete Setup" : "Sign In";
  document.getElementById("auth-alert").style.display = "none";
}

function showApp() {
  document.getElementById("auth-view").style.display = "none";
  document.getElementById("app-view").style.display = "block";
}

async function submitAuth() {
  const isSetup = document.getElementById("setup-msg").style.display === "block";
  const user = document.getElementById("auth-user").value.trim();
  const pass = document.getElementById("auth-pass").value;
  const timeout = parseInt(document.getElementById("auth-timeout").value) || 900;
  const alertEl = document.getElementById("auth-alert");

  if (!user || !pass) {
    alertEl.innerText = "Please enter username and password";
    alertEl.style.display = "block";
    return;
  }

  const endpoint = isSetup ? "/api/setup" : "/api/login";
  const payload = isSetup ? { user, pass, timeout } : { user, pass };

  const res = await fetch(endpoint, {
    method: "POST",
    headers: { "Content-Type": "application/json" },
    body: JSON.stringify(payload)
  });
  const data = await res.json();

  if (res.ok && data.success) {
    if (isSetup) {
      alertEl.className = "alert alert-success";
      alertEl.innerText = "Setup completed. Please log in.";
      alertEl.style.display = "block";
      setTimeout(() => showAuth(false), 1200);
    } else {
      sessionToken = data.token;
      localStorage.setItem("wt32_session", sessionToken);
      showApp();
      initWebSocket();
      startPolling();
    }
  } else {
    alertEl.className = "alert alert-danger";
    alertEl.innerText = data.error || "Authentication failed";
    alertEl.style.display = "block";
  }
}

async function logout() {
  await api("/api/logout", "POST");
  sessionToken = "";
  localStorage.removeItem("wt32_session");
  if (ws) ws.close();
  showAuth(false);
}

function initWebSocket() {
  if (ws) ws.close();
  const host = window.location.hostname;
  ws = new WebSocket("ws://" + host + ":81");

  ws.onopen = () => {
    document.getElementById("ws-pill").innerText = "LIVE WS";
    document.getElementById("ws-pill").className = "status-pill online";
    ws.send(JSON.stringify({ type: "auth", token: sessionToken }));
  };

  ws.onmessage = (evt) => {
    try {
      const data = JSON.parse(evt.data);
      if (data.type === "state") {
        handleStateUpdate(data);
      }
    } catch (e) {
      console.error("WS parse error", e);
    }
  };

  ws.onclose = () => {
    document.getElementById("ws-pill").innerText = "POLLING";
    document.getElementById("ws-pill").className = "status-pill polling";
    setTimeout(initWebSocket, 4000);
  };
}

function startPolling() {
  if (pollTimer) clearInterval(pollTimer);
  pollTimer = setInterval(async () => {
    if (!ws || ws.readyState !== WebSocket.OPEN) {
      const data = await api("/api/status");
      if (data) handleStateUpdate(data);
    }
  }, 2000);
}

function handleStateUpdate(data) {
  sysState = data;
  document.getElementById("header-ip").innerText = "IP: " + (data.net ? data.net.ip : "---");
  document.getElementById("header-temp").innerText = (data.temp ? data.temp.toFixed(1) : "--.-") + " °C";

  if (data.time) {
    const clockEl = document.getElementById("ntp-clock-display");
    if (clockEl) {
      clockEl.innerText = data.time + (data.ntp_synced ? " (Synced)" : " (Unsynced)");
      clockEl.style.color = data.ntp_synced ? "var(--success)" : "var(--warning)";
    }
    const hClock = document.getElementById("header-clock");
    if (hClock) {
      const parts = data.time.split(" ");
      hClock.innerText = parts.length > 1 ? parts[1] : data.time;
    }
    const hSync = document.getElementById("header-clock-sync");
    if (hSync) {
      hSync.innerText = data.ntp_synced ? "SYNC" : "RTC";
      hSync.className = "status-pill " + (data.ntp_synced ? "online" : "offline");
    }
    const curTimeDisp = document.getElementById("time-current-display");
    if (curTimeDisp) {
      curTimeDisp.innerText = data.time;
    }
  }

  const emgBanner = document.getElementById("emg-banner");
  if (data.emg && data.emg.active) {
    emgBanner.style.display = "flex";
    document.getElementById("emg-text").innerText = "EMERGENCY ACTIVE (" + data.emg.source + ") — ALL RELAYS LOCKED";
  } else {
    emgBanner.style.display = "none";
  }

  const pwr = (data.total_power_w !== undefined) ? data.total_power_w : (data.total_power !== undefined ? data.total_power : 0);
  const pwrEl = document.getElementById("sum-power");
  if (pwrEl) pwrEl.innerText = pwr + " W";

  const sunEl = document.getElementById("sum-sun");
  if (sunEl) {
    const rise = data.sunrise || (data.sun ? data.sun.sunrise : "");
    const set = data.sunset || (data.sun ? data.sun.sunset : "");
    if (rise && set && rise !== "--:--") {
      const isNight = data.is_night !== undefined ? data.is_night : (data.sun ? data.sun.is_night : false);
      sunEl.innerText = `🌅 ${rise} | 🌇 ${set} ${isNight ? '🌙' : '☀️'}`;
    } else {
      sunEl.innerText = "🌅 --:-- | 🌇 --:--";
    }
  }

  if (data.relays) {
    relayConfigsCache = data.relays;
    const onCount = data.relays.filter(r => r.state).length;
    document.getElementById("sum-relays").innerText = onCount + " / 16";
    renderRelayCards(data.relays);
  }

  if (data.inputs) {
    const activeCount = data.inputs.filter(i => i.active).length;
    document.getElementById("sum-inputs").innerText = activeCount + " / 16";
    renderInputCards(data.inputs);
  }

  if (data.net) {
    document.getElementById("sum-eth").innerText = data.net.connected ? (data.net.speed + "M Full") : "Disconnected";
  }
  if (data.mqtt) {
    document.getElementById("sum-mqtt").innerText = data.mqtt.connected ? "Connected" : (data.mqtt.enabled ? "Disconnected" : "Disabled");
  }
  if (data.watchdog) {
    updateWatchdogMonitor(data.watchdog);
  }
  if (data.uptime !== undefined) {
    const s = data.uptime;
    const d = Math.floor(s / 86400);
    const h = Math.floor((s % 86400) / 3600);
    const m = Math.floor((s % 3600) / 60);
    document.getElementById("sum-uptime").innerText = d + "d " + h + "h " + m + "m " + (s % 60) + "s";
  }

  if (data.recent_logs) {
    renderRecentLogs(data.recent_logs);
  }
}

function filterRoom(room) {
  currentRoomFilter = room;
  if (sysState && sysState.relays) {
    renderRelayCards(sysState.relays);
  }
}

function renderRelayCards(relays) {
  const rooms = Array.from(new Set(relays.map(r => r.room).filter(Boolean)));
  const filterBar = document.getElementById("room-filter-bar");
  if (filterBar) {
    let btnHtml = `<button class="btn ${currentRoomFilter === '' ? 'btn-primary' : 'btn-secondary'}" style="padding:2px 8px;font-size:11px;" onclick="filterRoom('')">All</button>`;
    rooms.forEach(rm => {
      btnHtml += `<button class="btn ${currentRoomFilter === rm ? 'btn-primary' : 'btn-secondary'}" style="padding:2px 8px;font-size:11px;" onclick="filterRoom('${rm.replace(/'/g, "\\'")}')">${rm}</button>`;
    });
    filterBar.innerHTML = btnHtml;
  }

  const container = document.getElementById("relay-cards-grid");
  const filtered = currentRoomFilter ? relays.filter(r => r.room === currentRoomFilter) : relays;

  container.innerHTML = filtered.map((r) => {
    const i = (r.idx !== undefined) ? (r.idx - 1) : relays.indexOf(r);
    const isLocked = (sysState.emg && sysState.emg.locked) || !r.enabled;
    const timerText = r.timer_left > 0 ? `Auto-off: ${r.timer_left}s` : "";
    const grpTag = r.group > 0 ? `<span class="tag warn">Grp ${r.group}</span>` : "";
    const lockTag = isLocked ? `<span class="tag danger">Locked</span>` : "";
    const roomTag = r.room ? `<span class="tag" style="background:#202830;color:#9ca3af;">${r.room}</span>` : "";
    const powerTag = (r.state && r.power_w > 0) ? `<span class="tag" style="background:#0f3a2c;color:#10b981;">⚡ ${r.power_w}W</span>` : "";
    const icon = DEVICE_ICONS[r.device_icon || 0] || "💡";

    return `
      <div class="relay-card ${r.state ? 'on' : ''} ${isLocked ? 'locked' : ''}">
        <div class="relay-header">
          <div>
            <div class="relay-num">RELAY ${i + 1}</div>
            <div class="relay-name">${icon} ${r.name}</div>
          </div>
          <span class="status-pill ${r.state ? 'online' : 'offline'}">${r.state ? 'ON' : 'OFF'}</span>
        </div>
        <div class="relay-meta">
          ${roomTag}
          ${powerTag}
          ${grpTag}
          ${lockTag}
        </div>
        <div class="relay-timer" style="display:${r.timer_left > 0 ? 'block' : 'none'}">${timerText}</div>
        <div class="relay-actions">
          <button class="btn btn-block ${r.state ? 'btn-danger' : 'btn-primary'}"
                  ${isLocked ? 'disabled' : ''}
                  onclick="toggleRelay(${i})">
            ${r.state ? 'Turn OFF' : 'Turn ON'}
          </button>
        </div>
      </div>
    `;
  }).join("");
}

function renderInputCards(inputs) {
  const container = document.getElementById("input-cards-grid");
  const modeNames = ["Toggle", "Momentary", "State", "Scene", "Disabled", "Emergency"];
  container.innerHTML = inputs.map((inp, i) => {
    const trgtName = (inp.target_relay > 0 && inp.target_relay <= 16) ? `➔ Relay ${inp.target_relay}` : '➔ No Relay';
    return `
      <div class="input-row">
        <div style="display:flex;align-items:center;gap:10px;">
          <div class="input-indicator ${inp.active ? 'active' : ''} ${inp.timeout ? 'timeout' : ''}"></div>
          <div>
            <div style="font-weight:700;font-size:13px;color:#fff;">${inp.name}</div>
            <div style="font-size:11px;color:var(--text-dim);"><strong style="color:var(--info);">${trgtName}</strong> &bull; ${modeNames[inp.mode] || 'Toggle'} &bull; ${inp.last_evt || 'none'}</div>
          </div>
        </div>
        <div>
          ${inp.timeout ? '<span class="tag danger">TIMEOUT</span>' : ''}
          <span class="status-pill ${inp.active ? 'online' : 'offline'}">${inp.active ? 'ACTIVE' : 'IDLE'}</span>
        </div>
      </div>
    `;
  }).join("");
}

function renderRecentLogs(logs) {
  const box = document.getElementById("dashboard-recent-logs");
  box.innerHTML = logs.map(l => {
    return `<div class="log-line"><span class="log-time">${l.t}</span><span class="log-subsys">[${getSubsysName(l.s)}]</span><span>${l.m}</span></div>`;
  }).join("");
}

function getSubsysName(s) {
  return ["SYSTEM", "RELAY", "INPUT", "EMERGENCY", "NETWORK", "MQTT", "SECURITY"][s] || "SYS";
}

async function toggleRelay(idx) {
  if (ws && ws.readyState === WebSocket.OPEN) {
    ws.send(JSON.stringify({ type: "toggle", relay: idx, token: sessionToken }));
  } else {
    await api("/api/relay/toggle", "POST", { index: idx });
  }
}

async function allRelays(on) {
  if (on) {
    await api("/api/relay/all_on", "POST");
  } else {
    await api("/api/relay/all_off", "POST");
  }
}

function openEmgResetModal() {
  document.getElementById("emg-chk-1").checked = false;
  document.getElementById("emg-chk-2").checked = false;
  document.getElementById("emg-modal").style.display = "flex";
}

function closeEmgModal() {
  document.getElementById("emg-modal").style.display = "none";
}

async function submitEmgReset() {
  const c1 = document.getElementById("emg-chk-1").checked;
  const c2 = document.getElementById("emg-chk-2").checked;
  if (!c1 || !c2) {
    alert("Please check both confirmation boxes before resetting emergency state.");
    return;
  }
  const res = await api("/api/emergency/reset", "POST", { confirmed: true });
  if (res && res.success) {
    closeEmgModal();
  }
}

async function loadRelayConfigs() {
  const data = await api("/api/relay/config");
  if (!data) return;
  relayConfigsCache = data;
  const tbody = document.getElementById("tbody-relay-cfg");
  const iconLabels = ["💡 Light", "🔌 Socket", "🚪 Door", "♨️ Heater", "🚰 Pump", "❄️ AC", "⚡ Power"];
  tbody.innerHTML = data.map((r, i) => {
    let grpOptions = `<option value="0" ${r.interlock_group==0?'selected':''}>None (0)</option>`;
    for (let g = 1; g <= 8; g++) {
      grpOptions += `<option value="${g}" ${r.interlock_group==g?'selected':''}>Group ${g}</option>`;
    }
    let iconOptions = iconLabels.map((lbl, idx) => `<option value="${idx}" ${(r.device_icon || 0) == idx ? 'selected' : ''}>${lbl}</option>`).join("");
    return `
      <tr>
        <td>${i + 1}</td>
        <td><select id="rc-icon-${i}" style="width:95px;font-size:11px;">${iconOptions}</select></td>
        <td><input type="text" id="rc-name-${i}" value="${r.name}" style="width:110px;"></td>
        <td><input type="text" id="rc-room-${i}" value="${r.room || ''}" placeholder="Room" style="width:85px;"></td>
        <td><input type="number" id="rc-pwr-${i}" value="${r.rated_power_w || 0}" style="width:65px;" min="0"></td>
        <td><input type="checkbox" id="rc-en-${i}" ${r.enabled ? 'checked' : ''}></td>
        <td>
          <select id="rc-rest-${i}">
            <option value="0" ${r.restore_mode == 0 ? 'selected' : ''}>Always OFF</option>
            <option value="1" ${r.restore_mode == 1 ? 'selected' : ''}>Restore previous</option>
            <option value="2" ${r.restore_mode == 2 ? 'selected' : ''}>Always ON</option>
          </select>
        </td>
        <td><input type="number" id="rc-aoff-${i}" value="${r.auto_off_sec}" style="width:65px;"></td>
        <td><input type="number" id="rc-ondel-${i}" value="${r.on_delay_ms}" style="width:70px;"></td>
        <td><input type="number" id="rc-offdel-${i}" value="${r.off_delay_ms}" style="width:70px;"></td>
        <td><select id="rc-grp-${i}" style="width:90px;">${grpOptions}</select></td>
        <td><input type="checkbox" id="rc-man-${i}" ${r.manual_control_enabled ? 'checked' : ''}></td>
        <td><input type="checkbox" id="rc-mqtt-${i}" ${r.mqtt_control_enabled ? 'checked' : ''}></td>
        <td><input type="checkbox" id="rc-inp-${i}" ${r.input_control_enabled ? 'checked' : ''}></td>
        <td><input type="checkbox" id="rc-log-${i}" ${r.event_logging_enabled ? 'checked' : ''}></td>
        <td><button class="btn btn-secondary" style="padding:4px 8px;font-size:11px;" onclick="saveRelayConfig(${i})">Save</button></td>
      </tr>
    `;
  }).join("");
}

async function saveRelayConfig(i) {
  const body = {
    index: i,
    name: document.getElementById(`rc-name-${i}`).value,
    device_icon: parseInt(document.getElementById(`rc-icon-${i}`).value) || 0,
    room: document.getElementById(`rc-room-${i}`).value || "",
    rated_power_w: parseInt(document.getElementById(`rc-pwr-${i}`).value) || 0,
    enabled: document.getElementById(`rc-en-${i}`).checked,
    restore_mode: parseInt(document.getElementById(`rc-rest-${i}`).value),
    auto_off_sec: parseInt(document.getElementById(`rc-aoff-${i}`).value) || 0,
    on_delay_ms: parseInt(document.getElementById(`rc-ondel-${i}`).value) || 0,
    off_delay_ms: parseInt(document.getElementById(`rc-offdel-${i}`).value) || 0,
    interlock_group: parseInt(document.getElementById(`rc-grp-${i}`).value) || 0,
    manual_control_enabled: document.getElementById(`rc-man-${i}`).checked,
    mqtt_control_enabled: document.getElementById(`rc-mqtt-${i}`).checked,
    input_control_enabled: document.getElementById(`rc-inp-${i}`).checked,
    event_logging_enabled: document.getElementById(`rc-log-${i}`).checked
  };
  await api("/api/relay/config", "POST", body);
  alert(`Relay ${i + 1} updated`);
}

async function loadInterlockMatrix() {
  const data = await api("/api/interlock/config");
  if (!data) return;
  const grid = document.getElementById("interlock-matrix-grid");
  grid.innerHTML = data.map((r, i) => {
    let opts = `<option value="0" ${r.group==0?'selected':''}>No Group (0)</option>`;
    for (let g = 1; g <= 8; g++) {
      opts += `<option value="${g}" ${r.group==g?'selected':''}>Group ${g}</option>`;
    }
    return `
      <div style="background:#161d24;padding:8px 10px;border-radius:6px;border:1px solid var(--card-border);">
        <div style="font-weight:700;font-size:12px;margin-bottom:4px;color:#fff;">R${i + 1}: ${r.name}</div>
        <select id="intl-grp-${i}" onchange="updateSingleInterlock(${i}, this.value)" style="font-size:12px;padding:4px;">
          ${opts}
        </select>
      </div>
    `;
  }).join("");

  renderInterlockStatus(data);
}

async function updateSingleInterlock(idx, grp) {
  await api("/api/interlock/config", "POST", { index: idx, group: parseInt(grp) });
  const data = await api("/api/interlock/config");
  if (data) renderInterlockStatus(data);
}

async function saveInterlockMatrix() {
  const groups = [];
  for (let i = 0; i < 16; i++) {
    groups.push(parseInt(document.getElementById(`intl-grp-${i}`).value) || 0);
  }
  await api("/api/interlock/config", "POST", { groups });
  alert("Interlock matrix saved successfully!");
  const data = await api("/api/interlock/config");
  if (data) renderInterlockStatus(data);
}

function renderInterlockStatus(relaysData) {
  const grid = document.getElementById("interlock-groups-grid");
  let html = "";
  for (let g = 1; g <= 8; g++) {
    const members = relaysData.filter(r => r.group === g);
    const activeRelay = sysState && sysState.relays ? sysState.relays.find(r => r.group === g && r.state) : null;
    html += `
      <div class="card" style="background:#161d24;">
        <div class="card-title">Group ${g}</div>
        <div style="font-size:12px;color:var(--text-dim);margin-bottom:8px;">
          Members: ${members.length > 0 ? members.map(m => `R${m.index+1} (${m.name})`).join(", ") : "None assigned"}
        </div>
        <div style="font-size:12px;">
          Status: ${activeRelay ? `<strong style="color:var(--success);">${activeRelay.name} (ON)</strong>` : "<span style='color:var(--text-dim)'>All OFF</span>"}
        </div>
      </div>
    `;
  }
  grid.innerHTML = html;
}

async function loadInputConfigs() {
  const data = await api("/api/input/config");
  if (!data) return;

  if (data.defaults) {
    document.getElementById("gdef-deb").value = data.defaults.debounce_ms;
    document.getElementById("gdef-long").value = data.defaults.long_press_ms;
    document.getElementById("gdef-dbl").value = data.defaults.double_click_ms;
    document.getElementById("gdef-tout").value = data.defaults.signal_timeout_s;
    document.getElementById("gdef-tact").value = data.defaults.timeout_action;
  }

  const container = document.getElementById("inputs-cards-container");

  let relayOptions = `<option value="0">None (No Target Relay)</option>`;
  for (let r = 1; r <= 16; r++) {
    const rName = (relayConfigsCache[r-1] && relayConfigsCache[r-1].name) ? relayConfigsCache[r-1].name : `Relay ${r}`;
    relayOptions += `<option value="${r}">Relay ${r}: ${rName}</option>`;
  }

  container.innerHTML = data.inputs.map((inp, i) => {
    return `
      <div class="card" style="background:#161d24;">
        <div style="display:flex;justify-content:space-between;align-items:center;margin-bottom:12px;">
          <div style="font-weight:700;font-size:14px;color:#fff;">Input ${i + 1}</div>
          <div style="display:flex;align-items:center;gap:6px;">
            <label style="font-size:12px;color:var(--text-dim);display:flex;align-items:center;gap:4px;">
              <input type="checkbox" id="ic-en-${i}" ${inp.enabled ? 'checked' : ''} style="width:auto;"> Enabled
            </label>
          </div>
        </div>

        <div style="display:grid;grid-template-columns:1fr 1fr;gap:10px;margin-bottom:10px;">
          <div class="form-group" style="margin-bottom:0;">
            <label class="form-label">Name</label>
            <input type="text" id="ic-name-${i}" value="${inp.name}">
          </div>
          <div class="form-group" style="margin-bottom:0;">
            <label class="form-label" style="color:var(--info);">Target Relay (Any 1-16)</label>
            <select id="ic-trgt-${i}" style="border-color:var(--primary);font-weight:600;">
              ${relayOptions}
            </select>
          </div>
        </div>

        <div style="display:grid;grid-template-columns:1fr 1fr;gap:10px;margin-bottom:10px;">
          <div class="form-group" style="margin-bottom:0;">
            <label class="form-label">Operation Mode</label>
            <select id="ic-mode-${i}">
              <option value="0" ${inp.mode==0?'selected':''}>Toggle Switch (Fast on press)</option>
              <option value="6" ${inp.mode==6?'selected':''}>Multi-Action (Single / Double / Long)</option>
              <option value="1" ${inp.mode==1?'selected':''}>Momentary Button (ON while held)</option>
              <option value="2" ${inp.mode==2?'selected':''}>Follow Switch State (Level/Rocker)</option>
              <option value="3" ${inp.mode==3?'selected':''}>Activate Scene</option>
              <option value="4" ${inp.mode==4?'selected':''}>Disabled</option>
              <option value="5" ${inp.mode==5?'selected':''}>Emergency Stop</option>
            </select>
          </div>
          <div class="form-group" style="margin-bottom:0;">
            <label class="form-label">Hardware Polarity</label>
            <select id="ic-pol-${i}">
              <option value="0" ${inp.polarity==0?'selected':''}>Active-LOW (Switch to GND)</option>
              <option value="1" ${inp.polarity==1?'selected':''}>Active-HIGH</option>
            </select>
          </div>
        </div>

        <div style="display:grid;grid-template-columns:1fr 1fr;gap:10px;margin-bottom:10px;">
          <div class="form-group" style="margin-bottom:0;">
            <label class="form-label" style="color:var(--info);">Double Click Action</label>
            <select id="ic-dbl-${i}">
              <option value="0" ${inp.double_action==0?'selected':''}>None</option>
              <option value="3" ${inp.double_action==3?'selected':''}>Toggle Target Relay</option>
              <option value="4" ${inp.double_action==4?'selected':''}>Activate Scene</option>
              <option value="6" ${inp.double_action==6?'selected':''}>Master ON (All Relays ON)</option>
              <option value="5" ${inp.double_action==5?'selected':''}>Master OFF (All Relays OFF)</option>
            </select>
          </div>
          <div class="form-group" style="margin-bottom:0;">
            <label class="form-label" style="color:var(--warning);">Long Press Action</label>
            <select id="ic-long-${i}">
              <option value="0" ${inp.long_action==0?'selected':''}>None</option>
              <option value="5" ${inp.long_action==5?'selected':''}>Master OFF (All Relays OFF)</option>
              <option value="6" ${inp.long_action==6?'selected':''}>Master ON (All Relays ON)</option>
              <option value="3" ${inp.long_action==3?'selected':''}>Toggle Target Relay</option>
              <option value="4" ${inp.long_action==4?'selected':''}>Activate Scene</option>
              <option value="7" ${inp.long_action==7?'selected':''}>Emergency Stop</option>
            </select>
          </div>
        </div>

        <div style="display:flex;justify-content:space-between;align-items:center;margin-top:12px;">
          <span style="font-size:11px;color:var(--text-dim);" id="ic-status-${i}">Debounce: ${inp.debounce_ms} ms</span>
          <div style="display:flex;gap:6px;">
            <button class="btn btn-secondary" style="padding:4px 8px;font-size:11px;" onclick="resetInputDefaults(${i})">Reset</button>
            <button class="btn btn-success" style="padding:4px 12px;font-size:11px;" onclick="saveInputConfig(${i})">Save Input ${i + 1}</button>
          </div>
        </div>
      </div>
    `;
  }).join("");

  data.inputs.forEach((inp, i) => {
    document.getElementById(`ic-trgt-${i}`).value = inp.target_relay;
    document.getElementById(`ic-dbl-${i}`).value = inp.double_action || 0;
    document.getElementById(`ic-long-${i}`).value = inp.long_action || 0;
  });
}

async function saveInputConfig(i) {
  const body = {
    index: i,
    name: document.getElementById(`ic-name-${i}`).value,
    enabled: document.getElementById(`ic-en-${i}`).checked,
    polarity: parseInt(document.getElementById(`ic-pol-${i}`).value),
    mode: parseInt(document.getElementById(`ic-mode-${i}`).value),
    target_relay: parseInt(document.getElementById(`ic-trgt-${i}`).value) || 0,
    target_scene: 1,
    short_action: 3,
    long_action: parseInt(document.getElementById(`ic-long-${i}`).value) || 0,
    double_action: parseInt(document.getElementById(`ic-dbl-${i}`).value) || 0,
    startup: 0,
    is_emergency: (parseInt(document.getElementById(`ic-mode-${i}`).value) === 5),
    dash_visible: true,
    mqtt_pub: true,
    log_enabled: true
  };
  await api("/api/input/config", "POST", body);
  const statusEl = document.getElementById(`ic-status-${i}`);
  if (statusEl) {
    statusEl.innerHTML = "<strong style='color:var(--success);'>Saved successfully!</strong>";
    setTimeout(() => { statusEl.innerText = "Debounce: 50 ms"; }, 2000);
  }
}

async function resetInputDefaults(i) {
  await api("/api/input/defaults/reset", "POST", { index: i });
  loadInputConfigs();
}

async function saveGlobalDefaults() {
  const body = {
    debounce_ms: parseInt(document.getElementById("gdef-deb").value) || 50,
    long_press_ms: parseInt(document.getElementById("gdef-long").value) || 1500,
    double_click_ms: parseInt(document.getElementById("gdef-dbl").value) || 350,
    signal_timeout_s: parseInt(document.getElementById("gdef-tout").value) || 0,
    timeout_action: parseInt(document.getElementById("gdef-tact").value)
  };
  await api("/api/input/defaults", "POST", body);
  alert("Global defaults saved");
}

async function applyDefaultsToAllPrompt() {
  if (confirm("Apply global timing defaults to all 16 inputs?")) {
    await api("/api/input/defaults/apply_all", "POST");
    loadInputConfigs();
    alert("Applied defaults to all inputs");
  }
}

function renderTimers() {
  if (!sysState || !sysState.relays) return;
  const tbody = document.getElementById("tbody-timers");
  tbody.innerHTML = sysState.relays.map((r, i) => `
    <tr>
      <td><strong>Relay ${i + 1}</strong> (${r.name})</td>
      <td><span class="status-pill ${r.state ? 'online' : 'offline'}">${r.state ? 'ON' : 'OFF'}</span></td>
      <td>${r.auto_off_sec > 0 ? r.auto_off_sec + ' s' : 'Disabled'}</td>
      <td style="color:${r.timer_left > 0 ? 'var(--warning)' : 'inherit'}">${r.timer_left > 0 ? r.timer_left + ' s remaining' : 'None'}</td>
      <td>${r.on_delay_ms || 0} ms</td>
      <td>${r.off_delay_ms || 0} ms</td>
    </tr>
  `).join("");
}

async function loadSceneConfigs() {
  const data = await api("/api/scene/config");
  if (!data) return;
  const container = document.getElementById("scenes-grid");
  container.innerHTML = data.map((sc, i) => `
    <div class="card">
      <div class="card-title">
        <input type="text" id="sc-name-${i}" value="${sc.name}" style="width:65%;">
        <div style="display:flex;gap:6px;">
          <button class="btn btn-secondary" style="padding:4px 8px;font-size:11px;" onclick="saveScene(${i})">Save</button>
          <button class="btn btn-success" style="padding:4px 8px;font-size:11px;" onclick="activateScene(${i})">Activate</button>
        </div>
      </div>
      <div style="font-size:12px;color:var(--text-dim);margin-bottom:8px;">Relay Actions:</div>
      <div style="display:grid;grid-template-columns:repeat(auto-fill,minmax(110px,1fr));gap:6px;">
        ${sc.actions.map((act, r) => `
          <div style="display:flex;align-items:center;justify-content:space-between;background:#12161a;padding:4px 6px;border-radius:4px;border:1px solid var(--card-border);">
            <span style="font-size:11px;">R${r + 1}</span>
            <select id="sc-${i}-r-${r}" style="width:70px;padding:2px;font-size:11px;">
              <option value="0" ${act==0?'selected':''}>Ignore</option>
              <option value="1" ${act==1?'selected':''}>OFF</option>
              <option value="2" ${act==2?'selected':''}>ON</option>
            </select>
          </div>
        `).join("")}
      </div>
    </div>
  `).join("");
}

async function saveScene(i) {
  const actions = [];
  for (let r = 0; r < 16; r++) {
    actions.push(parseInt(document.getElementById(`sc-${i}-r-${r}`).value));
  }
  const body = {
    index: i,
    name: document.getElementById(`sc-name-${i}`).value,
    actions: actions
  };
  await api("/api/scene/config", "POST", body);
  alert(`Scene ${i + 1} saved`);
}

async function activateScene(i) {
  await api("/api/scene/activate", "POST", { index: i });
}

async function loadNetworkCfg() {
  const data = await api("/api/network/config");
  if (!data) return;
  document.getElementById("net-host").value = data.hostname;
  document.getElementById("net-dhcp").value = data.dhcp ? "1" : "0";
  document.getElementById("net-ip").value = data.ip;
  document.getElementById("net-gw").value = data.gateway;
  document.getElementById("net-mask").value = data.subnet;
  document.getElementById("net-dns").value = data.dns;
  toggleNetFields();
}

function toggleNetFields() {
  const isDhcp = document.getElementById("net-dhcp").value === "1";
  document.getElementById("net-static-group").style.display = isDhcp ? "none" : "block";
}

async function saveNetworkCfg() {
  const body = {
    hostname: document.getElementById("net-host").value,
    dhcp: document.getElementById("net-dhcp").value === "1",
    ip: document.getElementById("net-ip").value,
    gateway: document.getElementById("net-gw").value,
    subnet: document.getElementById("net-mask").value,
    dns: document.getElementById("net-dns").value
  };
  await api("/api/network/config", "POST", body);
  const a = document.getElementById("net-alert");
  a.innerText = "Network settings saved. A system reboot is recommended.";
  a.style.display = "block";
  setTimeout(() => { a.style.display = "none"; }, 4000);
}

async function loadWifiCfg() {
  const data = await api("/api/wifi/config");
  if (!data) return;
  document.getElementById("wifi-sta-en").value = data.sta_enabled ? "1" : "0";
  document.getElementById("wifi-sta-ssid").value = data.sta_ssid || "";
  document.getElementById("wifi-sta-pass").value = "";
  document.getElementById("wifi-ap-fb").value = data.ap_fallback_enabled ? "1" : "0";
  document.getElementById("wifi-ap-ssid").value = data.ap_ssid || "WT32-Relay-Setup";
  document.getElementById("wifi-ap-pass").value = "";

  const b = document.getElementById("wifi-status-badge");
  if (data.sta_connected) {
    b.className = "status-pill online";
    b.innerText = "STA Connected (" + data.sta_ip + ", " + data.sta_rssi + " dBm)";
  } else if (data.ap_active) {
    b.className = "status-pill warning";
    b.innerText = "AP Active (" + data.ap_ip + ", clients: " + data.ap_clients + ")";
  } else {
    b.className = "status-pill offline";
    b.innerText = "Wi-Fi Idle (LAN Active)";
  }
}

async function saveWifiCfg() {
  const body = {
    sta_enabled: document.getElementById("wifi-sta-en").value === "1",
    sta_ssid: document.getElementById("wifi-sta-ssid").value,
    sta_pass: document.getElementById("wifi-sta-pass").value,
    ap_fallback_enabled: document.getElementById("wifi-ap-fb").value === "1",
    ap_ssid: document.getElementById("wifi-ap-ssid").value,
    ap_pass: document.getElementById("wifi-ap-pass").value
  };
  await api("/api/wifi/config", "POST", body);
  const a = document.getElementById("wifi-alert");
  a.innerText = "Wi-Fi settings saved successfully.";
  a.style.display = "block";
  setTimeout(() => { a.style.display = "none"; }, 4000);
}

async function scanWifiNetworks() {
  const listEl = document.getElementById("wifi-scan-list");
  listEl.style.display = "block";
  listEl.innerHTML = "<div style='color:var(--text-dim);font-size:12px;padding:4px;'>Scanning 2.4 GHz Wi-Fi networks...</div>";
  const networks = await api("/api/wifi/scan");
  if (!networks || networks.length === 0) {
    listEl.innerHTML = "<div style='color:var(--text-dim);font-size:12px;padding:4px;'>No networks detected.</div>";
    return;
  }
  listEl.innerHTML = networks.map(n => `
    <div style="display:flex;justify-content:space-between;padding:4px 8px;cursor:pointer;font-size:12px;border-bottom:1px solid var(--border);"
         onclick="document.getElementById('wifi-sta-ssid').value='${n.ssid}';document.getElementById('wifi-scan-list').style.display='none';">
      <span><strong>${n.ssid}</strong></span>
      <span style="color:var(--text-dim);">${n.rssi} dBm</span>
    </div>
  `).join("");
}

async function loadWatchdogCfg() {
  const data = await api("/api/watchdog/config");
  if (!data) return;

  const sel = document.getElementById("wd-relay");
  sel.innerHTML = "";
  for (let i = 1; i <= 16; i++) {
    const rName = relayConfigsCache[i - 1] ? relayConfigsCache[i - 1].name : `Relay ${i}`;
    sel.innerHTML += `<option value="${i}">${i}: ${rName}</option>`;
  }

  document.getElementById("wd-en").value = data.enabled ? "1" : "0";
  document.getElementById("wd-host").value = data.host || "8.8.8.8";
  document.getElementById("wd-port").value = data.port || 53;
  document.getElementById("wd-interval").value = data.interval || 60;
  document.getElementById("wd-fails").value = data.fails || 3;
  document.getElementById("wd-relay").value = data.relay || 1;
  document.getElementById("wd-off").value = data.off_sec || 10;
  document.getElementById("wd-cooldown").value = data.cooldown || 300;

  updateWatchdogMonitor(data);
}

function updateWatchdogMonitor(wd) {
  if (!wd) return;
  const statusEl = document.getElementById("wd-mon-status");
  if (statusEl) {
    statusEl.innerText = wd.last_result || (wd.enabled ? "Active" : "Disabled");
    statusEl.style.color = (wd.last_result && wd.last_result.startsWith("OK")) ? "var(--success)" :
                           (wd.is_power_cycling || wd.in_cooldown) ? "var(--warning)" : "var(--info)";
  }
  const failsEl = document.getElementById("wd-mon-fails");
  if (failsEl) failsEl.innerText = `${wd.current_fails || 0} / ${wd.fail_threshold || 3}`;
  const rebootsEl = document.getElementById("wd-mon-reboots");
  if (rebootsEl) rebootsEl.innerText = wd.total_reboots || 0;
  const targetEl = document.getElementById("wd-mon-target");
  if (targetEl) targetEl.innerText = `${wd.host || "8.8.8.8"}:${wd.port || 53}`;
}

async function saveWatchdogCfg() {
  const body = {
    enabled: document.getElementById("wd-en").value === "1",
    host: document.getElementById("wd-host").value,
    port: parseInt(document.getElementById("wd-port").value) || 53,
    interval: parseInt(document.getElementById("wd-interval").value) || 60,
    fails: parseInt(document.getElementById("wd-fails").value) || 3,
    relay: parseInt(document.getElementById("wd-relay").value) || 1,
    off_sec: parseInt(document.getElementById("wd-off").value) || 10,
    cooldown: parseInt(document.getElementById("wd-cooldown").value) || 300
  };
  await api("/api/watchdog/config", "POST", body);
  const a = document.getElementById("wd-alert");
  a.innerText = "Watchdog settings saved successfully.";
  a.style.display = "block";
  setTimeout(() => { a.style.display = "none"; }, 4000);
}

async function testWatchdogPing() {
  const resEl = document.getElementById("wd-test-result");
  resEl.style.display = "block";
  resEl.className = "alert alert-warning";
  resEl.innerText = "Pinging remote host now...";
  const res = await api("/api/watchdog/test", "POST");
  if (res && res.success) {
    resEl.className = "alert alert-success";
    resEl.innerText = "Ping test SUCCESS: " + res.result;
  } else {
    resEl.className = "alert alert-danger";
    resEl.innerText = "Ping test FAILED: " + (res ? res.result : "Connection failed");
  }
}

function toggleMqttSsl() {
  const isSsl = document.getElementById("mqtt-ssl").value === "1";
  document.getElementById("mqtt-ssl-mode-group").style.display = isSsl ? "block" : "none";
  const portEl = document.getElementById("mqtt-port");
  if (isSsl && portEl.value == "1883") portEl.value = 8883;
  if (!isSsl && portEl.value == "8883") portEl.value = 1883;
}

async function loadMqttCfg() {
  const data = await api("/api/mqtt/config");
  if (!data) return;
  document.getElementById("mqtt-en").value = data.enabled ? "1" : "0";
  document.getElementById("mqtt-ssl").value = data.use_ssl ? "1" : "0";
  document.getElementById("mqtt-ssl-mode").value = data.ssl_mode || 0;
  toggleMqttSsl();
  document.getElementById("mqtt-host").value = data.host;
  document.getElementById("mqtt-port").value = data.port;
  document.getElementById("mqtt-user").value = data.user;
  document.getElementById("mqtt-pass").value = data.pass;
  document.getElementById("mqtt-cid").value = data.client_id;
  document.getElementById("mqtt-topic").value = data.base_topic;
  document.getElementById("mqtt-ha").value = data.ha_discovery ? "1" : "0";
}

async function saveMqttCfg() {
  const body = {
    enabled: document.getElementById("mqtt-en").value === "1",
    use_ssl: document.getElementById("mqtt-ssl").value === "1",
    ssl_mode: parseInt(document.getElementById("mqtt-ssl-mode").value) || 0,
    host: document.getElementById("mqtt-host").value,
    port: parseInt(document.getElementById("mqtt-port").value) || 1883,
    user: document.getElementById("mqtt-user").value,
    pass: document.getElementById("mqtt-pass").value,
    client_id: document.getElementById("mqtt-cid").value,
    base_topic: document.getElementById("mqtt-topic").value,
    ha_discovery: document.getElementById("mqtt-ha").value === "1"
  };
  await api("/api/mqtt/config", "POST", body);
  const a = document.getElementById("mqtt-alert");
  a.innerText = "MQTT settings saved and reconnected.";
  a.style.display = "block";
}

async function loadSecurityCfg() {
  const data = await api("/api/security/config");
  if (!data) return;
  document.getElementById("sec-new-user").value = data.username;
  document.getElementById("sec-timeout").value = data.session_timeout_s;
  document.getElementById("sec-lock-att").value = data.lockout_attempts;
  document.getElementById("sec-lock-time").value = data.lockout_time_s;
}

async function changePassword() {
  const oldPass = document.getElementById("sec-old-pass").value;
  const newUser = document.getElementById("sec-new-user").value;
  const newPass = document.getElementById("sec-new-pass").value;
  const alertEl = document.getElementById("sec-alert");

  const res = await api("/api/security/credentials", "POST", { old_pass: oldPass, new_user: newUser, new_pass: newPass });
  if (res && res.success) {
    alertEl.className = "alert alert-success";
    alertEl.innerText = "Credentials updated successfully";
    alertEl.style.display = "block";
    document.getElementById("sec-old-pass").value = "";
    document.getElementById("sec-new-pass").value = "";
  } else {
    alertEl.className = "alert alert-danger";
    alertEl.innerText = res ? res.error : "Failed to change credentials";
    alertEl.style.display = "block";
  }
}

async function saveSecurityPolicy() {
  const body = {
    session_timeout_s: parseInt(document.getElementById("sec-timeout").value) || 900,
    lockout_attempts: parseInt(document.getElementById("sec-lock-att").value) || 5,
    lockout_time_s: parseInt(document.getElementById("sec-lock-time").value) || 300
  };
  await api("/api/security/policy", "POST", body);
  const a = document.getElementById("sec-pol-alert");
  a.innerText = "Security policy saved";
  a.style.display = "block";
}

async function loadEventLogs() {
  const subsys = document.getElementById("log-filter").value;
  const data = await api("/api/logs?subsys=" + subsys);
  if (!data) return;
  const box = document.getElementById("full-log-box");
  box.innerHTML = data.map(l => `
    <div class="log-line">
      <span class="log-time">${l.t}</span>
      <span class="log-subsys">[${getSubsysName(l.s)}]</span>
      <span>${l.m}</span>
    </div>
  `).join("");
}

async function clearLogs() {
  if (confirm("Clear in-memory event log buffer?")) {
    await api("/api/logs/clear", "POST");
    loadEventLogs();
  }
}

async function loadSysInfo() {
  const data = await api("/api/system/info");
  if (!data) return;
  const tbody = document.getElementById("tbody-sysinfo");
  tbody.innerHTML = `
    <tr><td>Device Hardware</td><td>WT32-ETH01 (ESP32-WROOM-32)</td></tr>
    <tr><td>Ethernet PHY</td><td>LAN8720 (RMII, 50MHz Clock GPIO0)</td></tr>
    <tr><td>Ethernet MAC</td><td>${data.mac}</td></tr>
    <tr><td>IP Address</td><td>${data.ip}</td></tr>
    <tr><td>CPU Frequency</td><td>${data.cpu_freq} MHz</td></tr>
    <tr><td>Internal Core Temp</td><td>${data.temp.toFixed(1)} °C</td></tr>
    <tr><td>Free Heap</td><td>${data.free_heap} bytes</td></tr>
    <tr><td>Min Free Heap</td><td>${data.min_heap} bytes</td></tr>
    <tr><td>Flash Chip Size</td><td>${data.flash_size} bytes</td></tr>
    <tr><td>I2C Bus Status</td><td>SDA GPIO14, SCL GPIO15 @ 100 kHz</td></tr>
    <tr><td>I2C Expanders</td><td>out1: ${data.out1_ok?'OK':'FAIL'}, out2: ${data.out2_ok?'OK':'FAIL'}, in1: ${data.in1_ok?'OK':'FAIL'}, in2: ${data.in2_ok?'OK':'FAIL'}</td></tr>
    <tr><td>Firmware Name</td><td>${data.firmware}</td></tr>
    <tr><td>Firmware Version</td><td>${data.version}</td></tr>
  `;
}

async function confirmReboot() {
  if (confirm("Are you sure you want to reboot the controller?")) {
    await api("/api/system/reboot", "POST");
    alert("Reboot command sent. Please reconnect in 10 seconds.");
  }
}

async function confirmFactoryReset() {
  const p1 = confirm("WARNING: Factory Reset will erase all persistent configurations in NVS.");
  if (p1) {
    const p2 = confirm("CONFIRM FACTORY RESET: All network settings, passwords, relay configs, and scenes will be lost. Proceed?");
    if (p2) {
      await api("/api/system/reset", "POST");
      alert("Factory reset complete. Device is rebooting...");
      setTimeout(() => window.location.reload(), 4000);
    }
  }
}

async function startOtaUpload() {
  const fileInput = document.getElementById("ota-file");
  const alertEl = document.getElementById("ota-alert");
  const progWrap = document.getElementById("ota-prog-wrap");
  const progBar = document.getElementById("ota-bar");
  const progPct = document.getElementById("ota-pct");
  const statusEl = document.getElementById("ota-status");
  const btn = document.getElementById("ota-btn");

  if (!fileInput.files || fileInput.files.length === 0) {
    alert("Please select a firmware .bin file first.");
    return;
  }
  const file = fileInput.files[0];
  if (!confirm(`Are you sure you want to flash "${file.name}" (${(file.size/1024).toFixed(1)} KB)?`)) return;

  btn.disabled = true;
  progWrap.style.display = "block";
  alertEl.style.display = "none";
  progBar.style.width = "0%";
  progPct.innerText = "0%";
  statusEl.innerText = "Flashing firmware to WT32-ETH01...";

  const formData = new FormData();
  formData.append("update", file);

  const xhr = new XMLHttpRequest();
  const token = sessionToken || localStorage.getItem("wt32_session") || "";
  xhr.open("POST", "/api/system/update?token=" + encodeURIComponent(token), true);
  xhr.setRequestHeader("X-Session", token);

  xhr.upload.onprogress = (e) => {
    if (e.lengthComputable) {
      const pct = Math.round((e.loaded / e.total) * 100);
      progBar.style.width = pct + "%";
      progPct.innerText = pct + "%";
      if (pct >= 100) {
        statusEl.innerText = "Writing flash & verifying integrity...";
      }
    }
  };

  xhr.onload = () => {
    btn.disabled = false;
    if (xhr.status === 200) {
      alertEl.className = "alert alert-success";
      alertEl.innerText = "Firmware updated successfully! System is rebooting. Page will reload in 10 seconds...";
      alertEl.style.display = "block";
      let count = 10;
      setInterval(() => {
        count--;
        if (count > 0) alertEl.innerText = `Rebooting... reconnecting in ${count}s...`;
        else location.reload();
      }, 1000);
    } else {
      alertEl.className = "alert alert-danger";
      alertEl.innerText = "Firmware update failed: " + xhr.responseText;
      alertEl.style.display = "block";
    }
  };

  xhr.onerror = () => {
    btn.disabled = false;
    alertEl.className = "alert alert-danger";
    alertEl.innerText = "Network error during upload.";
    alertEl.style.display = "block";
  };

  xhr.send(formData);
}

function downloadBackup() {
  const token = sessionToken || localStorage.getItem("wt32_session") || "";
  window.open("/api/system/backup?token=" + encodeURIComponent(token), "_blank");
}

async function restoreBackup() {
  const fileInput = document.getElementById("restore-file");
  const alertEl = document.getElementById("bak-alert");
  if (!fileInput.files || fileInput.files.length === 0) {
    alert("Please select a JSON backup file first.");
    return;
  }
  const file = fileInput.files[0];
  const reader = new FileReader();
  reader.onload = async (e) => {
    try {
      const json = JSON.parse(e.target.result);
      alertEl.className = "alert alert-warning";
      alertEl.innerText = "Applying configuration backup...";
      alertEl.style.display = "block";
      const res = await api("/api/system/restore", "POST", json);
      if (res && res.success) {
        alertEl.className = "alert alert-success";
        alertEl.innerText = "Configuration restored successfully! Reloading in 2 seconds...";
        setTimeout(() => location.reload(), 2000);
      } else {
        alertEl.className = "alert alert-danger";
        alertEl.innerText = (res && res.error) ? res.error : "Failed to restore backup";
      }
    } catch (err) {
      alertEl.className = "alert alert-danger";
      alertEl.innerText = "Invalid JSON file format";
      alertEl.style.display = "block";
    }
  };
  reader.readAsText(file);
}

let schedulesCache = [];

async function loadSchedules() {
  const data = await api("/api/schedule/config");
  if (!data) return;
  schedulesCache = data;
  renderSchedules();
}

function renderSchedules() {
  const container = document.getElementById("schedules-grid");
  if (!container) return;

  const dayNames = ["Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"];

  let relayOptions = "";
  for (let r = 1; r <= 16; r++) {
    const rName = (relayConfigsCache[r-1] && relayConfigsCache[r-1].name) ? relayConfigsCache[r-1].name : `Relay ${r}`;
    relayOptions += `<option value="${r}">Relay ${r}: ${rName}</option>`;
  }

  container.innerHTML = schedulesCache.map((sc, i) => {
    const timeStr = `${String(sc.hour).padStart(2,'0')}:${String(sc.minute).padStart(2,'0')}`;
    const trigType = sc.trigger_type || 0;
    const isClock = (trigType === 0);
    return `
      <div class="card" style="background:#161d24;margin-bottom:12px;">
        <div style="display:flex;justify-content:space-between;align-items:center;margin-bottom:10px;">
          <input type="text" id="sch-name-${i}" value="${sc.name}" style="font-weight:700;font-size:14px;color:#fff;width:60%;">
          <label style="font-size:12px;display:flex;align-items:center;gap:4px;cursor:pointer;">
            <input type="checkbox" id="sch-en-${i}" ${sc.enabled ? 'checked' : ''} style="width:auto;"> Active
          </label>
        </div>

        <div style="display:grid;grid-template-columns:1fr 1fr;gap:10px;margin-bottom:10px;">
          <div class="form-group" style="margin-bottom:0;">
            <label class="form-label">Trigger Type (Astro / Clock)</label>
            <select id="sch-trig-${i}" onchange="toggleScheduleFields(${i})">
              <option value="0" ${trigType == 0 ? 'selected' : ''}>⏰ Clock Time</option>
              <option value="1" ${trigType == 1 ? 'selected' : ''}>🌅 At Sunrise</option>
              <option value="2" ${trigType == 2 ? 'selected' : ''}>🌇 At Sunset</option>
            </select>
          </div>
          <div class="form-group" style="margin-bottom:0;">
            <label class="form-label" id="sch-trig-lbl-${i}">${isClock ? 'Trigger Time (HH:MM)' : 'Offset Minutes (±)'}</label>
            <div id="sch-time-wrap-${i}" style="display:${isClock ? 'block' : 'none'};">
              <input type="time" id="sch-time-${i}" value="${timeStr}">
            </div>
            <div id="sch-offset-wrap-${i}" style="display:${isClock ? 'none' : 'block'};">
              <input type="number" id="sch-offset-${i}" value="${sc.offset_minutes || 0}" min="-240" max="240" placeholder="e.g. -30 for 30m before">
            </div>
          </div>
        </div>

        <div style="display:grid;grid-template-columns:1fr 1fr;gap:10px;margin-bottom:10px;">
          <div class="form-group" style="margin-bottom:0;">
            <label class="form-label">Action</label>
            <select id="sch-act-${i}">
              <option value="1" ${sc.action==1?'selected':''}>Turn Relay ON</option>
              <option value="2" ${sc.action==2?'selected':''}>Turn Relay OFF</option>
              <option value="3" ${sc.action==3?'selected':''}>Toggle Relay</option>
              <option value="4" ${sc.action==4?'selected':''}>Activate Scene</option>
              <option value="5" ${sc.action==5?'selected':''}>Master OFF (All Relays OFF)</option>
              <option value="6" ${sc.action==6?'selected':''}>Master ON (All Relays ON)</option>
            </select>
          </div>
          <div class="form-group" style="margin-bottom:0;">
            <label class="form-label">Target Relay / Scene</label>
            <select id="sch-trgt-${i}">
              ${relayOptions}
            </select>
          </div>
        </div>

        <div style="display:grid;grid-template-columns:1fr;gap:10px;margin-bottom:10px;">
          <div class="form-group" style="margin-bottom:0;">
            <label class="form-label">Active Days</label>
            <div style="display:flex;gap:4px;flex-wrap:wrap;padding-top:4px;">
              ${dayNames.map((d, dIdx) => `
                <label style="font-size:11px;display:flex;align-items:center;gap:2px;cursor:pointer;background:#202830;padding:2px 4px;border-radius:3px;">
                  <input type="checkbox" id="sch-day-${i}-${dIdx}" ${(sc.days_mask & (1 << dIdx)) ? 'checked' : ''} style="width:auto;"> ${d}
                </label>
              `).join("")}
            </div>
          </div>
        </div>

        <div style="display:flex;justify-content:flex-end;gap:8px;margin-top:10px;">
          <button class="btn btn-success" style="padding:4px 12px;font-size:11px;" onclick="saveScheduleItem(${i})">Save Schedule</button>
        </div>
      </div>
    `;
  }).join("");

  schedulesCache.forEach((sc, i) => {
    const trgtEl = document.getElementById(`sch-trgt-${i}`);
    if (trgtEl) trgtEl.value = sc.target_relay;
  });
}

function toggleScheduleFields(i) {
  const trig = parseInt(document.getElementById(`sch-trig-${i}`).value);
  const timeWrap = document.getElementById(`sch-time-wrap-${i}`);
  const offsetWrap = document.getElementById(`sch-offset-wrap-${i}`);
  const lbl = document.getElementById(`sch-trig-lbl-${i}`);
  if (trig === 0) {
    if (timeWrap) timeWrap.style.display = "block";
    if (offsetWrap) offsetWrap.style.display = "none";
    if (lbl) lbl.innerText = "Trigger Time (HH:MM)";
  } else {
    if (timeWrap) timeWrap.style.display = "none";
    if (offsetWrap) offsetWrap.style.display = "block";
    if (lbl) lbl.innerText = "Offset Minutes (±)";
  }
}

async function saveScheduleItem(i) {
  const trigType = parseInt(document.getElementById(`sch-trig-${i}`).value) || 0;
  const timeVal = document.getElementById(`sch-time-${i}`).value || "00:00";
  const [h, m] = timeVal.split(":").map(Number);
  const offsetMin = parseInt(document.getElementById(`sch-offset-${i}`) ? document.getElementById(`sch-offset-${i}`).value : 0) || 0;

  let daysMask = 0;
  for (let d = 0; d < 7; d++) {
    if (document.getElementById(`sch-day-${i}-${d}`).checked) {
      daysMask |= (1 << d);
    }
  }

  const body = {
    index: i,
    name: document.getElementById(`sch-name-${i}`).value,
    enabled: document.getElementById(`sch-en-${i}`).checked,
    days_mask: daysMask,
    trigger_type: trigType,
    hour: h || 0,
    minute: m || 0,
    offset_minutes: offsetMin,
    action: parseInt(document.getElementById(`sch-act-${i}`).value),
    target_relay: parseInt(document.getElementById(`sch-trgt-${i}`).value) || 1,
    target_scene: parseInt(document.getElementById(`sch-trgt-${i}`).value) || 1
  };

  await api("/api/schedule/config", "POST", body);
  alert(`Schedule ${i + 1} saved successfully!`);
  loadSchedules();
}

async function loadEnergyStats() {
  const data = await api("/api/energy/status");
  if (!data) return;

  const totalPwrEl = document.getElementById("energy-total-power");
  const totalKwhEl = document.getElementById("energy-total-kwh");
  const activeCountEl = document.getElementById("energy-active-count");
  if (totalPwrEl) totalPwrEl.innerText = (data.total_power_w || 0) + " W";
  if (totalKwhEl) totalKwhEl.innerText = (data.total_kwh ? data.total_kwh.toFixed(3) : "0.000") + " kWh";
  if (activeCountEl && data.relays) {
    const act = data.relays.filter(r => r.state).length;
    activeCountEl.innerText = `${act} / 16`;
  }

  const tbody = document.getElementById("tbody-energy");
  if (!tbody || !data.relays) return;

  tbody.innerHTML = data.relays.map((r) => {
    const icon = DEVICE_ICONS[r.device_icon || 0] || "💡";
    const hp = (r.health_pct !== undefined) ? r.health_pct : 100;
    const hpColor = hp > 80 ? "var(--success)" : (hp > 50 ? "var(--warning)" : "var(--danger)");
    const d = Math.floor((r.on_time_sec || 0) / 86400);
    const h = Math.floor(((r.on_time_sec || 0) % 86400) / 3600);
    const m = Math.floor(((r.on_time_sec || 0) % 3600) / 60);
    const onTimeStr = `${d}d ${h}h ${m}m`;

    return `
      <tr>
        <td>${r.idx}</td>
        <td><strong>${icon} ${r.name}</strong></td>
        <td><span class="tag" style="background:#202830;color:#9ca3af;">${r.room || '-'}</span></td>
        <td>${r.power_w} W</td>
        <td><span class="status-pill ${r.state ? 'online' : 'offline'}">${r.state ? 'ON' : 'OFF'}</span></td>
        <td>🔄 ${r.cycles || 0}</td>
        <td>
          <div style="display:flex;align-items:center;gap:6px;">
            <div style="flex:1;height:8px;background:#202830;border-radius:4px;overflow:hidden;min-width:60px;">
              <div style="width:${hp}%;height:100%;background:${hpColor};"></div>
            </div>
            <span style="font-size:11px;color:${hpColor};font-weight:700;">${hp}%</span>
          </div>
        </td>
        <td>${onTimeStr}</td>
        <td><strong style="color:var(--success);">${(r.kwh || 0).toFixed(3)} kWh</strong></td>
        <td>
          <button class="btn btn-secondary" style="padding:2px 6px;font-size:10px;" onclick="resetEnergy(${r.idx})">Reset</button>
        </td>
      </tr>
    `;
  }).join("");
}

async function resetEnergy(idx) {
  if (!confirm(`Reset cycle counter and energy stats for Relay ${idx}?`)) return;
  const res = await api("/api/energy/reset", "POST", { idx: idx });
  if (res && res.success) {
    loadEnergyStats();
  }
}

async function resetAllEnergy() {
  if (!confirm("Are you sure you want to reset ALL 16 relays' odometer and kWh energy counters?")) return;
  const res = await api("/api/energy/reset", "POST", { all: true });
  if (res && res.success) {
    alert("All relay energy stats reset successfully.");
    loadEnergyStats();
  }
}

let logicRulesCache = [];

async function loadLogicRules() {
  const data = await api("/api/logic/config");
  if (!data || !data.rules) return;
  logicRulesCache = data.rules;
  renderLogicRules();
}

function renderLogicRules() {
  const container = document.getElementById("logic-rules-grid");
  if (!container) return;

  const condTypes = [
    { v: 0, t: "Disabled / None" },
    { v: 1, t: "Input is Active (Pressed)" },
    { v: 2, t: "Input is Inactive (Released)" },
    { v: 3, t: "Relay is ON" },
    { v: 4, t: "Relay is OFF" },
    { v: 5, t: "Solar: Is Night (Post-Sunset)" },
    { v: 6, t: "Solar: Is Day (Post-Sunrise)" }
  ];

  const logicOps = [
    { v: 0, t: "None (Single condition only)" },
    { v: 1, t: "AND (Both conditions true)" },
    { v: 2, t: "OR (Either condition true)" }
  ];

  const actionTypes = [
    { v: 0, t: "None" },
    { v: 1, t: "Turn Relay ON" },
    { v: 2, t: "Turn Relay OFF" },
    { v: 3, t: "Toggle Relay" },
    { v: 4, t: "Activate Scene" },
    { v: 5, t: "Master OFF (All Relays OFF)" },
    { v: 6, t: "Master ON (All Relays ON)" },
    { v: 7, t: "Emergency Stop" }
  ];

  container.innerHTML = logicRulesCache.map((r, i) => {
    return `
      <div class="card" style="background:#161d24;margin-bottom:12px;border:1px solid var(--border);">
        <div style="display:flex;justify-content:space-between;align-items:center;margin-bottom:10px;">
          <div style="display:flex;align-items:center;gap:8px;">
            <span class="tag" style="background:var(--primary);color:#fff;font-weight:700;">RULE ${i + 1}</span>
            <input type="text" id="lr-name-${i}" value="${r.name || ('Rule ' + (i+1))}" style="font-weight:700;font-size:13px;color:#fff;width:160px;">
          </div>
          <label style="font-size:12px;display:flex;align-items:center;gap:4px;cursor:pointer;">
            <input type="checkbox" id="lr-en-${i}" ${r.enabled ? 'checked' : ''} style="width:auto;"> Active
          </label>
        </div>

        <div style="background:#0f1418;padding:10px;border-radius:6px;margin-bottom:8px;border:1px solid #1e2630;">
          <div style="font-size:11px;font-weight:700;color:var(--info);margin-bottom:6px;">IF (CONDITION 1)</div>
          <div style="display:grid;grid-template-columns:1fr 100px;gap:8px;">
            <select id="lr-c1t-${i}" onchange="toggleLogicCondTarget(${i}, 1)">
              ${condTypes.map(c => `<option value="${c.v}" ${r.cond1_type == c.v ? 'selected' : ''}>${c.t}</option>`).join("")}
            </select>
            <input type="number" id="lr-c1tgt-${i}" value="${r.cond1_target || 1}" min="1" max="16" placeholder="Chan (1-16)" title="Channel (1-16)">
          </div>
        </div>

        <div style="display:flex;align-items:center;justify-content:center;gap:8px;margin-bottom:8px;">
          <span style="font-size:11px;color:var(--text-dim);font-weight:700;">OPERATOR:</span>
          <select id="lr-op-${i}" style="width:200px;">
            ${logicOps.map(op => `<option value="${op.v}" ${r.logic_op == op.v ? 'selected' : ''}>${op.t}</option>`).join("")}
          </select>
        </div>

        <div style="background:#0f1418;padding:10px;border-radius:6px;margin-bottom:8px;border:1px solid #1e2630;">
          <div style="font-size:11px;font-weight:700;color:var(--info);margin-bottom:6px;">IF (CONDITION 2 - OPTIONAL)</div>
          <div style="display:grid;grid-template-columns:1fr 100px;gap:8px;">
            <select id="lr-c2t-${i}" onchange="toggleLogicCondTarget(${i}, 2)">
              ${condTypes.map(c => `<option value="${c.v}" ${r.cond2_type == c.v ? 'selected' : ''}>${c.t}</option>`).join("")}
            </select>
            <input type="number" id="lr-c2tgt-${i}" value="${r.cond2_target || 1}" min="1" max="16" placeholder="Chan (1-16)" title="Channel (1-16)">
          </div>
        </div>

        <div style="background:#0f1418;padding:10px;border-radius:6px;margin-bottom:10px;border:1px solid #1e2630;">
          <div style="font-size:11px;font-weight:700;color:var(--success);margin-bottom:6px;">THEN (ACTION ON TRIGGER)</div>
          <div style="display:grid;grid-template-columns:1fr 100px;gap:8px;">
            <select id="lr-act-${i}">
              ${actionTypes.map(a => `<option value="${a.v}" ${r.action == a.v ? 'selected' : ''}>${a.t}</option>`).join("")}
            </select>
            <input type="number" id="lr-acttgt-${i}" value="${r.action_target || 1}" min="1" max="16" placeholder="Target (1-16)" title="Target Relay or Scene (1-16)">
          </div>
        </div>

        <div style="display:flex;justify-content:flex-end;">
          <button class="btn btn-success" style="padding:4px 12px;font-size:11px;" onclick="saveLogicRule(${i})">Save Rule</button>
        </div>
      </div>
    `;
  }).join("");
}

function toggleLogicCondTarget(i, condNum) {
  const typeEl = document.getElementById(`lr-c${condNum}t-${i}`);
  const tgtEl = document.getElementById(`lr-c${condNum}tgt-${i}`);
  if (!typeEl || !tgtEl) return;
  const val = parseInt(typeEl.value);
  if (val === 0 || val === 5 || val === 6) {
    tgtEl.disabled = true;
    tgtEl.style.opacity = "0.5";
  } else {
    tgtEl.disabled = false;
    tgtEl.style.opacity = "1";
  }
}

async function saveLogicRule(i) {
  const body = {
    idx: i + 1,
    enabled: document.getElementById(`lr-en-${i}`).checked,
    name: document.getElementById(`lr-name-${i}`).value,
    cond1_type: parseInt(document.getElementById(`lr-c1t-${i}`).value) || 0,
    cond1_target: parseInt(document.getElementById(`lr-c1tgt-${i}`).value) || 1,
    logic_op: parseInt(document.getElementById(`lr-op-${i}`).value) || 0,
    cond2_type: parseInt(document.getElementById(`lr-c2t-${i}`).value) || 0,
    cond2_target: parseInt(document.getElementById(`lr-c2tgt-${i}`).value) || 1,
    action: parseInt(document.getElementById(`lr-act-${i}`).value) || 0,
    action_target: parseInt(document.getElementById(`lr-acttgt-${i}`).value) || 1
  };
  const res = await api("/api/logic/config", "POST", body);
  if (res && res.success) {
    alert(`Rule ${i + 1} saved successfully!`);
    loadLogicRules();
  }
}

const CITY_PRESETS = {
  "am_yerevan":   { lat: 40.18, lon: 44.51, gmt: 14400, dst: 0 },
  "am_artsakh":   { lat: 39.82, lon: 46.75, gmt: 14400, dst: 0 },
  "ge_tbilisi":   { lat: 41.72, lon: 44.78, gmt: 14400, dst: 0 },
  "ru_moscow":    { lat: 55.75, lon: 37.62, gmt: 10800, dst: 0 },
  "ru_krasnodar": { lat: 45.04, lon: 38.98, gmt: 10800, dst: 0 },
  "ae_dubai":     { lat: 25.20, lon: 55.27, gmt: 14400, dst: 0 },
  "eu_berlin":    { lat: 52.52, lon: 13.40, gmt: 3600,  dst: 0 },
  "uk_london":    { lat: 51.50, lon: -0.12, gmt: 0,     dst: 0 },
  "us_ny":        { lat: 40.71, lon: -74.00, gmt: -18000, dst: 0 },
  "us_la":        { lat: 34.05, lon: -118.24, gmt: -28800, dst: 0 }
};

function onPresetChange(val) {
  if (val === 'custom') return;
  const p = CITY_PRESETS[val];
  if (p) {
    document.getElementById("time-lat").value = p.lat;
    document.getElementById("time-lon").value = p.lon;
    document.getElementById("time-gmt").value = p.gmt;
    document.getElementById("time-dst").value = p.dst;
  }
}

function getBrowserGps() {
  if (!navigator.geolocation) {
    alert("Geolocation is not supported by your browser.");
    return;
  }
  navigator.geolocation.getCurrentPosition(
    (pos) => {
      document.getElementById("time-lat").value = pos.coords.latitude.toFixed(4);
      document.getElementById("time-lon").value = pos.coords.longitude.toFixed(4);
      document.getElementById("time-preset").value = "custom";
      alert(`Coordinates retrieved successfully: Lat ${pos.coords.latitude.toFixed(4)}, Lon ${pos.coords.longitude.toFixed(4)}`);
    },
    (err) => {
      alert("Could not get GPS location: " + err.message);
    }
  );
}

async function loadTimeConfig() {
  const data = await api("/api/time/config");
  if (!data) return;

  if (document.getElementById("time-lat")) document.getElementById("time-lat").value = data.lat !== undefined ? data.lat : 40.18;
  if (document.getElementById("time-lon")) document.getElementById("time-lon").value = data.lon !== undefined ? data.lon : 44.51;
  if (document.getElementById("time-gmt")) document.getElementById("time-gmt").value = data.gmt !== undefined ? data.gmt : 14400;
  if (document.getElementById("time-dst")) document.getElementById("time-dst").value = data.dst !== undefined ? data.dst : 0;
  if (document.getElementById("time-ntp")) document.getElementById("time-ntp").value = data.ntp || "pool.ntp.org";

  if (document.getElementById("astro-sunrise")) document.getElementById("astro-sunrise").innerText = data.sunrise || "--:--";
  if (document.getElementById("astro-sunset")) document.getElementById("astro-sunset").innerText = data.sunset || "--:--";
  if (document.getElementById("astro-noon")) document.getElementById("astro-noon").innerText = data.solar_noon || "--:--";
  if (document.getElementById("astro-length")) document.getElementById("astro-length").innerText = data.day_length || "--:--";

  const phaseEl = document.getElementById("astro-phase");
  if (phaseEl) {
    phaseEl.innerHTML = data.is_night ? `<span class="status-pill offline">🌙 Night</span>` : `<span class="status-pill online">☀️ Day</span>`;
  }

  const presetSel = document.getElementById("time-preset");
  if (presetSel) {
    let matched = "custom";
    for (const [k, v] of Object.entries(CITY_PRESETS)) {
      if (Math.abs(v.lat - data.lat) < 0.1 && Math.abs(v.lon - data.lon) < 0.1 && v.gmt === data.gmt) {
        matched = k;
        break;
      }
    }
    presetSel.value = matched;
  }
}

async function saveTimeSettings() {
  const body = {
    lat: parseFloat(document.getElementById("time-lat").value) || 0.0,
    lon: parseFloat(document.getElementById("time-lon").value) || 0.0,
    gmt: parseInt(document.getElementById("time-gmt").value) || 0,
    dst: parseInt(document.getElementById("time-dst").value) || 0,
    ntp: document.getElementById("time-ntp").value.trim() || "pool.ntp.org"
  };
  const res = await api("/api/time/config", "POST", body);
  if (res && res.success) {
    alert("Timezone, Location & NTP settings saved successfully!");
    loadTimeConfig();
  }
}

async function syncBrowserTime() {
  const epoch = Math.floor(Date.now() / 1000);
  const browserGmtOffset = -new Date().getTimezoneOffset() * 60;
  const res = await api("/api/time/set", "POST", { epoch: epoch, gmt: browserGmtOffset });
  if (res && res.success) {
    alert("Device hardware clock successfully synchronized with browser/phone time!");
    loadTimeConfig();
    const st = await api("/api/status");
    if (st) handleStateUpdate(st);
  }
}

async function resyncNtp() {
  const res = await api("/api/time/sync_ntp", "POST");
  if (res && res.success) {
    alert("NTP synchronization triggered.");
    setTimeout(async () => {
      loadTimeConfig();
      const st = await api("/api/status");
      if (st) handleStateUpdate(st);
    }, 1500);
  }
}

async function applyManualTime() {
  const inp = document.getElementById("time-manual-input");
  if (!inp || !inp.value) {
    alert("Please select a date and time first.");
    return;
  }
  const dt = new Date(inp.value);
  if (isNaN(dt.getTime())) {
    alert("Invalid date/time value.");
    return;
  }
  const epoch = Math.floor(dt.getTime() / 1000);
  const res = await api("/api/time/set", "POST", { epoch: epoch });
  if (res && res.success) {
    alert("Manual date and time set successfully!");
    loadTimeConfig();
    const st = await api("/api/status");
    if (st) handleStateUpdate(st);
  }
}

setInterval(() => {
  const hClock = document.getElementById("header-clock");
  if (!hClock || hClock.innerText === "--:--:--" || !hClock.innerText.includes(":")) return;
  const parts = hClock.innerText.split(":").map(Number);
  if (parts.length === 3 && !isNaN(parts[0]) && !isNaN(parts[1]) && !isNaN(parts[2])) {
    let s = parts[2] + 1;
    let m = parts[1];
    let h = parts[0];
    if (s >= 60) { s = 0; m++; }
    if (m >= 60) { m = 0; h++; }
    if (h >= 24) { h = 0; }
    hClock.innerText = `${String(h).padStart(2,'0')}:${String(m).padStart(2,'0')}:${String(s).padStart(2,'0')}`;
  }
}, 1000);

window.addEventListener("DOMContentLoaded", () => {
  checkInit();
});
</script>
</body>
</html>
)rawliteral";
