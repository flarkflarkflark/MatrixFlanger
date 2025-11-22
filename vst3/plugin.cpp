/*
 * VST3 Plugin Entry Point
 * flark's MatrixFilter - VST3 Version
 */

#include "pluginterfaces/base/funknown.h"
#include "pluginterfaces/vst/ivstaudioprocessor.h"
#include "pluginterfaces/vst/ivsteditcontroller.h"
#include "pluginterfaces/vst/vsttypes.h"
#include "public.sdk/source/vst/vstguieditor.h"
#include "public.sdk/source/vst/vstpresetmanager.h"
#include "../src/plugin.cpp"
#include "../src/gui.h"
#include <algorithm>
#include <cmath>

// Plugin UID - Generate unique ID for the plugin
static const char* kVendorName = "flark";
static const char* kPluginName = "flark-MatrixFilter";
static const char* kProductName = "flark's Matrix Filter";
static const char* kVendorVersion = "1.0.0";

// VST3 unique class ID (128-bit)
static const uint128_t kPluginCID = Steinberg::FUID::fromString("FLARK-MATRIX-FILTER-VST3-001");

// Parameter IDs
enum ParameterID {
    kCutoff = 0,
    kResonance = 1,
    kGain = 2,
    kFilterType = 3,
    kEnabled = 4
};

// Processor class - handles audio processing
class MatrixFilterProcessor : public Steinberg::Vst::AudioProcessor {
public:
    MatrixFilterProcessor() {
        setInitialDelay(0);
        
        // Add audio input
        Steinberg::Vst::AudioInputPort audioIn;
        audioIn.info.mediaType = Steinberg::Vst::MediaTypes::kAudio;
        audioIn.info.direction = Steinberg::Vst::kInput;
        audioIn.info.busCount = 1;
        audioIn.info.channelCount.max = 2; // Stereo
        audioIn.info.channelCount.min = 1;
        audioIn.info.flags = 0;
        strcpy(audioIn.info.name, "Audio Input");
        addAudioInput(audioIn);
        
        // Add audio output
        Steinberg::Vst::AudioOutputPort audioOut;
        audioOut.info.mediaType = Steinberg::Vst::MediaTypes::kAudio;
        audioOut.info.direction = Steinberg::Vst::kOutput;
        audioOut.info.busCount = 1;
        audioOut.info.channelCount.max = 2; // Stereo
        audioOut.info.channelCount.min = 1;
        audioOut.info.flags = 0;
        strcpy(audioOut.info.name, "Audio Output");
        addAudioOutput(audioOut);
        
        // Initialize parameters
        addParameter(kCutoff, "Cutoff Frequency", "Hz", 0, 20000, 1000);
        addParameter(kResonance, "Resonance", "", 0.1, 10.0, 1.0);
        addParameter(kGain, "Gain", "dB", -60, 60, 0);
        addParameter(kFilterType, "Filter Type", "", 0, 6, 0);
        addParameter(kEnabled, "Enabled", "", 0, 1, 1);
        
        // Initialize DSP
        filter_init(&filter, FILTER_TYPE_LOWPASS, 1000.0f, 1.0f, 0.0f, 44100.0f);
        
        // Initialize audio buffer
        audio_buffer = nullptr;
        buffer_size = 0;
    }
    
    ~MatrixFilterProcessor() override {
        if (audio_buffer) {
            free(audio_buffer);
            audio_buffer = nullptr;
        }
    }
    
    Steinberg::tresult PLUGIN_API initialize(Steinberg::FUnknown* context) override {
        return AudioProcessor::initialize(context);
    }
    
    Steinberg::tresult PLUGIN_API terminate() override {
        return AudioProcessor::terminate();
    }
    
    Steinberg::tresult PLUGIN_API setProcessing(Steinberg::TBool state) override {
        if (state) {
            filter_reset(&filter);
        }
        return AudioProcessor::setProcessing(state);
    }
    
    Steinberg::tresult PLUGIN_API process(Steinberg::Vst::ProcessData& data) override {
        if (!data.input || !data.output) {
            return Steinberg::kInvalidArgument;
        }
        
        uint32_t sampleFrames = data.numSamples;
        float sampleRate = data.processContext->sampleRate;
        
        // Update sample rate in DSP
        if (sampleRate > 0 && sampleRate != current_sample_rate) {
            current_sample_rate = sampleRate;
            filter_set_sample_rate(&filter, sampleRate);
        }
        
        // Get parameters
        float cutoff = 1000.0f;
        float resonance = 1.0f;
        float gain = 0.0f;
        filter_type_t filter_type = FILTER_TYPE_LOWPASS;
        bool enabled = true;
        
        // Extract parameter values
        if (data.inputParameterChanges) {
            Steinberg::Vst::ParamValue newValue;
            Steinberg::int32 numParams = data.inputParameterChanges->getParameterCount();
            
            for (Steinberg::int32 i = 0; i < numParams; i++) {
                Steinberg::Vst::IParamChangeQueue* queue = data.inputParameterChanges->getParameterData(i);
                if (queue) {
                    Steinberg::int32 offsetSample;
                    Steinberg::int32 queueValue;
                    Steinberg::int32 numValues = queue->getPointCount();
                    
                    if (queue->getPoint(numValues - 1, offsetSample, newValue) == Steinberg::kResultOk) {
                        Steinberg::Vst::ParamID paramID = queue->getParameterID();
                        
                        switch (paramID) {
                            case kCutoff:
                                cutoff = newValue;
                                break;
                            case kResonance:
                                resonance = newValue;
                                break;
                            case kGain:
                                gain = newValue;
                                break;
                            case kFilterType:
                                filter_type = (filter_type_t)(int)newValue;
                                break;
                            case kEnabled:
                                enabled = newValue >= 0.5;
                                break;
                        }
                    }
                }
            }
            
            // Update filter parameters
            filter_set_parameters(&filter, filter_type, cutoff, resonance, gain);
        }
        
        // Ensure audio buffer
        if (!audio_buffer || buffer_size < sampleFrames) {
            if (audio_buffer) free(audio_buffer);
            audio_buffer = (float*)malloc(sampleFrames * sizeof(float));
            buffer_size = sampleFrames;
        }
        
        // Process audio
        Steinberg::Vst::AudioBusBuffers* inBus = data.input;
        Steinberg::Vst::AudioBusBuffers* outBus = data.output;
        
        if (inBus && outBus && inBus->channelBuffers32 && outBus->channelBuffers32) {
            uint32_t numChannels = std::min(inBus->numChannels, outBus->numChannels);
            
            for (uint32_t ch = 0; ch < numChannels; ch++) {
                float* input = inBus->channelBuffers32[ch];
                float* output = outBus->channelBuffers32[ch];
                
                if (input && output) {
                    if (enabled && filter.initialized) {
                        // Copy first channel for visualization
                        if (ch == 0 && audio_buffer) {
                            memcpy(audio_buffer, input, sampleFrames * sizeof(float));
                        }
                        
                        // Apply filter
                        filter_process_block(&filter, input, output, sampleFrames);
                    } else {
                        // Bypass - copy input to output
                        memcpy(output, input, sampleFrames * sizeof(float));
                        
                        // Copy first channel for visualization
                        if (ch == 0 && audio_buffer) {
                            memcpy(audio_buffer, input, sampleFrames * sizeof(float));
                        }
                    }
                }
            }
        }
        
        return Steinberg::kResultOk;
    }
    
    Steinberg::tresult PLUGIN_API getRoutingInfo(Steinberg::Vst::RoutingInfo& inInfo, Steinberg::Vst::RoutingInfo& outInfo) override {
        outInfo.mediaType = inInfo.mediaType;
        outInfo.busIndex = inInfo.busIndex;
        outInfo.channel = inInfo.channel;
        return Steinberg::kResultOk;
    }
    
    Steinberg::tresult PLUGIN_API canProcessSampleSize(Steinberg::int32 symbolicSampleSize) override {
        return Steinberg::kResultOk; // Support both 32-bit and 64-bit
    }
    
    Steinberg::tresult PLUGIN_API setState(Steinberg::IBStream* state) override {
        Steinberg::int32 stateSize;
        Steinberg::tresult result = Steinberg::kResultOk;
        
        result = state->seek(0, Steinberg::IBStream::kIBStreamEndPosition);
        result = state->tell(stateSize);
        result = state->seek(0, Steinberg::IBStream::kIBStreamStartPosition);
        
        if (stateSize >= sizeof(float) * 5) {
            float param;
            state->read(&param, sizeof(float)); cutoff = param;
            state->read(&param, sizeof(float)); resonance = param;
            state->read(&param, sizeof(float)); gain = param;
            int32_t value;
            state->read(&value, sizeof(int32_t)); filter_type = (filter_type_t)value;
            bool enabled;
            state->read(&enabled, sizeof(bool)); enabled = enabled;
            
            filter_set_parameters(&filter, filter_type, cutoff, resonance, gain);
        }
        
        return Steinberg::kResultOk;
    }
    
    Steinberg::tresult PLUGIN_API getState(Steinberg::IBStream* state) override {
        state->write(&cutoff, sizeof(float));
        state->write(&resonance, sizeof(float));
        state->write(&gain, sizeof(float));
        int32_t filterType = (int32_t)filter_type;
        state->write(&filterType, sizeof(int32_t));
        state->write(&enabled, sizeof(bool));
        return Steinberg::kResultOk;
    }
    
    Steinberg::tresult PLUGIN_API setupProcessing(Steinberg::Vst::ProcessSetup& setup) override {
        current_sample_rate = setup.sampleRate;
        return Steinberg::kResultOk;
    }
    
    Steinberg::tresult PLUGIN_API getProcessorInfo(Steinberg::Vst::ProcessorInfo& info) override {
        info.precision = Steinberg::Vst::ProcessPrecision::k32;
        info.advancedFlags.disableDoublePrecision = Steinberg::kTrue;
        return Steinberg::kResultOk;
    }
    
    // Helper methods
    void addParameter(Steinberg::Vst::ParamID id, const char* name, const char* units, 
                     float min, float max, float defaultValue) {
        Steinberg::Vst::ParameterInfo paramInfo;
        Steinberg::FUnknownConstructor(Steinberg::Vst::ParameterInfo);
        
        paramInfo.id = id;
        paramInfo.flags = Steinberg::Vst::ParameterFlags::kCanAutomate | Steinberg::Vst::ParameterFlags::kIsBypass;
        Steinberg::FUnknownAssign(paramInfo.title, name);
        Steinberg::FUnknownAssign(paramInfo.units, units);
        paramInfo.minValue = min;
        paramInfo.maxValue = max;
        paramInfo.defaultValue = defaultValue;
        paramInfo.stepCount = (max - min > 100) ? 0 : (int32_t)(max - min);
        
        addParameter(paramInfo);
    }
    
    // Accessors for GUI
    float* getAudioBuffer() const { return audio_buffer; }
    uint32_t getBufferSize() const { return buffer_size; }
    
protected:
    filter_t filter;
    float current_sample_rate;
    
    // Audio buffer for visualization
    float* audio_buffer;
    uint32_t buffer_size;
};

// Factory class for creating plugin instances
class MatrixFilterFactory : public Steinberg::FUnknown {
public:
    Steinberg::tresult PLUGIN_API createInstance(Steinberg::FUID classID, Steinberg::FUID interfaceID, void** obj) override {
        if (classID == Steinberg::Vst::IComponentUIDs::kProcessorClass) {
            *obj = new MatrixFilterProcessor();
            return Steinberg::kResultOk;
        }
        return Steinberg::kNoInterface;
    }
};

// Plugin entry point
Steinberg::FUnknown* createInstance(Steinberg::FUID classID, Steinberg::FUID interfaceID) {
    static MatrixFilterFactory factory;
    if (classID == kPluginCID || interfaceID == Steinberg::Vst::IComponentUIDs::kProcessorClass) {
        return &factory;
    }
    return nullptr;
}

// Plugin factory
BEGIN_FACTORY_DEF(
    kVendorName,
    kProductName,
    kVendorVersion,
    Steinberg::Vst::kCpuFamily | Steinberg::Vst::kArmNeonFamily
)

CLASS_FACTORY_END