# LV2 Plugin for flark's MatrixFilter  
# Linux Audio LADSPA plugin
# RDF metadata and extension support

Plugin URI: http://flark.dev/matrixfilter
LV2 Version: 1.18.0
Format: Audio Effect Plugin

Features:
- Matrix-style visual effects
- Biquad filter implementation
- OpenGL rendering
- Spectrum analysis
- Cross-plugin communication

Required Extensions:
- lv2:Port
- lv2:GUI
- http://flark.dev/matrixfilter/ui

Installation:
- Plugin: ~/.lv2/flark-matrixfilter.lv2/
- Manifest: include manifest.ttl
- Library: flark-matrixfilter.so

Linux DAW Compatibility:
- Ardour
- Hydrogen
- LMMS
- Qtractor
- Reaper (Linux)

Visit: https://flark.dev/matrixfilter for documentation
