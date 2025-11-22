/*
 * LV2 Plugin Implementation
 * flark's MatrixFilter - LV2 Version
 */

#include <lv2/lv2.h>
#include <lv2/lv2plug.in/ns/extensions/ui/ui.h>
#include <lv2/lv2plug.in/ns/extensions/options/options.h>
#include <lv2/lv2plug.in/ns/extensions/state/state.h>
#include <lv2/lv2plug.in/ns/extensions/parameters/parameters.h>
#include <lv2/lv2plug.in/ns/extensions/presets/presets.h>
#include <lv2/lv2plug.in/ns/ext/atom/atom.h>
#include <lv2/lv2plug.in/ns/ext/uri-map/uri-map.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// Include DSP header
#include "../src/dsp.h"

// LV2 plugin URI
#define LV2_MATRIXFILTER_URI "http://flark.dev/matrixfilter"

// Plugin state structure
typedef struct {
    float cutoff_freq;
    float resonance;
    float gain;
    filter_type_t filter_type;
    bool enabled;
    float sample_rate;
} MatrixFilterPlugin;

// LV2 plugin instance
typedef struct {
    // LV2 handle
    LV2_Handle handle;
    
    // Plugin features
    const LV2_URID_Map* urid_map;
    const LV2_Options_Interface* options_iface;
    const LV2_State_Interface* state_iface;
    const LV2_Parameters_Interface* params_iface;
    const LV2_Presets_Interface* presets_iface;
    
    // URIDs
    LV2_URID atom_Float;
    LV2_URID atom_Resource;
    LV2_URID atom_Path;
    LV2_URID state_Changed;
    LV2_URID param_sampleRate;
    LV2_URID matrixFilter_Config;
    
    // Plugin state
    MatrixFilterPlugin plugin;
    
    // Audio buffers
    float* input_buffer;
    float* output_buffer;
    uint32_t buffer_size;
    
    // DSP
    filter_t filter;
} MatrixFilterInstance;

// Plugin ports
enum {
    LV2_MATRIXFILTER_AUDIO_IN_L = 0,
    LV2_MATRIXFILTER_AUDIO_IN_R,
    LV2_MATRIXFILTER_AUDIO_OUT_L,
    LV2_MATRIXFILTER_AUDIO_OUT_R,
    LV2_MATRIXFILTER_CUTOFF,
    LV2_MATRIXFILTER_RESONANCE,
    LV2_MATRIXFILTER_GAIN,
    LV2_MATRIXFILTER_FILTER_TYPE,
    LV2_MATRIXFILTER_ENABLED,
    LV2_MATRIXFILTER_PORT_COUNT
};

// Plugin extension data structure
typedef struct {
    const LV2_Descriptor* descriptor;
    const LV2_Feature* const* features;
} MatrixFilterExtension;

// Forward declarations
static void* instantiate(const LV2_Descriptor* descriptor, double sample_rate, const char* bundle_path, const LV2_Feature* const* features);
static void cleanup(LV2_Handle instance);
static void connect_port(LV2_Handle instance, uint32_t port, void* data_location);
static void run(LV2_Handle instance, uint32_t sample_count);
static const LV2_Descriptor* get_descriptor(uint32_t index);
static void free_instance(LV2_Handle instance);

// Plugin descriptor
static const LV2_Descriptor plugin_descriptor = {
    LV2_MATRIXFILTER_URI,
    instantiate,
    connect_port,
    run,
    free_instance,
    NULL, // extension_data
};

// URI map callback
static uint32_t uri_map_uri_to_id(LV2_URI_Map_Callback_Data callback_data, const char* uri) {
    if (!strcmp(uri, LV2_ATOM__Float)) return 1;
    if (!strcmp(uri, LV2_ATOM__Resource)) return 2;
    if (!strcmp(uri, LV2_ATOM__Path)) return 3;
    if (!strcmp(uri, LV2_STATE__Changed)) return 4;
    if (!strcmp(uri, LV2_PARAMETERS__sampleRate)) return 5;
    if (!strcmp(uri, "http://flark.dev/matrixfilter/config")) return 6;
    return 0;
}

// Plugin initialization
static void* instantiate(const LV2_Descriptor* descriptor, double sample_rate, const char* bundle_path, const LV2_Feature* const* features) {
    MatrixFilterInstance* instance = (MatrixFilterInstance*)calloc(1, sizeof(MatrixFilterInstance));
    if (!instance) return NULL;
    
    instance->handle = instance;
    
    // Get required features
    for (int i = 0; features[i]; i++) {
        if (!strcmp(features[i]->URI, LV2_URID__map)) {
            instance->urid_map = (const LV2_URID_Map*)features[i]->data;
        } else if (!strcmp(features[i]->URI, LV2_OPTIONS__interface)) {
            instance->options_iface = (const LV2_Options_Interface*)features[i]->data;
        } else if (!strcmp(features[i]->URI, LV2_STATE__interface)) {
            instance->state_iface = (const LV2_State_Interface*)features[i]->data;
        } else if (!strcmp(features[i]->URI, LV2_PARAMETERS__interface)) {
            instance->params_iface = (const LV2_Parameters_Interface*)features[i]->data;
        } else if (!strcmp(features[i]->URI, LV2_PRESETS__interface)) {
            instance->presets_iface = (const LV2_Presets_Interface*)features[i]->data;
        }
    }
    
    // Setup URIDs
    if (instance->urid_map) {
        instance->atom_Float = instance->urid_map->map(instance->urid_map->callback_data, LV2_ATOM__Float);
        instance->atom_Resource = instance->urid_map->map(instance->urid_map->callback_data, LV2_ATOM__Resource);
        instance->atom_Path = instance->urid_map->map(instance->urid_map->callback_data, LV2_ATOM__Path);
        instance->state_Changed = instance->urid_map->map(instance->urid_map->callback_data, LV2_STATE__Changed);
        instance->param_sampleRate = instance->urid_map->map(instance->urid_map->callback_data, LV2_PARAMETERS__sampleRate);
        instance->matrixFilter_Config = instance->urid_map->map(instance->urid_map->callback_data, "http://flark.dev/matrixfilter/config");
    }
    
    // Initialize plugin parameters
    instance->plugin.cutoff_freq = 1000.0f;
    instance->plugin.resonance = 1.0f;
    instance->plugin.gain = 0.0f;
    instance->plugin.filter_type = FILTER_TYPE_LOWPASS;
    instance->plugin.enabled = true;
    instance->plugin.sample_rate = (float)sample_rate;
    
    // Initialize DSP
    filter_init(&instance->filter, instance->plugin.filter_type, 
                instance->plugin.cutoff_freq, instance->plugin.resonance, 
                instance->plugin.gain, instance->plugin.sample_rate);
    
    return instance;
}

// Cleanup plugin instance
static void cleanup(LV2_Handle instance) {
    MatrixFilterInstance* plugin = (MatrixFilterInstance*)instance;
    if (plugin) {
        if (plugin->input_buffer) free(plugin->input_buffer);
        if (plugin->output_buffer) free(plugin->output_buffer);
        free(plugin);
    }
}

// Free plugin instance (called by framework)
static void free_instance(LV2_Handle instance) {
    cleanup(instance);
}

// Connect ports
static void connect_port(LV2_Handle instance, uint32_t port, void* data_location) {
    MatrixFilterInstance* plugin = (MatrixFilterInstance*)instance;
    if (!plugin) return;
    
    switch (port) {
        case LV2_MATRIXFILTER_AUDIO_IN_L:
        case LV2_MATRIXFILTER_AUDIO_IN_R:
        case LV2_MATRIXFILTER_AUDIO_OUT_L:
        case LV2_MATRIXFILTER_AUDIO_OUT_R:
            // Audio ports are connected to input/output buffers
            break;
        case LV2_MATRIXFILTER_CUTOFF:
            plugin->plugin.cutoff_freq = *(float*)data_location;
            break;
        case LV2_MATRIXFILTER_RESONANCE:
            plugin->plugin.resonance = *(float*)data_location;
            break;
        case LV2_MATRIXFILTER_GAIN:
            plugin->plugin.gain = *(float*)data_location;
            break;
        case LV2_MATRIXFILTER_FILTER_TYPE:
            plugin->plugin.filter_type = (filter_type_t)*(float*)data_location;
            break;
        case LV2_MATRIXFILTER_ENABLED:
            plugin->plugin.enabled = *(float*)data_location >= 0.5f;
            break;
    }
}

// Plugin processing
static void run(LV2_Handle instance, uint32_t sample_count) {
    MatrixFilterInstance* plugin = (MatrixFilterInstance*)instance;
    if (!plugin) return;
    
    // Update filter parameters
    filter_set_parameters(&plugin->filter, plugin->plugin.filter_type,
                         plugin->plugin.cutoff_freq, plugin->plugin.resonance,
                         plugin->plugin.gain);
    filter_set_sample_rate(&plugin->filter, plugin->plugin.sample_rate);
    
    // Get audio ports (simplified for stereo)
    float* input_l = (float*)plugin->input_buffer;  // Would be connected via connect_port
    float* input_r = (float*)plugin->input_buffer;
    float* output_l = (float*)plugin->output_buffer;  // Would be connected via connect_port
    float* output_r = (float*)plugin->output_buffer;
    
    // Process audio if inputs/outputs are connected and filter is enabled
    if (plugin->plugin.enabled && plugin->filter.initialized) {
        // In a real implementation, audio ports would be properly connected
        // For now, this is a simplified version
        if (input_l && output_l) {
            // Process left channel
            filter_process_block(&plugin->filter, input_l, output_l, sample_count);
        }
        if (input_r && output_r) {
            // Process right channel
            filter_process_block(&plugin->filter, input_r, output_r, sample_count);
        }
    }
}

// Plugin descriptor accessor
static const LV2_Descriptor* get_descriptor(uint32_t index) {
    switch (index) {
        case 0: return &plugin_descriptor;
        default: return NULL;
    }
}

// Plugin entry point
LV2_SYMBOL_EXPORT
const LV2_Descriptor* lv2_descriptor(uint32_t index) {
    return get_descriptor(index);
}

// Plugin factory entry point
LV2_SYMBOL_EXPORT
const LV2_Descriptor** lv2_plugin_descriptors(void) {
    static const LV2_Descriptor* descriptors[] = { &plugin_descriptor, NULL };
    return descriptors;
}