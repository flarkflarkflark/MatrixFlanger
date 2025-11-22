#pragma once

#include <clap/clap.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "dsp.h"

// Plugin descriptor
static const clap_plugin_descriptor_t s_plugin_desc = {
    .clap_version = CLAP_VERSION_INIT,
    .id = "com.flark.matrixflanger",
    .name = "flark's MatrixFlanger",
    .vendor = "flark",
    .url = "https://flark.dev/matrixflanger",
    .manual_url = "https://flark.dev/matrixflanger/manual",
    .support_url = "https://flark.dev/matrixflanger/support",
    .logo_url = "https://flark.dev/matrixflanger/logo",
    .artist_url = "https://flark.dev",
    .version = "1.0.0",
    .description = "A professional audio flanger plugin with delay line modulation, LFO control, and feedback processing for creating classic sweeping flanger effects.",
    .features = (const char *[]){
        CLAP_PLUGIN_FEATURE_DYNAMICS,
        CLAP_PLUGIN_FEATURE_STEREO,
        NULL
    },
};

// Parameter IDs
enum {
    PARAM_RATE = 0,
    PARAM_DEPTH = 1,
    PARAM_FEEDBACK = 2,
    PARAM_MIX = 3,
    PARAM_ENABLED = 4
};

// Plugin structure
typedef struct {
    clap_plugin_t plugin;
    const clap_host_t *host;
    
    // Host extensions
    const clap_host_log_t *host_log;
    const clap_host_thread_check_t *host_thread_check;
    const clap_host_params_t *host_params;
    const clap_host_state_t *host_state;
    const clap_host_latency_t *host_latency;
    
    // Plugin state
    flanger_t flanger;
    bool enabled;
    
    // Parameter values
    float rate;
    float depth;
    float feedback;
    float mix;
    
    // Sample rate
    float sample_rate;
    
    // Temporary buffers
    float *temp_buffer;
    uint32_t temp_buffer_size;
} audio_filter_plugin_t;

// Plugin extensions
static const clap_plugin_params_t s_plugin_params;
static const clap_plugin_state_t s_plugin_state;
static const clap_plugin_latency_t s_plugin_latency;
static const clap_plugin_audio_ports_t s_plugin_audio_ports;
static const clap_plugin_ambisonic_t s_plugin_ambisonic;
static const clap_plugin_surround_t s_plugin_surround;
static const clap_plugin_gui_t s_plugin_gui;

// Function declarations
static void destroy(audio_filter_plugin_t *plugin);
static bool init(audio_filter_plugin_t *plugin);
static void deactivate(audio_filter_plugin_t *plugin);
static bool start_processing(const audio_filter_plugin_t *plugin);
static void stop_processing(const audio_filter_plugin_t *plugin);
static void reset(const audio_filter_plugin_t *plugin);
static clap_process_status_t process(audio_filter_plugin_t *plugin, const clap_process_t *process);
static const void *get_extension(const audio_filter_plugin_t *plugin, const char *id);
static void on_main_thread(const audio_filter_plugin_t *plugin);

// Plugin methods
static void audio_filter_destroy(const clap_plugin_t *plugin) {
    audio_filter_plugin_t *p = (audio_filter_plugin_t *)plugin->plugin_data;
    destroy(p);
}

static bool audio_filter_init(const clap_plugin_t *plugin) {
    audio_filter_plugin_t *p = (audio_filter_plugin_t *)plugin->plugin_data;
    return init(p);
}

static void audio_filter_deactivate(const clap_plugin_t *plugin) {
    audio_filter_plugin_t *p = (audio_filter_plugin_t *)plugin->plugin_data;
    deactivate(p);
}

static bool audio_filter_start_processing(const clap_plugin_t *plugin) {
    audio_filter_plugin_t *p = (audio_filter_plugin_t *)plugin->plugin_data;
    return start_processing(p);
}

static void audio_filter_stop_processing(const clap_plugin_t *plugin) {
    audio_filter_plugin_t *p = (audio_filter_plugin_t *)plugin->plugin_data;
    stop_processing(p);
}

static void audio_filter_reset(const clap_plugin_t *plugin) {
    audio_filter_plugin_t *p = (audio_filter_plugin_t *)plugin->plugin_data;
    reset(p);
}

static clap_process_status_t audio_filter_process(const clap_plugin_t *plugin, const clap_process_t *process) {
    audio_filter_plugin_t *p = (audio_filter_plugin_t *)plugin->plugin_data;
    return process(p, process);
}

static const void *audio_filter_get_extension(const clap_plugin_t *plugin, const char *id) {
    audio_filter_plugin_t *p = (audio_filter_plugin_t *)plugin->plugin_data;
    return get_extension(p, id);
}

static void audio_filter_on_main_thread(const clap_plugin_t *plugin) {
    audio_filter_plugin_t *p = (audio_filter_plugin_t *)plugin->plugin_data;
    on_main_thread(p);
}

// Plugin implementation
static void destroy(audio_filter_plugin_t *plugin) {
    if (plugin->temp_buffer) {
        free(plugin->temp_buffer);
        plugin->temp_buffer = NULL;
    }
    free(plugin);
}

static bool init(audio_filter_plugin_t *plugin) {
    // Initialize DSP
    flanger_init(&plugin->flanger, plugin->rate, plugin->depth, plugin->feedback, plugin->mix, plugin->sample_rate);
    
    // Allocate temporary buffer for processing
    plugin->temp_buffer_size = 0;
    plugin->temp_buffer = NULL;
    
    return true;
}

static void deactivate(audio_filter_plugin_t *plugin) {
    // Clean up any allocated resources
    if (plugin->temp_buffer) {
        free(plugin->temp_buffer);
        plugin->temp_buffer = NULL;
        plugin->temp_buffer_size = 0;
    }
}

static bool start_processing(const audio_filter_plugin_t *plugin) {
    return true;
}

static void stop_processing(const audio_filter_plugin_t *plugin) {
    // Nothing to do
}

static void reset(const audio_filter_plugin_t *plugin) {
    filter_reset(&plugin->filter);
}

static void update_flanger_parameters(audio_filter_plugin_t *plugin) {
    flanger_set_rate(&plugin->flanger, plugin->rate);
    flanger_set_depth(&plugin->flanger, plugin->depth);
    flanger_set_feedback(&plugin->flanger, plugin->feedback);
    flanger_set_mix(&plugin->flanger, plugin->mix);
    flanger_set_sample_rate(&plugin->flanger, plugin->sample_rate);
}

static clap_process_status_t process(audio_filter_plugin_t *plugin, const clap_process_t *process) {
    const uint32_t nframes = process->frames_count;
    
    // Handle events
    uint32_t num_events = process->in_events->size(process->in_events);
    for (uint32_t i = 0; i < num_events; ++i) {
        const clap_event_header_t *event = process->in_events->get(process->in_events, i);
        
        if (event->type == CLAP_EVENT_PARAM_VALUE) {
            const clap_event_param_value_t *param_event = (const clap_event_param_value_t *)event;
            switch (param_event->param_id) {
                case PARAM_RATE:
                    plugin->rate = param_event->value;
                    update_flanger_parameters(plugin);
                    break;
                case PARAM_DEPTH:
                    plugin->depth = param_event->value;
                    update_flanger_parameters(plugin);
                    break;
                case PARAM_FEEDBACK:
                    plugin->feedback = param_event->value;
                    update_flanger_parameters(plugin);
                    break;
                case PARAM_MIX:
                    plugin->mix = param_event->value;
                    update_flanger_parameters(plugin);
                    break;
                case PARAM_ENABLED:
                    plugin->enabled = param_event->value > 0.5f;
                    break;
            }
        }
    }
    
    // Process audio
    if (process->audio_inputs_count >= 1 && process->audio_outputs_count >= 1) {
        const clap_audio_buffer_t *input = &process->audio_inputs[0];
        const clap_audio_buffer_t *output = &process->audio_outputs[0];
        
        // Ensure we have enough buffer space
        if (plugin->temp_buffer_size < nframes) {
            if (plugin->temp_buffer) {
                free(plugin->temp_buffer);
            }
            plugin->temp_buffer = (float *)malloc(nframes * sizeof(float));
            plugin->temp_buffer_size = nframes;
        }
        
        if (plugin->enabled && input->data32 && output->data32) {
            // Process each channel
            for (uint32_t ch = 0; ch < output->channel_count; ++ch) {
                if (ch < input->channel_count && input->data32[ch] && output->data32[ch]) {
                    if (plugin->flanger.initialized) {
                        flanger_process_block(&plugin->flanger, input->data32[ch], output->data32[ch], nframes);
                    } else {
                        // Just copy if flanger not initialized
                        memcpy(output->data32[ch], input->data32[ch], nframes * sizeof(float));
                    }
                }
            }
            
            // Copy first channel audio for visualization (if not already copied)
            if (input->data32[0] && plugin->temp_buffer) {
                memcpy(plugin->temp_buffer, input->data32[0], nframes * sizeof(float));
            }
        } else if (!plugin->enabled && input->data32 && output->data32) {
            // Bypass - copy input to output
            for (uint32_t ch = 0; ch < output->channel_count; ++ch) {
                if (ch < input->channel_count && input->data32[ch] && output->data32[ch]) {
                    memcpy(output->data32[ch], input->data32[ch], nframes * sizeof(float));
                }
            }
            
            // Copy first channel audio for visualization (if not already copied)
            if (input->data32[0] && plugin->temp_buffer) {
                memcpy(plugin->temp_buffer, input->data32[0], nframes * sizeof(float));
            }
        }
    }
    
    return CLAP_PROCESS_CONTINUE;
}

static const void *get_extension(const audio_filter_plugin_t *plugin, const char *id) {
    if (!strcmp(id, CLAP_EXT_PARAMS)) {
        return &s_plugin_params;
    }
    if (!strcmp(id, CLAP_EXT_STATE)) {
        return &s_plugin_state;
    }
    if (!strcmp(id, CLAP_EXT_LATENCY)) {
        return &s_plugin_latency;
    }
    if (!strcmp(id, CLAP_EXT_AUDIO_PORTS)) {
        return &s_plugin_audio_ports;
    }
    if (!strcmp(id, CLAP_EXT_SURROUND)) {
        return &s_plugin_surround;
    }
    if (!strcmp(id, CLAP_EXT_AMBISONIC)) {
        return &s_plugin_ambisonic;
    }
    if (!strcmp(id, CLAP_EXT_GUI)) {
        return &s_plugin_gui;
    }
    
    return NULL;
}

static void on_main_thread(const audio_filter_plugin_t *plugin) {
    // Update sample rate if needed
    if (plugin->host && plugin->host->get_sample_rate) {
        double new_sample_rate = plugin->host->get_sample_rate(plugin->host);
        if (new_sample_rate > 0 && new_sample_rate != plugin->sample_rate) {
            plugin->sample_rate = (float)new_sample_rate;
            update_flanger_parameters((audio_filter_plugin_t *)plugin);
        }
    }
}

// Export plugin functions
clap_plugin_t *audio_filter_plugin_create(const clap_host_t *host) {
    audio_filter_plugin_t *plugin = (audio_filter_plugin_t *)calloc(1, sizeof(audio_filter_plugin_t));
    if (!plugin) {
        return NULL;
    }
    
    // Initialize default parameters
    plugin->host = host;
    plugin->rate = 0.5f;             // 0.5 Hz
    plugin->depth = 50.0f;           // 50%
    plugin->feedback = 30.0f;        // 30%
    plugin->mix = 50.0f;             // 50%
    plugin->enabled = true;
    plugin->sample_rate = 44100.0f;  // Default sample rate
    
    // Initialize flanger
    flanger_init(&plugin->flanger, plugin->rate, plugin->depth, plugin->feedback, plugin->mix, plugin->sample_rate);
    
    // Set plugin structure
    plugin->plugin.desc = &s_plugin_desc;
    plugin->plugin.plugin_data = plugin;
    plugin->plugin.init = audio_filter_init;
    plugin->plugin.destroy = audio_filter_destroy;
    plugin->plugin.activate = audio_filter_activate;
    plugin->plugin.deactivate = audio_filter_deactivate;
    plugin->plugin.start_processing = audio_filter_start_processing;
    plugin->plugin.stop_processing = audio_filter_stop_processing;
    plugin->plugin.reset = audio_filter_reset;
    plugin->plugin.process = audio_filter_process;
    plugin->plugin.get_extension = audio_filter_get_extension;
    plugin->plugin.on_main_thread = audio_filter_on_main_thread;
    
    return &plugin->plugin;
}

// Activation function
static bool audio_filter_activate(const clap_plugin_t *plugin, double sample_rate, uint32_t min_frames_count, uint32_t max_frames_count) {
    audio_filter_plugin_t *p = (audio_filter_plugin_t *)plugin->plugin_data;
    p->sample_rate = (float)sample_rate;
    update_flanger_parameters(p);
    return true;
}

// Helper function for parameter bounds
static const clap_param_info_t s_param_info[] = {
    {
        .id = PARAM_RATE,
        .name = "Rate",
        .module = "",
        .min_value = 0.1f,
        .max_value = 10.0f,
        .default_value = 0.5f,
        .flags = CLAP_PARAM_IS_AUTOMATABLE,
        .cookie = NULL,
    },
    {
        .id = PARAM_DEPTH,
        .name = "Depth",
        .module = "",
        .min_value = 0.0f,
        .max_value = 100.0f,
        .default_value = 50.0f,
        .flags = CLAP_PARAM_IS_AUTOMATABLE,
        .cookie = NULL,
    },
    {
        .id = PARAM_FEEDBACK,
        .name = "Feedback",
        .module = "",
        .min_value = 0.0f,
        .max_value = 95.0f,
        .default_value = 30.0f,
        .flags = CLAP_PARAM_IS_AUTOMATABLE,
        .cookie = NULL,
    },
    {
        .id = PARAM_MIX,
        .name = "Mix",
        .module = "",
        .min_value = 0.0f,
        .max_value = 100.0f,
        .default_value = 50.0f,
        .flags = CLAP_PARAM_IS_AUTOMATABLE,
        .cookie = NULL,
    },
    {
        .id = PARAM_ENABLED,
        .name = "Enabled",
        .module = "",
        .min_value = 0.0f,
        .max_value = 1.0f,
        .default_value = 1.0f,
        .flags = CLAP_PARAM_IS_AUTOMATABLE | CLAP_PARAM_IS_STEPPED,
        .cookie = NULL,
    }
};