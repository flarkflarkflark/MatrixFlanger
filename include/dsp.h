#pragma once

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// Filter types
typedef enum {
    FILTER_TYPE_LOWPASS = 0,
    FILTER_TYPE_HIGHPASS = 1,
    FILTER_TYPE_BANDPASS = 2,
    FILTER_TYPE_NOTCH = 3,
    FILTER_TYPE_PEAKING = 4,
    FILTER_TYPE_LOWSHELF = 5,
    FILTER_TYPE_HIGHSHELF = 6
} filter_type_t;

// Filter structure
typedef struct {
    // Filter parameters
    filter_type_t type;
    float cutoff_freq;
    float resonance;
    float gain;
    float sample_rate;
    
    // Internal filter state (for IIR filters)
    float x1, x2;  // input delays
    float y1, y2;  // output delays
    
    // Biquad coefficients (for enhanced filters)
    float a0, a1, a2, b0, b1, b2;
    
    // Filter is initialized
    bool initialized;
} filter_t;

// Initialize filter with parameters
void filter_init(filter_t *filter, filter_type_t type, float cutoff_freq, float resonance, float gain, float sample_rate);

// Update filter parameters
void filter_set_parameters(filter_t *filter, filter_type_t type, float cutoff_freq, float resonance, float gain);

// Set sample rate
void filter_set_sample_rate(filter_t *filter, float sample_rate);

// Process single sample
float filter_process_sample(filter_t *filter, float input);

// Process block of samples
void filter_process_block(filter_t *filter, const float *input, float *output, uint32_t frames);

// Reset filter state
void filter_reset(filter_t *filter);

// Calculate frequency response at given frequency (for visualization)
void filter_get_frequency_response(filter_t *filter, float frequency, float *magnitude_db, float *phase_deg);

// Utility functions
float freq_to_omega(float frequency, float sample_rate);
float db_to_gain(float db);
float gain_to_db(float gain);
float clampf(float value, float min_val, float max_val);

#ifdef __cplusplus
}
#endif