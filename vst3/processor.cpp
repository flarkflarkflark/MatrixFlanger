/*
 * VST3 Audio Processor
 * flark's MatrixFilter - VST3 Version
 */

#include "public.sdk/source/vst/vstaudioprocessor.h"
#include "public.sdk/source/vst/vstguieditor.h"
#include "pluginterfaces/vst/vsttypes.h"
#include "pluginterfaces/vst/ivstparameterchanges.h"
#include "pluginterfaces/vst/ivstaudioprocessor.h"
#include "../src/dsp.h"
#include "../src/gui.h"

using namespace Steinberg;
using namespace Steinberg::Vst;

class MatrixFilterProcessor : public AudioProcessor, public IAudioProcessor {
public:
    MatrixFilterProcessor() : AudioProcessor() {
        // Add audio input
        AudioBusInfo inputInfo;
        inputInfo.mediaType = MediaTypes::kAudio;
        inputInfo.direction = kInput;
        inputInfo.busCount = 1;
        inputInfo.channelCount = {1, 2}; // Mono to Stereo
        inputInfo.flags = 0;
        addAudioInput(String("Audio Input"), inputInfo);

        // Add audio output
        AudioBusInfo outputInfo;
        outputInfo.mediaType = MediaTypes::kAudio;
        outputInfo.direction = kOutput;
        outputInfo.busCount = 1;
        outputInfo.channelCount = {1, 2}; // Mono to Stereo
        outputInfo.flags = 0;
        addAudioOutput(String("Audio Output"), outputInfo);

        // Add parameters
        addParameter(new Parameter(String("Cutoff Frequency"), String("Hz"), 0, 20000, 1000, ParameterFlags::kCanAutomate));
        addParameter(new Parameter(String("Resonance"), String(""), 0.1, 10.0, 1.0, ParameterFlags::kCanAutomate));
        addParameter(new Parameter(String("Gain"), String("dB"), -60, 60, 0, ParameterFlags::kCanAutomate));
        addParameter(new Parameter(String("Filter Type"), String(""), 0, 6, 0, ParameterFlags::kCanAutomate | ParameterFlags::kIsBypass));
        addParameter(new Parameter(String("Enabled"), String(""), 0, 1, 1, ParameterFlags::kCanAutomate | ParameterFlags::kIsBypass));

        // Initialize DSP
        filter_init(&filter, FILTER_TYPE_LOWPASS, 1000.0f, 1.0f, 0.0f, 44100.0f);
        current_sample_rate = 44100.0f;
        
        // Initialize parameters
        cutoff_freq = 1000.0f;
        resonance = 1.0f;
        gain = 0.0f;
        filter_type = FILTER_TYPE_LOWPASS;
        enabled = true;
    }

    tresult PLUGIN_API initialize(FUnknown* context) override {
        tresult result = AudioProcessor::initialize(context);
        if (result == kResultOk) {
            return kResultOk;
        }
        return result;
    }

    tresult PLUGIN_API setActive(TBool state) override {
        if (state) {
            filter_reset(&filter);
        }
        return AudioProcessor::setActive(state);
    }

    tresult PLUGIN_API process(ProcessData& data) override {
        if (!data.inputParameterChanges) {
            return kResultOk;
        }

        uint32_t nframes = data.numSamples;
        float sampleRate = getSampleRate();
        
        // Extract parameter changes
        int32_t numParams = data.inputParameterChanges->getParameterCount();
        for (int32_t i = 0; i < numParams; i++) {
            IParamChangeQueue* queue = data.inputParameterChanges->getParameterData(i);
            if (queue) {
                int32_t offsetSample;
                ParamValue newValue;
                int32_t numValues = queue->getPointCount();
                
                if (numValues > 0 && queue->getPoint(numValues - 1, offsetSample, newValue) == kResultOk) {
                    ParamID paramID = queue->getParameterID();
                    
                    switch (paramID) {
                        case 0: // Cutoff
                            cutoff_freq = (float)newValue;
                            break;
                        case 1: // Resonance
                            resonance = (float)newValue;
                            break;
                        case 2: // Gain
                            gain = (float)newValue;
                            break;
                        case 3: // Filter Type
                            filter_type = (filter_type_t)(int)newValue;
                            break;
                        case 4: // Enabled
                            enabled = newValue >= 0.5;
                            break;
                    }
                }
            }
        }

        // Update filter parameters
        filter_set_parameters(&filter, filter_type, cutoff_freq, resonance, gain);
        filter_set_sample_rate(&filter, sampleRate);

        // Process audio if input and output are valid
        if (data.inputs[0].channelBuffers32 && data.outputs[0].channelBuffers32) {
            uint32_t numChannels = std::min(data.inputs[0].numChannels, data.outputs[0].numChannels);
            
            for (uint32_t ch = 0; ch < numChannels; ch++) {
                float* input = data.inputs[0].channelBuffers32[ch];
                float* output = data.outputs[0].channelBuffers32[ch];
                
                if (input && output) {
                    if (enabled && filter.initialized) {
                        filter_process_block(&filter, input, output, nframes);
                    } else {
                        // Bypass
                        memcpy(output, input, nframes * sizeof(float));
                    }
                }
            }
        }

        return kResultOk;
    }

    tresult PLUGIN_API setParamNormalized(ParamID id, ParamValue valueNormalized) override {
        switch (id) {
            case 0: cutoff_freq = (float)valueNormalized; break;
            case 1: resonance = (float)valueNormalized; break;
            case 2: gain = (float)valueNormalized; break;
            case 3: filter_type = (filter_type_t)(int)valueNormalized; break;
            case 4: enabled = valueNormalized >= 0.5; break;
        }
        return AudioProcessor::setParamNormalized(id, valueNormalized);
    }

    ParamValue PLUGIN_API getParamNormalized(ParamID id) const override {
        switch (id) {
            case 0: return cutoff_freq;
            case 1: return resonance;
            case 2: return gain;
            case 3: return (float)filter_type;
            case 4: return enabled ? 1.0 : 0.0;
        }
        return 0.0;
    }

    tresult PLUGIN_API getParamStringByValue(ParamID id, ParamValue valueNormalized, String128 string) const override {
        switch (id) {
            case 0: // Cutoff
                sprintf16(string, u"%5.1f Hz", valueNormalized);
                break;
            case 1: // Resonance
                sprintf16(string, u"%.2f", valueNormalized);
                break;
            case 2: // Gain
                sprintf16(string, u"%+.1f dB", valueNormalized);
                break;
            case 3: // Filter Type
                {
                    const char* filterNames[] = {"Low-Pass", "High-Pass", "Band-Pass", "Notch", "Peaking", "Low Shelf", "High Shelf"};
                    int index = (int)valueNormalized;
                    if (index >= 0 && index < 7) {
                        strcpy16(string, u8"filterNames[index]");
                    }
                }
                break;
            case 4: // Enabled
                strcpy16(string, valueNormalized >= 0.5 ? u8"On" : u8"Off");
                break;
        }
        return kResultOk;
    }

    tresult PLUGIN_API getParamValueByString(ParamID id, const char16* string, ParamValue& valueNormalized) const override {
        // Basic string parsing - in practice would be more sophisticated
        return kNotImplemented;
    }

    ITextEditController* PLUGIN_API createTextEditController(ParamID id) override {
        return nullptr; // Not implementing text editing
    }

    tresult PLUGIN_API setState(IBStream* state) override {
        // Load plugin state
        if (!state) return kInvalidArgument;
        
        float param;
        int32_t paramInt;
        bool paramBool;
        
        state->read(&param, sizeof(float)); cutoff_freq = param;
        state->read(&param, sizeof(float)); resonance = param;
        state->read(&param, sizeof(float)); gain = param;
        state->read(&paramInt, sizeof(int32_t)); filter_type = (filter_type_t)paramInt;
        state->read(&paramBool, sizeof(bool)); enabled = paramBool;
        
        filter_set_parameters(&filter, filter_type, cutoff_freq, resonance, gain);
        
        return kResultOk;
    }

    tresult PLUGIN_API getState(IBStream* state) override {
        // Save plugin state
        if (!state) return kInvalidArgument;
        
        state->write(&cutoff_freq, sizeof(float));
        state->write(&resonance, sizeof(float));
        state->write(&gain, sizeof(float));
        int32_t filterTypeInt = (int32_t)filter_type;
        state->write(&filterTypeInt, sizeof(int32_t));
        state->write(&enabled, sizeof(bool));
        
        return kResultOk;
    }

    void setCurrentProcessMode(ProcessMode mode) override {
        AudioProcessor::setCurrentProcessMode(mode);
    }

    void setCurrentBlockSize(int32 blockSize) override {
        AudioProcessor::setCurrentBlockSize(blockSize);
    }

    void setCurrentSampleRate(double sampleRate) override {
        current_sample_rate = sampleRate;
        AudioProcessor::setCurrentSampleRate(sampleRate);
    }

private:
    filter_t filter;
    float current_sample_rate;
    
    // Parameters
    float cutoff_freq;
    float resonance;
    float gain;
    filter_type_t filter_type;
    bool enabled;
};