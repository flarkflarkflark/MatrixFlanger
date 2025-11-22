#include "gui.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// Matrix initialization
void matrix_init(gui_context_t *gui) {
    srand((unsigned int)time(NULL));
    
    for (int i = 0; i < MATRIX_WIDTH; ++i) {
        gui->columns[i].x = i;
        gui->columns[i].y = -rand() % MATRIX_HEIGHT;
        gui->columns[i].speed = 0.5f + (rand() % 100) / 100.0f;
        gui->columns[i].current_char = MATRIX_CHARS[rand() % (sizeof(MATRIX_CHARS) - 1)];
        gui->columns[i].brightness = 0.0f;
        gui->columns[i].target_brightness = 0.0f;
    }
    
    gui->time_accumulator = 0.0f;
}

// Update matrix animation
void matrix_update(gui_context_t *gui, float delta_time) {
    gui->time_accumulator += delta_time;
    
    for (int i = 0; i < MATRIX_WIDTH; ++i) {
        matrix_column_t *col = &gui->columns[i];
        
        // Update position
        col->y += col->speed * delta_time * 60.0f; // Normalize to 60fps
        
        // Reset when off screen
        if (col->y >= MATRIX_HEIGHT) {
            col->y = -1;
            col->speed = 0.5f + (rand() % 100) / 100.0f;
            col->current_char = MATRIX_CHARS[rand() % (sizeof(MATRIX_CHARS) - 1)];
        }
        
        // Update brightness based on spectrum data
        if (gui->spectrum.spectrum[0] > 0.01f) {
            float normalized_freq = (float)i / (float)MATRIX_WIDTH;
            uint32_t spectrum_index = (uint32_t)(normalized_freq * (MAX_FREQUENCY_BINS - 1));
            col->target_brightness = gui->spectrum.spectrum[spectrum_index] * 2.0f;
        } else {
            col->target_brightness = 0.0f;
        }
        
        // Smooth brightness transition
        col->brightness += (col->target_brightness - col->brightness) * 0.1f;
        
        // Change character occasionally
        if (col->brightness > 0.5f && ((rand() % 1000) == 0)) {
            col->current_char = MATRIX_CHARS[rand() % (sizeof(MATRIX_CHARS) - 1)];
        }
    }
}

// Spectrum analyzer initialization
void spectrum_init(spectrum_analyzer_t *spectrum) {
    memset(spectrum, 0, sizeof(spectrum_analyzer_t));
    spectrum->sample_count = 0;
}

// Simple FFT (Fast Fourier Transform) implementation
static void fft_r2(float *real, float *imag, int n) {
    if (n <= 1) return;
    
    // Bit reversal
    for (int i = 1, j = 0; i < n; i++) {
        int bit = n >> 1;
        for (; j & bit; bit >>= 1) {
            j ^= bit;
        }
        j ^= bit;
        
        if (i < j) {
            float temp = real[i];
            real[i] = real[j];
            real[j] = temp;
            temp = imag[i];
            imag[i] = imag[j];
            imag[j] = temp;
        }
    }
    
    // FFT computation
    for (int len = 2; len <= n; len <<= 1) {
        float ang = -2.0f * M_PI / len;
        float wlen_real = cosf(ang);
        float wlen_imag = sinf(ang);
        
        for (int i = 0; i < n; i += len) {
            float w_real = 1.0f;
            float w_imag = 0.0f;
            
            for (int j = 0; j < len / 2; j++) {
                float u_real = real[i + j];
                float u_imag = imag[i + j];
                float v_real = real[i + j + len / 2] * w_real - imag[i + j + len / 2] * w_imag;
                float v_imag = real[i + j + len / 2] * w_imag + imag[i + j + len / 2] * w_real;
                
                real[i + j] = u_real + v_real;
                imag[i + j] = u_imag + v_imag;
                real[i + j + len / 2] = u_real - v_real;
                imag[i + j + len / 2] = u_imag - v_imag;
                
                float temp_real = w_real * wlen_real - w_imag * wlen_imag;
                float temp_imag = w_real * wlen_imag + w_imag * wlen_real;
                w_real = temp_real;
                w_imag = temp_imag;
            }
        }
    }
}

// Analyze audio spectrum
void spectrum_analyze(spectrum_analyzer_t *spectrum, const float *audio_data, uint32_t frames) {
    if (frames < MAX_FREQUENCY_BINS) return;
    
    // Use Hamming window
    static float window[MAX_FREQUENCY_BINS];
    static bool window_initialized = false;
    
    if (!window_initialized) {
        for (int i = 0; i < MAX_FREQUENCY_BINS; i++) {
            window[i] = 0.54f - 0.46f * cosf(2.0f * M_PI * i / (MAX_FREQUENCY_BINS - 1));
        }
        window_initialized = true;
    }
    
    // Prepare data for FFT
    float real[MAX_FREQUENCY_BINS];
    float imag[MAX_FREQUENCY_BINS];
    
    for (int i = 0; i < MAX_FREQUENCY_BINS; i++) {
        real[i] = audio_data[i] * window[i];
        imag[i] = 0.0f;
    }
    
    // Perform FFT
    fft_r2(real, imag, MAX_FREQUENCY_BINS);
    
    // Calculate magnitude spectrum
    for (int i = 0; i < MAX_FREQUENCY_BINS / 2; i++) {
        float magnitude = sqrtf(real[i] * real[i] + imag[i] * imag[i]) / MAX_FREQUENCY_BINS;
        spectrum->spectrum[i] = magnitude;
    }
    
    // Apply some smoothing
    for (int i = 0; i < MAX_FREQUENCY_BINS / 2; i++) {
        if (i > 0) {
            spectrum->spectrum[i] = (spectrum->spectrum[i] * 0.7f) + (spectrum->spectrum[i-1] * 0.3f);
        }
    }
}

// Update peak values for decay effect
void spectrum_update_peaks(spectrum_analyzer_t *spectrum) {
    for (int i = 0; i < MAX_FREQUENCY_BINS / 2; i++) {
        if (spectrum->spectrum[i] > spectrum->peak_values[i]) {
            spectrum->peak_values[i] = spectrum->spectrum[i];
        } else {
            spectrum->peak_values[i] *= 0.95f; // Decay
        }
    }
}

// Handle audio data from plugin
void gui_handle_audio_data(gui_context_t *gui, const float *audio_data, uint32_t frames) {
    pthread_mutex_lock(&gui->mutex);
    
    if (frames >= MAX_FREQUENCY_BINS) {
        spectrum_analyze(&gui->spectrum, audio_data, frames);
        spectrum_update_peaks(&gui->spectrum);
    }
    
    pthread_mutex_unlock(&gui->mutex);
}

// OpenGL initialization
void opengl_init() {
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

// Setup orthographic projection
void opengl_setup_projection(uint32_t width, uint32_t height) {
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, MATRIX_WIDTH, 0, MATRIX_HEIGHT, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glViewport(0, 0, width, height);
}

// Clear screen with dark blue background
void opengl_clear_screen() {
    glClearColor(0.0f, 0.1f, 0.2f, 1.0f);  // Dark blue-black background
    glClear(GL_COLOR_BUFFER_BIT);
}

// Draw a single character in matrix style with enhanced blue theme
void opengl_draw_character(float x, float y, float size, char c, float brightness) {
    if (brightness <= 0.01f) return;
    
    // Enhanced blue matrix color with depth
    float alpha = brightness;
    float intensity_modifier = 0.8f + (brightness * 0.2f);
    
    // Primary blue color (deeper blue with slight green for depth)
    float r = 0.0f;
    float g = 0.05f * intensity_modifier;
    float b = alpha * intensity_modifier;
    
    // Draw main character
    glColor4f(r, g, b, alpha);
    glBegin(GL_QUADS);
    glVertex2f(x, y);
    glVertex2f(x + size, y);
    glVertex2f(x + size, y + size);
    glVertex2f(x, y + size);
    glEnd();
    
    // Enhanced glow effect with multiple layers
    if (brightness > 0.3f) {
        // Inner glow - brighter blue
        glColor4f(r * 0.3f, g * 0.3f, b * 0.7f, alpha * 0.4f);
        for (int i = 1; i <= 1; i++) {
            glBegin(GL_QUADS);
            glVertex2f(x - i*0.08f, y - i*0.08f);
            glVertex2f(x + size + i*0.08f, y - i*0.08f);
            glVertex2f(x + size + i*0.08f, y + size + i*0.08f);
            glVertex2f(x - i*0.08f, y + size + i*0.08f);
            glEnd();
        }
        
        // Outer glow - pale blue
        glColor4f(r * 0.6f, g * 0.6f, b * 1.0f, alpha * 0.15f);
        for (int i = 2; i <= 3; i++) {
            glBegin(GL_QUADS);
            glVertex2f(x - i*0.12f, y - i*0.12f);
            glVertex2f(x + size + i*0.12f, y - i*0.12f);
            glVertex2f(x + size + i*0.12f, y + size + i*0.12f);
            glVertex2f(x - i*0.12f, y + size + i*0.12f);
            glEnd();
        }
    }
    
    // Add subtle depth shadow for high brightness characters
    if (brightness > 0.7f) {
        glColor4f(0.0f, 0.0f, 0.1f, alpha * 0.2f);
        glBegin(GL_QUADS);
        glVertex2f(x + size*0.05f, y + size*0.05f);
        glVertex2f(x + size + size*0.05f, y + size*0.05f);
        glVertex2f(x + size + size*0.05f, y + size + size*0.05f);
        glVertex2f(x + size*0.05f, y + size + size*0.05f);
        glEnd();
    }
}

// Render the enhanced matrix visualization with blue theme UI elements
void matrix_render(gui_context_t *gui) {
    opengl_clear_screen();
    
    float cell_size = 1.0f;
    
    // Draw a subtle gradient background for visual depth
    draw_background_gradient(gui);
    
    // Draw audio spectrum visualization at the bottom
    draw_audio_spectrum_visualization(gui);
    
    for (int i = 0; i < MATRIX_WIDTH; ++i) {
        matrix_column_t *col = &gui->columns[i];
        
        if (col->y >= 0 && col->y < MATRIX_HEIGHT) {
            int y_pos = (int)col->y;
            
            // Enhanced trailing characters with better depth
            for (int trail = 0; trail < 6; trail++) {
                float trail_y = y_pos - trail * 0.75f;
                float trail_brightness = col->brightness * (1.0f - trail * 0.15f);
                float trail_size = cell_size * (0.8f - trail * 0.05f);
                
                if (trail_y >= 0 && trail_y < MATRIX_HEIGHT && trail_size > 0.3f) {
                    opengl_draw_character(col->x, trail_y, trail_size, 
                                        col->current_char, trail_brightness);
                }
            }
        }
    }
    
    // Draw UI overlay elements (control indicators, status)
    draw_ui_overlay_elements(gui);
}

// Draw subtle background gradient for depth
static void draw_background_gradient(gui_context_t *gui) {
    // Create a subtle vertical gradient from dark blue to slightly lighter blue
    const int gradient_steps = 8;
    for (int i = 0; i < gradient_steps; i++) {
        float gradient_factor = (float)i / (gradient_steps - 1);
        float alpha = 0.1f + (gradient_factor * 0.1f);
        
        // Dark blue gradient colors
        float r = 0.0f;
        float g = 0.08f + (gradient_factor * 0.04f);
        float b = 0.16f + (gradient_factor * 0.08f);
        
        glColor4f(r, g, b, alpha);
        
        float y_start = (float)i * MATRIX_HEIGHT / gradient_steps;
        float y_end = (float)(i + 1) * MATRIX_HEIGHT / gradient_steps;
        
        glBegin(GL_QUADS);
        glVertex2f(0.0f, y_start);
        glVertex2f(MATRIX_WIDTH, y_start);
        glVertex2f(MATRIX_WIDTH, y_end);
        glVertex2f(0.0f, y_end);
        glEnd();
    }
}

// Draw audio spectrum visualization at the bottom
static void draw_audio_spectrum_visualization(gui_context_t *gui) {
    const float spectrum_height = 3.0f;
    const float spectrum_y = MATRIX_HEIGHT - spectrum_height;
    const int spectrum_bars = 32; // Fewer bars for cleaner look
    
    for (int i = 0; i < spectrum_bars; i++) {
        float bar_width = (float)MATRIX_WIDTH / spectrum_bars;
        float x_pos = i * bar_width;
        
        // Get corresponding spectrum data
        float spectrum_value = 0.0f;
        if (i < MAX_FREQUENCY_BINS / 2) {
            spectrum_value = gui->spectrum.spectrum[i] * 2.0f; // Amplify for visibility
        }
        
        // Create blue spectrum bar with gradient
        float intensity = fminf(spectrum_value, 1.0f);
        float bar_height = intensity * spectrum_height;
        
        // Base bar color (darker blue)
        glColor4f(0.0f, 0.1f, 0.3f, 0.6f);
        glBegin(GL_QUADS);
        glVertex2f(x_pos, spectrum_y);
        glVertex2f(x_pos + bar_width * 0.9f, spectrum_y);
        glVertex2f(x_pos + bar_width * 0.9f, spectrum_y + bar_height);
        glVertex2f(x_pos, spectrum_y + bar_height);
        glEnd();
        
        // Highlight bar (brighter blue)
        if (intensity > 0.1f) {
            glColor4f(0.2f, 0.4f, 0.8f, 0.8f);
            glBegin(GL_QUADS);
            glVertex2f(x_pos + bar_width * 0.1f, spectrum_y + bar_height * 0.3f);
            glVertex2f(x_pos + bar_width * 0.8f, spectrum_y + bar_height * 0.3f);
            glVertex2f(x_pos + bar_width * 0.8f, spectrum_y + bar_height);
            glVertex2f(x_pos + bar_width * 0.1f, spectrum_y + bar_height);
            glEnd();
        }
    }
}

// Draw UI overlay elements (status, control indicators)
static void draw_ui_overlay_elements(gui_context_t *gui) {
    // Draw a subtle border around the matrix area
    glColor4f(0.1f, 0.3f, 0.6f, 0.3f); // Medium blue border
    glLineWidth(2.0f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(0.5f, 0.5f);
    glVertex2f(MATRIX_WIDTH - 0.5f, 0.5f);
    glVertex2f(MATRIX_WIDTH - 0.5f, MATRIX_HEIGHT - 0.5f);
    glVertex2f(0.5f, MATRIX_HEIGHT - 0.5f);
    glEnd();
    
    // Add corner accents for visual interest
    draw_corner_accent(2.0f, 2.0f, true);  // Top-left
    draw_corner_accent(MATRIX_WIDTH - 4.0f, 2.0f, false);  // Top-right
    
    // Draw audio activity indicator
    draw_audio_activity_indicator(gui);
}

// Draw corner accent elements
static void draw_corner_accent(float x, float y, bool top_left) {
    glColor4f(0.4f, 0.6f, 1.0f, 0.6f); // Bright blue accent
    
    const float accent_size = 1.5f;
    
    glBegin(GL_QUADS);
    if (top_left) {
        glVertex2f(x, y);
        glVertex2f(x + accent_size, y);
        glVertex2f(x + accent_size, y + accent_size * 0.3f);
        glVertex2f(x, y + accent_size * 0.3f);
    } else {
        glVertex2f(x, y);
        glVertex2f(x + accent_size, y);
        glVertex2f(x + accent_size, y + accent_size * 0.3f);
        glVertex2f(x, y + accent_size * 0.3f);
    }
    glEnd();
}

// Draw audio activity indicator
static void draw_audio_activity_indicator(gui_context_t *gui) {
    // Calculate average audio level
    float avg_level = 0.0f;
    for (int i = 0; i < 16 && i < MAX_FREQUENCY_BINS / 2; i++) {
        avg_level += gui->spectrum.spectrum[i];
    }
    avg_level /= 16.0f;
    
    // Activity indicator position (top-right corner)
    float indicator_x = MATRIX_WIDTH - 3.0f;
    float indicator_y = MATRIX_HEIGHT - 2.0f;
    
    // Base indicator color based on activity level
    float activity_alpha = fminf(avg_level * 3.0f, 1.0f);
    glColor4f(0.0f, 0.4f, 0.9f, activity_alpha);
    
    // Draw activity pulse
    const float pulse_size = 1.0f;
    glBegin(GL_QUADS);
    glVertex2f(indicator_x, indicator_y);
    glVertex2f(indicator_x + pulse_size, indicator_y);
    glVertex2f(indicator_x + pulse_size, indicator_y + pulse_size);
    glVertex2f(indicator_x, indicator_y + pulse_size);
    glEnd();
    
    // Add glow for high activity
    if (activity_alpha > 0.6f) {
        glColor4f(0.3f, 0.6f, 1.0f, activity_alpha * 0.3f);
        glBegin(GL_QUADS);
        glVertex2f(indicator_x - 0.2f, indicator_y - 0.2f);
        glVertex2f(indicator_x + pulse_size + 0.2f, indicator_y - 0.2f);
        glVertex2f(indicator_x + pulse_size + 0.2f, indicator_y + pulse_size + 0.2f);
        glVertex2f(indicator_x - 0.2f, indicator_y + pulse_size + 0.2f);
        glEnd();
    }
}

// GUI creation
bool gui_create(gui_context_t *gui, const clap_plugin_t *plugin, uint32_t width, uint32_t height) {
    memset(gui, 0, sizeof(gui_context_t));
    
    gui->width = width;
    gui->height = height;
    gui->plugin = plugin;
    
    if (pthread_mutex_init(&gui->mutex, NULL) != 0) {
        return false;
    }
    
    // Initialize components
    matrix_init(gui);
    spectrum_init(&gui->spectrum);
    
    gui->running = true;
    
    printf("GUI created: %dx%d\n", width, height);
    return true;
}

// GUI destruction
void gui_destroy(gui_context_t *gui) {
    gui->running = false;
    pthread_mutex_destroy(&gui->mutex);
}

// Update GUI with audio buffer
void gui_update(gui_context_t *gui, const float *audio_buffer, uint32_t frames, double sample_rate) {
    if (!gui->running) return;
    
    // Update matrix animation
    matrix_update(gui, 1.0f / 60.0f); // Assume 60fps
    
    // Store audio data for processing
    if (audio_buffer && frames > 0) {
        static float *audio_copy = NULL;
        static uint32_t audio_copy_size = 0;
        
        if (audio_copy_size < frames) {
            if (audio_copy) free(audio_copy);
            audio_copy = (float *)malloc(frames * sizeof(float));
            audio_copy_size = frames;
        }
        
        if (audio_copy) {
            memcpy(audio_copy, audio_buffer, frames * sizeof(float));
            gui_handle_audio_data(gui, audio_copy, frames);
        }
    }
}

// Render GUI
void gui_render(gui_context_t *gui) {
    matrix_render(gui);
}