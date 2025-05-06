#ifndef RIDESTATS_HEAD
#define RIDESTATS_HEAD

#include <Arduino.h>

class RideStats
{
public:
    void init();
    void update();

    float getAccelTime(int threshold_index, int position)
    {
        if (threshold_index >= 0 && threshold_index < 10 &&
            position >= 0 && position < 10)
        {
            return best_accel_times[threshold_index][position];
        }
        return 0.0f;
    }

    // New method to get threshold value
    int getThresholdSpeed(int index)
    {
        const int thresholds[] = {10, 20, 30, 40, 50, 60, 70, 80, 90, 100};
        if (index >= 0 && index < 10)
        {
            return thresholds[index];
        }
        return 0;
    }

    float distance_driven_trip_m;
    float distance_driven_total_m;
    int max_rpm;
    unsigned long last_distance_update_time_us;
    unsigned long hall_pulse_last_us = 0;
    unsigned long hall_pulse_previous_us;

    unsigned long last_save_time = 0;
    float last_save_distance = 0;

    bool accel_measuring;
    bool accel_started;
    float last_speed_mps;
    unsigned long last_accel_update_time_us;
    unsigned long accel_start_time;
    float max_accel_mps2;
    float best_accel_times[10][10] = {{0}};

private:
    int findPositionInArray(float arr[], int size, float value);

    bool timing_active = false;
    bool threshold_reached[10] = {false};

    int best_accel_times_index;
    const float DISTANCE_UPDATE_INTERVAL_s = 1;
};

#endif