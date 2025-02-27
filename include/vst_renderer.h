#pragma once

#include <string>
#include <vector>
#include <memory>

// Forward declarations for JUCE classes
namespace juce {
    class AudioPluginFormatManager;
    class AudioPluginInstance;
    class MidiFile;
    class AudioBuffer;
}

class VstRenderer {
public:
    VstRenderer();
    ~VstRenderer();

    bool loadVst(const std::string& vstPath);
    bool renderMidi(const std::vector<uint8_t>& midiData, float sampleRate, int numChannels);
    const std::vector<float>& getAudioData() const;
    
private:
    std::string vstPath;
    std::vector<float> audioData;
    
    // JUCE specific members (only used when built with JUCE)
    #ifdef USE_JUCE
    std::unique_ptr<juce::AudioPluginFormatManager> formatManager;
    std::unique_ptr<juce::AudioPluginInstance> vstInstance;
    
    bool loadVstWithJuce(const std::string& vstPath);
    bool renderMidiWithJuce(const std::vector<uint8_t>& midiData, float sampleRate, int numChannels);
    std::unique_ptr<juce::MidiFile> parseMidiData(const std::vector<uint8_t>& midiData);
    #else
    void* vstInstance; // Dummy placeholder when not using JUCE
    
    bool renderDummyAudio(float sampleRate, int numChannels);
    #endif
};