#include "astro_clock.h"
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#ifndef DEG_TO_RAD
#define DEG_TO_RAD (M_PI / 180.0)
#endif

#ifndef RAD_TO_DEG
#define RAD_TO_DEG (180.0 / M_PI)
#endif


AstroClock Astro;

AstroClock::AstroClock() : sunrise_min(360), sunset_min(1140), last_calc_day(0) {}

void AstroClock::calculateSolarTimes(int day_of_year, float lat, float lon, float tz_hours, uint16_t &out_sunrise, uint16_t &out_sunset) {
    // Standard NOAA calculation for sunrise and sunset
    float gamma = (2.0 * M_PI / 365.0) * (day_of_year - 1); // Fractional year in radians

    // Equation of time in minutes
    float eqtime = 229.18 * (0.000075 + 0.001868 * cos(gamma) - 0.032077 * sin(gamma)
                   - 0.014615 * cos(2.0 * gamma) - 0.040849 * sin(2.0 * gamma));

    // Solar declination angle in radians
    float decl = 0.006918 - 0.399912 * cos(gamma) + 0.070257 * sin(gamma)
                 - 0.006758 * cos(2.0 * gamma) + 0.000907 * sin(2.0 * gamma)
                 - 0.002697 * cos(3.0 * gamma) + 0.00148 * sin(3.0 * gamma);

    float lat_rad = lat * DEG_TO_RAD;

    // Zenith for official sunrise/sunset is 90.833 degrees (accounting for atmospheric refraction)
    float zenith_rad = 90.833 * DEG_TO_RAD;

    float cos_ha = (cos(zenith_rad) / (cos(lat_rad) * cos(decl))) - (tan(lat_rad) * tan(decl));

    if (cos_ha > 1.0) {
        // Polar night: Sun never rises
        out_sunrise = 720;
        out_sunset = 720;
        return;
    }
    if (cos_ha < -1.0) {
        // Polar day: Sun never sets
        out_sunrise = 0;
        out_sunset = 1439;
        return;
    }

    float ha_deg = acos(cos_ha) * RAD_TO_DEG; // Hour angle in degrees

    // Solar noon in minutes from midnight UTC
    float solar_noon = 720.0 - (4.0 * lon) - eqtime;

    // Sunrise and sunset in UTC minutes
    float sunrise_utc = solar_noon - (ha_deg * 4.0);
    float sunset_utc = solar_noon + (ha_deg * 4.0);

    // Convert to local time
    float local_sunrise = sunrise_utc + (tz_hours * 60.0);
    float local_sunset = sunset_utc + (tz_hours * 60.0);

    // Normalize to 0..1439
    while (local_sunrise < 0.0) local_sunrise += 1440.0;
    while (local_sunrise >= 1440.0) local_sunrise -= 1440.0;

    while (local_sunset < 0.0) local_sunset += 1440.0;
    while (local_sunset >= 1440.0) local_sunset -= 1440.0;

    out_sunrise = (uint16_t)round(local_sunrise);
    out_sunset = (uint16_t)round(local_sunset);
}

void AstroClock::update(time_t epoch, float lat, float lon, int32_t gmt_offset_s) {
    if (epoch < 100000) return; // Time not valid

    struct tm timeinfo;
    if (!localtime_r(&epoch, &timeinfo)) return;

    time_t day_id = epoch / 86400;
    if (day_id != last_calc_day) {
        last_calc_day = day_id;
        float tz_hours = (float)gmt_offset_s / 3600.0f;
        calculateSolarTimes(timeinfo.tm_yday + 1, lat, lon, tz_hours, sunrise_min, sunset_min);
    }
}

bool AstroClock::isNight() const {
    time_t now = time(nullptr);
    struct tm timeinfo;
    if (!localtime_r(&now, &timeinfo)) return false;

    uint16_t current_min = timeinfo.tm_hour * 60 + timeinfo.tm_min;
    if (sunrise_min < sunset_min) {
        // Normal day: Night is before sunrise OR after sunset
        return (current_min < sunrise_min || current_min >= sunset_min);
    } else {
        // Sunset wrapped past midnight
        return (current_min >= sunset_min && current_min < sunrise_min);
    }
}

bool AstroClock::isNight(time_t epoch, float lat, float lon, int32_t gmt_offset_s) {
    update(epoch, lat, lon, gmt_offset_s);
    return isNight();
}

String AstroClock::getSunriseFormatted() const {
    char buf[8];
    snprintf(buf, sizeof(buf), "%02u:%02u", sunrise_min / 60, sunrise_min % 60);
    return String(buf);
}

String AstroClock::getSunsetFormatted() const {
    char buf[8];
    snprintf(buf, sizeof(buf), "%02u:%02u", sunset_min / 60, sunset_min % 60);
    return String(buf);
}

String AstroClock::getSolarNoonFormatted() const {
    uint16_t noon = (sunrise_min + sunset_min) / 2;
    char buf[8];
    snprintf(buf, sizeof(buf), "%02u:%02u", noon / 60, noon % 60);
    return String(buf);
}

String AstroClock::getDayLengthFormatted() const {
    uint16_t len = (sunset_min > sunrise_min) ? (sunset_min - sunrise_min) : 0;
    char buf[12];
    snprintf(buf, sizeof(buf), "%uh %02um", len / 60, len % 60);
    return String(buf);
}

