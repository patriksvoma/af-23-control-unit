#include "rideStats.h"
#include "motor.h"
#include "storage.h"



void RideStats::init() {
    // load previous total distance from memory
    last_speed_mps = 0;
    last_accel_update_time_us = 0;
    best_accel_times_index = 0;
    // Initialize all best acceleration times to 0


    storage::loadTotalDistance(&distance_driven_total_m);
}

void RideStats::update() {
    int rpm = motor::getRPM();
    float diff_rpm = (float)rpm * BELT_GEAR_RATIO;
    float wheel_rpm = (float)diff_rpm * DIFF_GEAR_RATIO;
    float speed_mps = wheel_rpm * (WHEEL_CIRCUMFERENCE_mm / 1000.0) / 60.0;
    
    // Distance calculation (existing code)
    if (micros() - last_distance_update_time_us > DISTANCE_UPDATE_INTERVAL_s * 1000000) {
        unsigned long time = micros();
        double time_delta_s = (time - last_distance_update_time_us) / 1000000.0;
        double distance_delta_m = speed_mps * time_delta_s;
        distance_driven_trip_m += distance_delta_m;
        distance_driven_total_m += distance_delta_m;
        last_distance_update_time_us = time;
        
        if (millis() - last_save_time > 60000 || distance_driven_trip_m - last_save_distance > 100) {
            storage::saveTotalDistance(distance_driven_total_m);
            last_save_time = millis();
            last_save_distance = distance_driven_trip_m;
        }
    }
    
    if (rpm > max_rpm) max_rpm = rpm;
    
    // Acceleration calculation
    unsigned long current_time = micros();
    
    // Calculate basic acceleration
    if (last_accel_update_time_us > 0) {
        float time_delta = (current_time - last_accel_update_time_us) / 1000000.0f;
        if (time_delta > 0.01f && time_delta < 0.5f) {
            float accel = (speed_mps - last_speed_mps) / time_delta;
            
            // Track max acceleration with reasonable limits
            if (accel > 0.1f && accel < 30.0f && accel > max_accel_mps2) {
                max_accel_mps2 = accel;
            }
        }
    }
    
    // Define speed thresholds in km/h
    const float thresholds_kmh[] = {10, 20, 30, 40, 50, 60, 70, 80, 90, 100};
    const int num_thresholds = sizeof(thresholds_kmh) / sizeof(thresholds_kmh[0]);
    
    // Convert to m/s for internal calculations
    static float thresholds_mps[10];
    static bool thresholds_initialized = false;
    
    if (!thresholds_initialized) {
        for (int i = 0; i < num_thresholds; i++) {
            thresholds_mps[i] = thresholds_kmh[i] / 3.6f;  // km/h to m/s
        }
        thresholds_initialized = true;
    }
    
    // Start a new timing run if speed drops below threshold
    if (speed_mps < 1.39f) {  // Below 5 km/h
        accel_start_time = current_time;
        timing_active = true;
        
        // Reset threshold flags for the new run
        for (int i = 0; i < num_thresholds; i++) {
            threshold_reached[i] = false;
        }
        
        Serial.println("Starting new acceleration timing run");
    }
    
    // Check for threshold crossings if we're in an active run
    if (timing_active) {
        for (int i = 0; i < num_thresholds; i++) {
            // Only check thresholds we haven't reached yet in this run
            if (!threshold_reached[i] && last_speed_mps < thresholds_mps[i] && speed_mps >= thresholds_mps[i]) {
                // We just crossed this threshold
                float accel_time = (current_time - accel_start_time) / 1000000.0f;
                threshold_reached[i] = true;
                
                Serial.printf("Crossed %d km/h threshold in %.2f seconds\n", 
                             (int)thresholds_kmh[i], accel_time);
                
                // Only record reasonable times
                if (accel_time > 0.1f && accel_time < 30.0f) {
                    // Record this time if it's better than the worst or if there's an empty slot
                    if (best_accel_times[i][9] == 0.0f || accel_time < best_accel_times[i][9]) {
                        best_accel_times[i][9] = accel_time;
                        
                        // Sort the times for this threshold (simple bubble sort)
                        for (int j = 9; j > 0; j--) {
                            if (best_accel_times[i][j-1] == 0.0f || 
                                best_accel_times[i][j] < best_accel_times[i][j-1]) {
                                // Swap values
                                float temp = best_accel_times[i][j];
                                best_accel_times[i][j] = best_accel_times[i][j-1];
                                best_accel_times[i][j-1] = temp;
                            } else {
                                // Array is sorted from here
                                break;
                            }
                        }
                        
                        Serial.printf("New 0-%d km/h time recorded: %.2f s (position #%d)\n", 
                                    (int)thresholds_kmh[i], accel_time, 
                                    findPositionInArray(best_accel_times[i], 10, accel_time));
                    }
                }
            }
        }
        
        // End the timing run if we've been at a stable speed for too long
        // or if we've reached all thresholds
        static unsigned long stable_speed_start = 0;
        static float last_recorded_speed = 0;
        
        if (fabs(speed_mps - last_recorded_speed) < 0.5f) {
            // Speed hasn't changed much
            if (stable_speed_start == 0) {
                stable_speed_start = current_time;
            } else if (current_time - stable_speed_start > 3000000) {  // 3 seconds of stable speed
                timing_active = false;
                stable_speed_start = 0;
                Serial.println("Ending timing run due to stable speed");
            }
        } else {
            // Speed is changing, reset stability timer
            stable_speed_start = 0;
            last_recorded_speed = speed_mps;
        }
        
        // Also end timing if we've been timing for too long
        if (current_time - accel_start_time > 60000000) {  // 60 seconds max
            timing_active = false;
            Serial.println("Ending timing run due to timeout");
        }
    }
    
    // Update for next cycle
    last_speed_mps = speed_mps;
    last_accel_update_time_us = current_time;
}

// Helper function to find position of a value in a sorted array
int RideStats::findPositionInArray(float arr[], int size, float value) {
    for (int i = 0; i < size; i++) {
        if (arr[i] == value) return i + 1;  // 1-based position
    }
    return -1;  // Not found
}