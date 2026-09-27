#pragma once

#include <Arduino.h>
#include <time.h>

class AstroClock {
public:
    AstroClock();

    // Recalculates sunrise and sunset for the current day
    void update(time_t epoch, float lat, float lon, int32_t gmt_offset_s);

    uint16_t getSunriseMinute() const { return sunrise_min; }
    uint16_t getSunsetMinute() const { return sunset_min; }

    bool isNight() const;
    bool isNight(time_t epoch, float lat, float lon, int32_t gmt_offset_s);

    String getSunriseFormatted() const;
    String getSunsetFormatted() const;
    String getSolarNoonFormatted() const;
    String getDayLengthFormatted() const;

private:
    uint16_t sunrise_min; // Minute of day (0..1439)
    uint16_t sunset_min;  // Minute of day (0..1439)
    time_t last_calc_day; // Track which day we calculated for

    void calculateSolarTimes(int day_of_year, float lat, float lon, float tz_hours, uint16_t &out_sunrise, uint16_t &out_sunset);
};

extern AstroClock Astro;
