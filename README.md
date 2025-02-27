# Midiverse

A C++ service that renders MIDI files to audio using VST plugins.

## Overview

Midiverse is a web service that allows you to convert MIDI files to audio by processing them through VST plugins. It provides a simple HTTP API for uploading MIDI files, specifying VST instruments, and downloading the rendered audio.

## Features

- Process MIDI files through VST plugins
- Customize sample rate, bit depth, and channel count
- RESTful API with JSON support
- Download rendered audio as WAV files
- JUCE integration for VST3, VST2, AU support
- Fallback mode with simple sine wave generation

## Requirements

- C++17 compatible compiler
- CMake 3.10 or higher
- Crow (C++ web framework, included with install script)
- JUCE (for VST support, included with install script)

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

If you just want to test the service without VST support, it will generate simple sine wave melodies instead.

```bash
mkdir -p build
cd build
cmake ..
make
```

### Running the Server

Start the server with an optional port number:

```bash
./midiverse [port]
```

The default port is 8080 if not specified.

## API Endpoints

### Health Check

```
GET /health
```

Returns "OK" if the server is running.

### Render MIDI to Audio

```
POST /render
```

Request body (JSON):

```json
{
  "midiFile": "/path/to/file.mid",
  "vstPath": "/path/to/plugin.vst",
  "sampleRate": 44100,
  "numChannels": 2,
  "bitDepth": 16
}
```

Response:

```json
{
  "status": "success",
  "outputFile": "output/file_plugin_44100hz.wav"
}
```

### Download Rendered Audio

```
GET /download/{filename}
```

Downloads the rendered audio file.

## Client Example

A Python client example is provided in the `examples` directory:

```bash
# Install required Python packages
pip install requests

# Run the client
python examples/client.py path/to/input.mid path/to/plugin.vst
```

Additional options:

```bash
python examples/client.py --help
```

## JUCE Integration

When built with JUCE support (`-DUSE_JUCE=ON`), Midiverse can load and render audio through:

- VST3 plugins (.vst3)
- VST2 plugins (.dll, .vst, .so)
- Audio Units (.component) on macOS

The JUCE integration provides:

1. Plugin loading and format detection
2. MIDI file parsing and event processing
3. Accurate timing and audio processing
4. Audio rendering at the specified sample rate and channel count

### Plugin Compatibility Notes

- VST3 plugins are recommended for best compatibility
- Plugin paths must be absolute paths
- The service needs read/write access to the plugin files
- Some plugins require initialization parameters; these are not yet supported

## Troubleshooting

### Common Issues

- **"Failed to load VST plugin"**: Check that the plugin path is correct and the plugin format is supported
- **"No suitable plugin format found"**: The file extension may not be recognized. Try a different plugin format.
- **"Failed to parse MIDI data"**: The MIDI file may be corrupted or in an unsupported format.

### Logs

The service logs details about MIDI files and VST loading to the console, which can help diagnose issues.

## License

This project is licensed under the MIT License - see the LICENSE file for details.

## Acknowledgments

- [Crow](https://github.com/CrowCpp/Crow) - C++ microframework for web
- [JUCE](https://github.com/juce-framework/JUCE) - Cross-platform C++ framework for audio applications# midiverse
