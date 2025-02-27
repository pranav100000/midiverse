#include "../include/midi_processor.h"
#include "../include/vst_renderer.h"
#include "../include/audio_writer.h"

#include <iostream>
#include <string>
#include <filesystem>

namespace fs = std::filesystem;

void printUsage(const char* programName) {
    std::cout << "Usage: " << programName << " <midi_file> <vst_plugin> [options]" << std::endl;
    std::cout << std::endl;
    std::cout << "Options:" << std::endl;
    std::cout << "  -o, --output <file>      Output file path (default: output.wav)" << std::endl;
    std::cout << "  -r, --rate <rate>        Sample rate in Hz (default: 44100)" << std::endl;
    std::cout << "  -c, --channels <num>     Number of channels (default: 2)" << std::endl;
    std::cout << "  -b, --bit-depth <depth>  Bit depth (default: 16)" << std::endl;
    std::cout << "  -h, --help               Show this help message" << std::endl;
}

int main(int argc, char* argv[]) {
    if (argc < 3) {
        printUsage(argv[0]);
        return 1;
    }
    
    // Parse command line arguments
    std::string midiFile;
    std::string vstPath;
    std::string outputFile = "output.wav";
    float sampleRate = 44100;
    int numChannels = 2;
    int bitDepth = 16;
    
    // First two arguments are midi file and vst plugin
    midiFile = argv[1];
    vstPath = argv[2];
    
    // Parse optional arguments
    for (int i = 3; i < argc; i++) {
        std::string arg = argv[i];
        
        if (arg == "-h" || arg == "--help") {
            printUsage(argv[0]);
            return 0;
        } else if (arg == "-o" || arg == "--output") {
            if (i + 1 < argc) {
                outputFile = argv[++i];
            } else {
                std::cerr << "Error: Output file path required" << std::endl;
                return 1;
            }
        } else if (arg == "-r" || arg == "--rate") {
            if (i + 1 < argc) {
                sampleRate = std::stof(argv[++i]);
            } else {
                std::cerr << "Error: Sample rate required" << std::endl;
                return 1;
            }
        } else if (arg == "-c" || arg == "--channels") {
            if (i + 1 < argc) {
                numChannels = std::stoi(argv[++i]);
            } else {
                std::cerr << "Error: Number of channels required" << std::endl;
                return 1;
            }
        } else if (arg == "-b" || arg == "--bit-depth") {
            if (i + 1 < argc) {
                bitDepth = std::stoi(argv[++i]);
            } else {
                std::cerr << "Error: Bit depth required" << std::endl;
                return 1;
            }
        } else {
            std::cerr << "Unknown option: " << arg << std::endl;
            printUsage(argv[0]);
            return 1;
        }
    }
    
    // Validate inputs
    if (!fs::exists(midiFile)) {
        std::cerr << "Error: MIDI file not found: " << midiFile << std::endl;
        return 1;
    }
    
    if (!fs::exists(vstPath)) {
        std::cerr << "Error: VST plugin not found: " << vstPath << std::endl;
        return 1;
    }
    
    // Create output directory if needed
    fs::path outputPath(outputFile);
    fs::path outputDir = outputPath.parent_path();
    if (!outputDir.empty() && !fs::exists(outputDir)) {
        if (!fs::create_directories(outputDir)) {
            std::cerr << "Error: Failed to create output directory: " << outputDir << std::endl;
            return 1;
        }
    }
    
    // Initialize components
    MidiProcessor midiProcessor;
    VstRenderer vstRenderer;
    AudioWriter audioWriter;
    
    try {
        // Load and process MIDI file
        std::cout << "Loading MIDI file: " << midiFile << std::endl;
        if (!midiProcessor.loadMidiFile(midiFile)) {
            std::cerr << "Error: Failed to load MIDI file" << std::endl;
            return 1;
        }
        
        // Load VST plugin
        std::cout << "Loading VST plugin: " << vstPath << std::endl;
        if (!vstRenderer.loadVst(vstPath)) {
            std::cerr << "Error: Failed to load VST plugin" << std::endl;
            return 1;
        }
        
        // Render MIDI through VST
        std::cout << "Rendering MIDI with VST plugin..." << std::endl;
        std::cout << "Sample rate: " << sampleRate << " Hz" << std::endl;
        std::cout << "Channels: " << numChannels << std::endl;
        std::cout << "Bit depth: " << bitDepth << " bits" << std::endl;
        
        if (!vstRenderer.renderMidi(midiProcessor.getMidiData(), sampleRate, numChannels)) {
            std::cerr << "Error: Failed to render MIDI" << std::endl;
            return 1;
        }
        
        // Write to WAV file
        std::cout << "Writing to output file: " << outputFile << std::endl;
        if (!audioWriter.writeWavFile(outputFile, vstRenderer.getAudioData(), 
                                     sampleRate, numChannels, bitDepth)) {
            std::cerr << "Error: Failed to write audio file" << std::endl;
            return 1;
        }
        
        std::cout << "Successfully rendered MIDI to audio!" << std::endl;
        std::cout << "Output file: " << fs::absolute(outputFile) << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}