/*
 * LV2 UI Implementation with Matrix Effects
 * flark's MatrixFilter - LV2 Version
 */

#include <lv2/lv2plug.in/ns/extensions/ui/ui.h>
#include <lv2/lv2plug.in/ns/extensions/options/options.h>
#include <lv2/lv2plug.in/ns/ext/uri-map/uri-map.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

// OpenGL headers (platform-specific)
#ifdef __APPLE__
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#else
#include <GL/gl.h>
#include <GL/glu.h>
#endif

// Include GUI header
#include "../src/gui.h"

// LV2 UI URI
#define LV2_MATRIXFILTER_UI_URI "http://flark.dev/matrixfilter_ui"

// UI instance structure
typedef struct {
    LV2UI_Write_Function write_function;
    LV2UI_Controller controller;
    
    // OpenGL context
    void* gl_context;
    
    // UI features
    const LV2_URID_Map* urid_map;
    const LV2_Feature* const* features;
    
    // Matrix effect instance
    MatrixEffect matrix_effect;
    
    // Window dimensions
    int width;
    int height;
    
    // Animation timer
    double last_time;
    bool running;
} MatrixFilterUI;

// Forward declarations
static void cleanup(LV2UI_Handle instance);
static void port_event(LV2UI_Handle instance, uint32_t port, uint32_t size, uint32_t format, const void* buffer);
static int select_func(LV2UI_Feature_Handle handle, const char* feature);
static int subscribe(LV2UI_Handle handle, uint32_t port_index, uint32_t format, const LV2_Feature* const* features);
static int unsubscribe(LV2UI_Handle handle, uint32_t port_index, uint32_t format, const LV2_Feature* const* features);
static void free_instance(LV2UI_Handle instance);

// UI descriptor
static const LV2UI_Descriptor ui_descriptor = {
    LV2_MATRIXFILTER_UI_URI,
    instantiate,
    cleanup,
    port_event,
    &extension_data
};

// Plugin UI extension data
static const void* extension_data(const char* uri) {
    static const LV2UI_Descriptor extension = {
        NULL,  // instantiate
        NULL,  // cleanup
        NULL,  // port_event
        select_func,
        subscribe,
        unsubscribe
    };
    
    if (!strcmp(uri, LV2_UI__portSubscribe)) {
        return &extension;
    }
    return NULL;
}

// UI instantiation
static LV2UI_Handle instantiate(const LV2UI_Descriptor* descriptor, const char* plugin_uri,
                                const char* bundle_path, const char* ui_uri,
                                LV2UI_Write_Function write_function, LV2UI_Controller controller,
                                LV2UI_Widget* widget, const LV2_Feature* const* features) {
    
    MatrixFilterUI* ui = (MatrixFilterUI*)calloc(1, sizeof(MatrixFilterUI));
    if (!ui) return NULL;
    
    ui->write_function = write_function;
    ui->controller = controller;
    ui->width = 800;
    ui->height = 600;
    ui->running = true;
    ui->last_time = 0.0;
    
    // Extract features
    for (int i = 0; features[i]; i++) {
        if (!strcmp(features[i]->URI, LV2_URID__map)) {
            ui->urid_map = (const LV2_URID_Map*)features[i]->data;
        } else if (!strcmp(features[i]->URI, LV2_UI__parent)) {
            ui->gl_context = features[i]->data;
        }
    }
    
    // Initialize matrix effect
    MatrixEffect_Init(&ui->matrix_effect, ui->width, ui->height);
    
    // Create widget (platform-specific)
    ui->gl_context = create_opengl_widget(ui->width, ui->height);
    *widget = ui->gl_context;
    
    if (!ui->gl_context) {
        free(ui);
        return NULL;
    }
    
    return ui;
}

// Cleanup UI
static void cleanup(LV2UI_Handle instance) {
    MatrixFilterUI* ui = (MatrixFilterUI*)instance;
    if (ui) {
        if (ui->gl_context) {
            destroy_opengl_widget(ui->gl_context);
        }
        MatrixEffect_Destroy(&ui->matrix_effect);
        free(ui);
    }
}

// Port event handler
static void port_event(LV2UI_Handle instance, uint32_t port, uint32_t size, uint32_t format, const void* buffer) {
    MatrixFilterUI* ui = (MatrixFilterUI*)instance;
    if (!ui) return;
    
    // Handle port events for parameter changes
    switch (port) {
        case 4: // Cutoff frequency
            if (size == sizeof(float) && format == 0) {
                float cutoff = *(const float*)buffer;
                MatrixEffect_UpdateParameter(&ui->matrix_effect, PARAM_CUTOFF, cutoff);
            }
            break;
        case 5: // Resonance
            if (size == sizeof(float) && format == 0) {
                float resonance = *(const float*)buffer;
                MatrixEffect_UpdateParameter(&ui->matrix_effect, PARAM_RESONANCE, resonance);
            }
            break;
        case 6: // Gain
            if (size == sizeof(float) && format == 0) {
                float gain = *(const float*)buffer;
                MatrixEffect_UpdateParameter(&ui->matrix_effect, PARAM_GAIN, gain);
            }
            break;
        case 7: // Filter type
            if (size == sizeof(float) && format == 0) {
                float filter_type = *(const float*)buffer;
                MatrixEffect_UpdateParameter(&ui->matrix_effect, PARAM_FILTER_TYPE, filter_type);
            }
            break;
        case 8: // Enabled
            if (size == sizeof(float) && format == 0) {
                float enabled = *(const float*)buffer;
                MatrixEffect_UpdateParameter(&ui->matrix_effect, PARAM_ENABLED, enabled > 0.5f);
            }
            break;
    }
}

// Feature selection callback
static int select_func(LV2UI_Feature_Handle handle, const char* feature) {
    MatrixFilterUI* ui = (MatrixFilterUI*)handle;
    if (!ui || !feature) return 0;
    
    // Handle UI-specific features
    if (!strcmp(feature, LV2_UI__idleInterface)) {
        // Handle idle interface for animation
        if (ui->running && ui->gl_context) {
            // Update animation
            double current_time = get_time();
            double delta_time = current_time - ui->last_time;
            
            // Update matrix effect
            MatrixEffect_Update(&ui->matrix_effect, delta_time);
            
            // Render
            render_matrix_effect(ui->gl_context, &ui->matrix_effect);
            
            ui->last_time = current_time;
        }
        return 1;
    }
    
    return 0;
}

// Subscribe to port updates
static int subscribe(LV2UI_Handle handle, uint32_t port_index, uint32_t format, const LV2_Feature* const* features) {
    MatrixFilterUI* ui = (MatrixFilterUI*)handle;
    if (!ui) return -1;
    
    // Subscribe to parameter ports for real-time updates
    if (port_index >= 4 && port_index <= 8 && format == 0) {
        // Parameters we care about
        return 0; // Success
    }
    
    return -1; // Not interested
}

// Unsubscribe from port updates
static int unsubscribe(LV2UI_Handle handle, uint32_t port_index, uint32_t format, const LV2_Feature* const* features) {
    return 0; // Always succeed
}

// Helper functions for platform-specific OpenGL widget creation
static void* create_opengl_widget(int width, int height) {
    // This would be implemented differently for each platform
    // Linux: X11 + GLX
    // Windows: Win32 + WGL
    // macOS: Cocoa + NSOpenGL
    
    // For demonstration, return a dummy widget pointer
    // In a real implementation, this would create the actual GL context
    void* widget = malloc(sizeof(unsigned int)); // Dummy widget
    return widget;
}

static void destroy_opengl_widget(void* widget) {
    if (widget) {
        free(widget);
    }
}

static double get_time() {
    // Return current time in seconds
    static double start_time = 0.0;
    if (start_time == 0.0) {
        start_time = get_current_time_seconds();
    }
    return get_current_time_seconds() - start_time;
}

// UI entry points
LV2_SYMBOL_EXPORT
const LV2UI_Descriptor* lv2ui_descriptor(uint32_t index) {
    switch (index) {
        case 0: return &ui_descriptor;
        default: return NULL;
    }
}

// Plugin UI factory
LV2_SYMBOL_EXPORT
const LV2UI_Descriptor** lv2ui_plugin_uides(void) {
    static const LV2UI_Descriptor* descriptors[] = { &ui_descriptor, NULL };
    return descriptors;
}