/*
 * VST3 Parameter Controller
 * flark's MatrixFilter - VST3 Version
 */

#include "public.sdk/source/vst/vsteditcontroller.h"
#include "pluginterfaces/vst/ivsteditcontroller.h"
#include "pluginterfaces/vst/ivstcomponent.h"
#include "../src/dsp.h"

using namespace Steinberg;
using namespace Steinberg::Vst;

class MatrixFilterController : public EditController {
public:
    MatrixFilterController() : EditController() {
        // Component parameters
        parameters.addParameter(new Parameter("Cutoff Frequency", "Hz", 0, 20000, 1000, ParameterFlags::kCanAutomate));
        parameters.addParameter(new Parameter("Resonance", "", 0.1, 10.0, 1.0, ParameterFlags::kCanAutomate));
        parameters.addParameter(new Parameter("Gain", "dB", -60, 60, 0, ParameterFlags::kCanAutomate));
        parameters.addParameter(new Parameter("Filter Type", "", 0, 6, 0, ParameterFlags::kCanAutomate | ParameterFlags::kIsBypass));
        parameters.addParameter(new Parameter("Enabled", "", 0, 1, 1, ParameterFlags::kCanAutomate | ParameterFlags::kIsBypass));
    }

    tresult PLUGIN_API initialize(FUnknown* context) override {
        tresult result = EditController::initialize(context);
        if (result == kResultOk) {
            return kResultOk;
        }
        return result;
    }

    tresult PLUGIN_API setComponentState(IBStream* state) override {
        if (!state) return kInvalidArgument;
        
        // Read component state and update parameters
        float param;
        int32_t paramInt;
        bool paramBool;
        
        if (state->read(&param, sizeof(float)) == kResultOk) {
            parameters.getParameter(0)->setNormalized(param / 20000.0); // Cutoff
        }
        if (state->read(&param, sizeof(float)) == kResultOk) {
            parameters.getParameter(1)->setNormalized((param - 0.1) / (10.0 - 0.1)); // Resonance
        }
        if (state->read(&param, sizeof(float)) == kResultOk) {
            parameters.getParameter(2)->setNormalized((param + 60.0) / 120.0); // Gain
        }
        if (state->read(&paramInt, sizeof(int32_t)) == kResultOk) {
            parameters.getParameter(3)->setNormalized(paramInt / 6.0); // Filter Type
        }
        if (state->read(&paramBool, sizeof(bool)) == kResultOk) {
            parameters.getParameter(4)->setNormalized(paramBool ? 1.0 : 0.0); // Enabled
        }
        
        return kResultOk;
    }

    tresult PLUGIN_API setParamNormalized(ParamID id, ParamValue valueNormalized) override {
        tresult result = EditController::setParamNormalized(id, valueNormalized);
        return result;
    }

    ParamValue PLUGIN_API getParamNormalized(ParamID id) const override {
        return EditController::getParamNormalized(id);
    }

    tresult PLUGIN_API getParamStringByValue(ParamID id, ParamValue valueNormalized, String128 string) const override {
        switch (id) {
            case 0: // Cutoff
                sprintf16(string, u"%5.1f Hz", valueNormalized * 20000.0);
                break;
            case 1: // Resonance
                sprintf16(string, u"%.2f", 0.1 + valueNormalized * (10.0 - 0.1));
                break;
            case 2: // Gain
                sprintf16(string, u"%+.1f dB", -60.0 + valueNormalized * 120.0);
                break;
            case 3: // Filter Type
                {
                    const char* filterNames[] = {"Low-Pass", "High-Pass", "Band-Pass", "Notch", "Peaking", "Low Shelf", "High Shelf"};
                    int index = (int)(valueNormalized * 6.0);
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
        // Parse string values back to normalized parameters
        switch (id) {
            case 0: // Cutoff
                {
                    double value;
                    if (sscanf16(string, u"%lf", &value) == 1) {
                        valueNormalized = value / 20000.0;
                        return kResultOk;
                    }
                }
                break;
            case 4: // Enabled
                {
                    String s(string);
                    if (s == u8"On" || s == u8"on") {
                        valueNormalized = 1.0;
                        return kResultOk;
                    } else if (s == u8"Off" || s == u8"off") {
                        valueNormalized = 0.0;
                        return kResultOk;
                    }
                }
                break;
        }
        return kNotImplemented;
    }

    ITextEditController* PLUGIN_API createTextEditController(ParamID id) override {
        // Only allow text editing for certain parameters
        if (id == 0 || id == 1 || id == 2) {
            return new TextEditController();
        }
        return nullptr;
    }

    // Simple text edit controller
    class TextEditController : public ITextEditController {
    public:
        tresult PLUGIN_API setText(ParamID id, const char16* text) override {
            return kNotImplemented;
        }
        tresult PLUGIN_API getText(ParamID id, String128 text) override {
            return kNotImplemented;
        }
        tresult PLUGIN_API setVisible(ParamID id, TBool visible) override {
            return kNotImplemented;
        }
        Steinberg::uint32 PLUGIN_API getVersion() const override {
            return 1;
        }
    };
};