/*
 * VST3 Plugin Entry Point and Factory
 * flark's MatrixFilter - VST3 Version
 */

#include "public.sdk/source/vst/vstinit.h"
#include "pluginterfaces/vst/ivstcomponent.h"
#include "pluginterfaces/vst/ivstaudioprocessor.h"
#include "pluginterfaces/vst/ivsteditcontroller.h"
#include "public.sdk/source/vst/vstaudioprocessor.h"
#include "public.sdk/source/vst/vsteditcontroller.h"
#include "processor.cpp"
#include "controller.cpp"
#include "editcontroller.cpp"

// Plugin Class ID - Generate a unique 128-bit ID
static const Steinberg::FUID PluginCID(0x5C52414B, 0x5241444952, 0x5846494C54, 0x45522D5653); // "FLARKMATRIX-FILTER-VST3"
static const Steinberg::FUID EditorCID(0x454449544F, 0x5241444952, 0x5846494C54, 0x45522D5645); // "EDITORMATRIX-FILTER-VSTE"

// Vendor and Product Information
static const char* kVendorName = "flark";
static const char* kProductName = "flark-MatrixFilter";
static const char* kProductVersion = "1.0.0";
static const char* kVendorVersion = "1.0.0";

// VST3 Plugin Capabilities
static const Steinberg::Vst::SpeakerArrangement kInput = Steinberg::Vst::kStereo;
static const Steinberg::Vst::SpeakerArrangement kOutput = Steinberg::Vst::kStereo;

// Plugin Information
class MatrixFilterComponent : public Steinberg::Vst::AudioComponent {
public:
    MatrixFilterComponent() : AudioComponent() {
        // Set plugin information
        setClassID(PluginCID);
        setName(String(kProductName));
        setVendor(String(kVendorName));
        setVersion(String(kVendorVersion));
        
        // Add audio input
        addAudioInput(String("Audio Input"), kInput, Steinberg::Vst::kMain, 1);
        addAudioOutput(String("Audio Output"), kOutput, Steinberg::Vst::kMain, 1);
        
        // Add editor component
        addComponent(new MatrixFilterEditor);
    }

    Steinberg::tresult PLUGIN_API initialize(Steinberg::FUnknown* context) override {
        return AudioComponent::initialize(context);
    }

    Steinberg::tresult PLUGIN_API terminate() override {
        return AudioComponent::terminate();
    }

    Steinberg::tresult PLUGIN_API connect(IConnectionPoint* other) override {
        return AudioComponent::connect(other);
    }

    Steinberg::tresult PLUGIN_API disconnect(IConnectionPoint* other) override {
        return AudioComponent::disconnect(other);
    }

    Steinberg::int32 PLUGIN_API getComponentCount() override {
        return 1; // Only one component
    }

    Steinberg::IComponent* PLUGIN_API getComponent(Steinberg::int32 index) override {
        if (index == 0) {
            return this;
        }
        return nullptr;
    }

    Steinberg::IConnectionPoint* PLUGIN_API getConnectionPoint(Steinberg::int32 index) override {
        return AudioComponent::getConnectionPoint(index);
    }

    Steinberg::tresult PLUGIN_API getName(Steinberg::String name) override {
        char string[256];
        sprintf(string, "%s %s %s", kVendorName, kProductName, kProductVersion);
        name.assign(string);
        return Steinberg::kResultOk;
    }

    Steinberg::tresult PLUGIN_API canProcessSampleSize(Steinberg::int32 symbolicSampleSize) override {
        return Steinberg::kResultOk; // Support both 32-bit and 64-bit
    }

    Steinberg::tresult PLUGIN_API getIcon(Steinberg::IBitmap** icon) override {
        return Steinberg::kNotImplemented;
    }

    Steinberg::tresult PLUGIN_API setIcon(Steinberg::IBitmap* icon) override {
        return Steinberg::kNotImplemented;
    }

    Steinberg::tresult PLUGIN_API getWorldInfo(Steinberg::IWorldInfo* worldInfo) override {
        return Steinberg::kNotImplemented;
    }

private:
    // Internal editor component class
    class MatrixFilterEditor : public Steinberg::Vst::EditorView {
    public:
        MatrixFilterEditor() : EditorView() {
            setEditor(new MatrixFilterEditController);
        }
        
        Steinberg::tresult PLUGIN_API open(void* parent, const char* type) override {
            return EditorView::open(parent, type);
        }
        
        Steinberg::tresult PLUGIN_API close() override {
            return EditorView::close();
        }
        
        Steinberg::tresult PLUGIN_API setZoom(float zoomFactor) override {
            return EditorView::setZoom(zoomFactor);
        }
        
        Steinberg::tresult PLUGIN_API canResize() override {
            return Steinberg::kResultTrue;
        }
        
        Steinberg::tresult PLUGIN_API checkSizeConstraint(Steinberg::ViewRect* rect) override {
            if (!rect) return Steinberg::kInvalidArgument;
            
            // Minimum and maximum size constraints
            rect->right = std::min(rect->right, (Steinberg::int32)(rect->left + 800));
            rect->bottom = std::min(rect->bottom, (Steinberg::int32)(rect->top + 600));
            rect->left = std::max(rect->left, (Steinberg::int32)(rect->right - 600));
            rect->top = std::max(rect->top, (Steinberg::int32)(rect->bottom - 400));
            
            return Steinberg::kResultOk;
        }
    };
};

// Plugin Factory - creates plugin instances
class MatrixFilterFactory : public Steinberg::FUnknown {
public:
    Steinberg::tresult PLUGIN_API createInstance(Steinberg::FUID classID, Steinberg::FUID interfaceID, void** obj) override {
        if (classID == PluginCID) {
            if (interfaceID == Steinberg::Vst::IComponent::iid) {
                *obj = new MatrixFilterComponent();
                return Steinberg::kResultOk;
            }
        } else if (classID == EditorCID) {
            if (interfaceID == Steinberg::Vst::IEditController::iid) {
                *obj = new MatrixFilterEditController();
                return Steinberg::kResultOk;
            }
        }
        return Steinberg::kNoInterface;
    }
};

// Plugin entry point
extern "C" {
    Steinberg::FUnknown* createInstance(Steinberg::FUID classID, Steinberg::FUID interfaceID) {
        static MatrixFilterFactory factory;
        
        if (classID == PluginCID || interfaceID == Steinberg::Vst::IComponent::iid) {
            return &factory;
        }
        return nullptr;
    }
}

// VST3 Plugin Factory
BEGIN_FACTORY_DEF(
    kVendorName,
    "https://flark.dev/matrixfilter",
    "mailto:contact@flark.dev"
)

DEF_CLASS(
    Steinberg::FUID::fromString("FLARKMATRIX-FILTER-VST3-001"), // Class ID
    Steinberg::Vst::kDistributable,                              // Flags
    kProductName,                                               // Name
    kVendorVersion,                                             // Version
    kProductVersion,                                            // SDK Version
    Steinberg::Vst::kNoFlags,                                   // Effects
    0,                                                          // CPU Requirements
    0                                                           // Number of inner structures
)

DEF_CLASS(
    Steinberg::FUID::fromString("EDITORMATRIX-FILTER-VSTE-001"), // Editor Class ID
    Steinberg::Vst::kDistributable | Steinberg::Vst::kHiddenComponent, // Flags
    String("Matrix Filter Editor"),                              // Name
    kVendorVersion,                                             // Version
    kProductVersion,                                            // SDK Version
    Steinberg::Vst::kNoFlags,                                   // Effects
    0,                                                          // CPU Requirements
    0                                                           // Number of inner structures
)

CLASS_FACTORY_END

// Module entry points
extern "C" {
    Steinberg::FUnknown* PLUGIN_API createInstance(Steinberg::FUID classID, Steinberg::FUID interfaceID) {
        return ::createInstance(classID, interfaceID);
    }
    
    Steinberg::int32 PLUGIN_API GetPluginFactoryCount() {
        return 1;
    }
    
    Steinberg::IPluginFactory* PLUGIN_API GetPluginFactory(Steinberg::int32 index) {
        if (index == 0) {
            static Steinberg::IPluginFactory factory = {
                &gPluginFactoryInfo,
                createInstance,
                nullptr, // setHostCallback
                nullptr  // getVendorSpecific
            };
            return &factory;
        }
        return nullptr;
    }
}