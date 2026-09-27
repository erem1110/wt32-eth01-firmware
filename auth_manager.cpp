#include "auth_manager.h"
#include "storage.h"
#include "event_log.h"
#include <mbedtls/sha256.h>
#include <esp_system.h>

AuthManager Auth;

AuthManager::AuthManager() : failed_login_count(0), lockout_until_ms(0) {
    for (uint8_t i = 0; i < MAX_ACTIVE_SESSIONS; ++i) {
        sessions[i].active = false;
        sessions[i].token[0] = '\0';
        sessions[i].expire_ms = 0;
    }
}

void AuthManager::begin() {
    Storage.loadSecurity(config);
}

void AuthManager::generateRandomHex(char *buf, size_t num_bytes) {
    for (size_t i = 0; i < num_bytes; ++i) {
        uint8_t r = (uint8_t)esp_random();
        sprintf(buf + (i * 2), "%02x", r);
    }
    buf[num_bytes * 2] = '\0';
}

void AuthManager::computeHash(const char *salt, const char *password, char *out_hash_hex) {
    mbedtls_sha256_context ctx;
    mbedtls_sha256_init(&ctx);
    mbedtls_sha256_starts_ret(&ctx, 0);
    mbedtls_sha256_update_ret(&ctx, (const unsigned char*)salt, strlen(salt));
    mbedtls_sha256_update_ret(&ctx, (const unsigned char*)password, strlen(password));
    unsigned char output[32];
    mbedtls_sha256_finish_ret(&ctx, output);
    mbedtls_sha256_free(&ctx);

    for (int i = 0; i < 32; ++i) {
        sprintf(out_hash_hex + (i * 2), "%02x", output[i]);
    }
    out_hash_hex[64] = '\0';
}

bool AuthManager::isLockedOut() const {
    if (lockout_until_ms == 0) return false;
    return (long)(lockout_until_ms - millis()) > 0;
}

uint32_t AuthManager::getRemainingLockoutSeconds() const {
    if (!isLockedOut()) return 0;
    return (uint32_t)((lockout_until_ms - millis() + 999UL) / 1000UL);
}

bool AuthManager::setupInitialAdmin(const char *username, const char *password, uint32_t session_timeout_s) {
    if (config.is_configured) return false;
    if (!username || !password || strlen(username) < 3 || strlen(password) < 4) return false;

    strncpy(config.username, username, sizeof(config.username) - 1);
    config.username[sizeof(config.username) - 1] = '\0';

    generateRandomHex(config.salt, 16);
    computeHash(config.salt, password, config.pass_hash);

    config.session_timeout_s = (session_timeout_s > 60) ? session_timeout_s : DEFAULT_SESSION_EXP;
    config.is_configured = true;

    Storage.saveSecurity(config);
    EventLog.log(SUBSYS_SECURITY, "Initial setup completed for user: %s", config.username);
    return true;
}

String AuthManager::createSessionToken() {
    char token[33];
    generateRandomHex(token, 16);

    uint8_t slot = 0;
    unsigned long oldest_expire = 0xFFFFFFFF;
    for (uint8_t i = 0; i < MAX_ACTIVE_SESSIONS; ++i) {
        if (!sessions[i].active) {
            slot = i;
            break;
        }
        if (sessions[i].expire_ms < oldest_expire) {
            oldest_expire = sessions[i].expire_ms;
            slot = i;
        }
    }

    sessions[slot].active = true;
    strncpy(sessions[slot].token, token, sizeof(sessions[slot].token));
    sessions[slot].expire_ms = millis() + ((unsigned long)config.session_timeout_s * 1000UL);

    return String(token);
}

bool AuthManager::authenticate(const char *username, const char *password, String &out_session, String &err) {
    if (!config.is_configured) {
        err = "Device not configured";
        return false;
    }

    if (isLockedOut()) {
        err = "Temporarily locked out. Please wait.";
        EventLog.log(SUBSYS_SECURITY, "Login attempt during lockout");
        return false;
    }

    if (!username || !password) {
        err = "Invalid input";
        return false;
    }

    char entered_hash[65];
    computeHash(config.salt, password, entered_hash);

    if (strcmp(config.username, username) == 0 && strcmp(config.pass_hash, entered_hash) == 0) {
        failed_login_count = 0;
        out_session = createSessionToken();
        EventLog.log(SUBSYS_SECURITY, "Successful login for %s", username);
        return true;
    }

    failed_login_count++;
    EventLog.log(SUBSYS_SECURITY, "Failed login attempt (%u/%u)", failed_login_count, config.lockout_attempts);

    if (failed_login_count >= config.lockout_attempts) {
        lockout_until_ms = millis() + ((unsigned long)config.lockout_time_s * 1000UL);
        EventLog.log(SUBSYS_SECURITY, "Too many failed attempts. Lockout activated for %u s", config.lockout_time_s);
        err = "Account locked out due to multiple failed attempts";
    } else {
        err = "Invalid username or password";
    }

    return false;
}

bool AuthManager::changeCredentials(const char *old_password, const char *new_username, const char *new_password) {
    if (!config.is_configured) return false;

    char cur_hash[65];
    computeHash(config.salt, old_password, cur_hash);
    if (strcmp(config.pass_hash, cur_hash) != 0) {
        EventLog.log(SUBSYS_SECURITY, "Password change rejected: incorrect current password");
        return false;
    }

    if (new_username && strlen(new_username) >= 3) {
        strncpy(config.username, new_username, sizeof(config.username) - 1);
        config.username[sizeof(config.username) - 1] = '\0';
    }

    if (new_password && strlen(new_password) >= 4) {
        generateRandomHex(config.salt, 16);
        computeHash(config.salt, new_password, config.pass_hash);
    }

    Storage.saveSecurity(config);
    EventLog.log(SUBSYS_SECURITY, "Credentials updated successfully");
    return true;
}

void AuthManager::logout(const String &token) {
    for (uint8_t i = 0; i < MAX_ACTIVE_SESSIONS; ++i) {
        if (sessions[i].active && token.equals(sessions[i].token)) {
            sessions[i].active = false;
            sessions[i].token[0] = '\0';
            break;
        }
    }
}

bool AuthManager::validateSession(const String &token) {
    if (!config.is_configured) return false;
    if (token.length() != 32) return false;

    unsigned long now = millis();
    for (uint8_t i = 0; i < MAX_ACTIVE_SESSIONS; ++i) {
        if (sessions[i].active && token.equals(sessions[i].token)) {
            if ((long)(now - sessions[i].expire_ms) < 0) {
                // Refresh sliding session window
                sessions[i].expire_ms = now + ((unsigned long)config.session_timeout_s * 1000UL);
                return true;
            } else {
                sessions[i].active = false;
                sessions[i].token[0] = '\0';
            }
        }
    }
    return false;
}

void AuthManager::updateConfig(const SecurityConfig &cfg) {
    config.session_timeout_s = cfg.session_timeout_s;
    config.lockout_attempts = cfg.lockout_attempts;
    config.lockout_time_s = cfg.lockout_time_s;
    Storage.saveSecurity(config);
    EventLog.log(SUBSYS_SECURITY, "Security settings updated");
}

void AuthManager::update() {
    if (lockout_until_ms > 0 && !isLockedOut()) {
        lockout_until_ms = 0;
        failed_login_count = 0;
        EventLog.log(SUBSYS_SECURITY, "Lockout period ended");
    }
}
