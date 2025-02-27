# Midiverse

A C++ tool that renders MIDI files to audio using VST plugins.

## Overview

Midiverse allows you to convert MIDI files to audio by processing them through VST plugins (or using a fallback sine wave generator). It provides both a command-line interface and Python wrapper for easy integration.

## Features

- Process MIDI files through VST plugins
- Customize sample rate, bit depth, and channel count
- JUCE integration for VST3, VST2, AU support (optional)
- Fallback mode with simple sine wave generation
- Simple command-line interface

## Requirements

- C++17 compatible compiler
- CMake 3.10 or higher
- JUCE (for VST support, included with install script, optional)

## Getting Started

### Installing Dependencies

Run the included installation script to download dependencies:

```bash
./install_deps.sh
```

### Building

#### With VST Support (using JUCE)

```bash
mkdir -p build
cd build
cmake -DUSE_JUCE=ON ..
make
```

#### Without VST Support (minimal build)

If you just want to test with the fallback sine wave generator:

```bash
mkdir -p build
cd build
cmake ..
make
```

## Usage

### Command Line Interface

Use the CLI tool directly:

```bash
./build/midiverse_cli <midi_file> <vst_plugin> [options]
```

Options:
```
  -o, --output <file>      Output file path (default: output.wav)
  -r, --rate <rate>        Sample rate in Hz (default: 44100)
  -c, --channels <num>     Number of channels (default: 2)
  -b, --bit-depth <depth>  Bit depth (default: 16)
  -h, --help               Show this help message
```

### Python Wrapper

A Python wrapper is provided for easier use:

```bash
./midiverse.py <midi_file> <vst_plugin> [options]
```

Options are the same as the CLI tool.

### Example

```bash
# Generate test MIDI files
python examples/generate_test_midi.py

# Process with dummy VST (generates sine wave)
./midiverse.py test_scale.mid ./dummy_vst.vst -o rendered_scale.wav
```

## VST Support

By default, Midiverse runs in a fallback mode that generates sine wave audio instead of using actual VST plugins. This is useful for testing or when you don't have VST plugins available.

### Full VST Support (including VST3)

When built with JUCE support (`-DUSE_JUCE=ON`), Midiverse can load and render audio through real VST plugins:

- VST3 plugins (.vst3)
- VST2 plugins (.dll, .vst, .so)
- Audio Units (.component) on macOS

To enable VST3 support, you must build with the JUCE option:
```bash
mkdir -p build && cd build && cmake -DUSE_JUCE=ON .. && make
```

The JUCE integration provides:

1. Plugin loading and format detection
2. MIDI file parsing and event processing
3. Accurate timing and audio processing
4. Audio rendering at the specified sample rate and channel count

### Plugin Compatibility Notes

- VST3 plugins are recommended for best compatibility
- Plugin paths must be absolute paths
- The application needs read/write access to the plugin files
- Some plugins require initialization parameters; these are not yet supported

## Implementation Notes

Midiverse consists of several components:

1. **MidiProcessor**: Parses and processes MIDI files
2. **VstRenderer**: Renders MIDI data through VST plugins (or fallback generator)
3. **AudioWriter**: Writes audio data to WAV files

The application can run in two modes:
- Full mode with JUCE integration for VST support
- Minimal mode with a sine wave generator for testing

## Troubleshooting

### Common Issues

- **"Failed to load VST plugin"**: Check that the plugin path is correct and the plugin format is supported
- **"No suitable plugin format found"**: The file extension may not be recognized. Try a different plugin format.
- **"Failed to parse MIDI data"**: The MIDI file may be corrupted or in an unsupported format.

## License

This project is licensed under the MIT License - see the LICENSE file for details.

## Acknowledgments

- [JUCE](https://github.com/juce-framework/JUCE) - Cross-platform C++ framework for audio applications
- [Crow](https://github.com/CrowCpp/Crow) - C++ microframework for web (server implementation)
