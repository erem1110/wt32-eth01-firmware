#pragma once

#include <Arduino.h>
#include "types.h"

#define MAX_ACTIVE_SESSIONS 4

struct SessionRecord {
    bool active;
    char token[33];
    unsigned long expire_ms;
};

class AuthManager {
public:
    AuthManager();
    void begin();
    void update();

    bool isConfigured() const { return config.is_configured; }
    bool isLockedOut() const;
    uint32_t getRemainingLockoutSeconds() const;

    bool setupInitialAdmin(const char *username, const char *password, uint32_t session_timeout_s);
    bool authenticate(const char *username, const char *password, String &out_session, String &err);
    bool changeCredentials(const char *old_password, const char *new_username, const char *new_password);
    void logout(const String &token);
    bool validateSession(const String &token);

    const SecurityConfig& getConfig() const { return config; }
    void updateConfig(const SecurityConfig &cfg);

    static void computeHash(const char *salt, const char *password, char *out_hash_hex);

private:
    SecurityConfig config;
    SessionRecord sessions[MAX_ACTIVE_SESSIONS];
    uint8_t failed_login_count;
    unsigned long lockout_until_ms;

    void generateRandomHex(char *buf, size_t num_bytes);
    String createSessionToken();
};

extern AuthManager Auth;
