#include "vst_renderer.h"
#include <iostream>
#include <stdexcept>
#include <cmath>

#ifdef USE_JUCE
// Include JUCE headers when built with JUCE support
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_audio_formats/juce_audio_formats.h>
#include <juce_audio_devices/juce_audio_devices.h>
#include <juce_audio_utils/juce_audio_utils.h>
#endif

VstRenderer::VstRenderer() 
#ifndef USE_JUCE
    : vstInstance(nullptr)
#endif
{
#ifdef USE_JUCE
    // Initialize JUCE components
    juce::MessageManager::getInstance();
    formatManager = std::make_unique<juce::AudioPluginFormatManager>();
    formatManager->addDefaultFormats();
#endif
}

VstRenderer::~VstRenderer() {
#ifdef USE_JUCE
    // JUCE cleanup happens automatically through unique_ptr
    juce::MessageManager::getInstance()->deleteInstance();
#else
    // Clean up dummy VST instance if loaded
    if (vstInstance) {
        vstInstance = nullptr;
    }
#endif
}

bool VstRenderer::loadVst(const std::string& vstPath) {
    this->vstPath = vstPath;
    
#ifdef USE_JUCE
    return loadVstWithJuce(vstPath);
#else
    std::cout << "Loading VST plugin (dummy mode): " << vstPath << std::endl;
    std::cout << "Note: Built without JUCE support. Using dummy audio generation." << std::endl;
    
    // Simulate successful loading with a dummy pointer
    vstInstance = reinterpret_cast<void*>(1);
    return true;
#endif
}

bool VstRenderer::renderMidi(const std::vector<uint8_t>& midiData, float sampleRate, int numChannels) {
#ifdef USE_JUCE
    return renderMidiWithJuce(midiData, sampleRate, numChannels);
#else
    if (!vstInstance) {
        std::cerr << "No VST plugin loaded" << std::endl;
        return false;
    }
    
    std::cout << "Rendering MIDI through dummy audio generator..." << std::endl;
    return renderDummyAudio(sampleRate, numChannels);
#endif
}

const std::vector<float>& VstRenderer::getAudioData() const {
    return audioData;
}

#ifdef USE_JUCE
//===== JUCE-specific implementations =====

bool VstRenderer::loadVstWithJuce(const std::string& vstPath) {
    std::cout << "Loading VST plugin with JUCE: " << vstPath << std::endl;
    
    juce::String errorMessage;
    
    // Determine plugin format based on file extension
    juce::String pluginPath = juce::String(vstPath);
    juce::AudioPluginFormat* format = nullptr;
    
    // Look for the appropriate format
    for (int i = 0; i < formatManager->getNumFormats(); ++i) {
        format = formatManager->getFormat(i);
        if (format->fileMightContainThisPluginType(pluginPath)) {
            break;
        }
    }
    
    if (format == nullptr) {
        std::cerr << "No suitable plugin format found for: " << vstPath << std::endl;
        return false;
    }
    
    // Create plugin instance
    std::unique_ptr<juce::PluginDescription> description = std::make_unique<juce::PluginDescription>();
    description->fileOrIdentifier = pluginPath;
    
    vstInstance.reset(format->createInstanceFromDescription(*description, 44100, 512, errorMessage));
    
    if (vstInstance == nullptr) {
        std::cerr << "Failed to load VST plugin: " << errorMessage.toStdString() << std::endl;
        return false;
    }
    
    // Initialize the plugin
    vstInstance->prepareToPlay(44100, 512);
    
    std::cout << "Successfully loaded VST plugin: " << vstInstance->getName().toStdString() << std::endl;
    return true;
}

std::unique_ptr<juce::MidiFile> VstRenderer::parseMidiData(const std::vector<uint8_t>& midiData) {
    // Create a memory input stream from the MIDI data
    juce::MemoryInputStream inputStream(midiData.data(), midiData.size(), false);
    
    // Parse the MIDI file
    auto midiFile = std::make_unique<juce::MidiFile>();
    if (midiFile->readFrom(inputStream)) {
        return midiFile;
    }
    
    return nullptr;
}

bool VstRenderer::renderMidiWithJuce(const std::vector<uint8_t>& midiData, float sampleRate, int numChannels) {
    if (!vstInstance) {
        std::cerr << "No VST plugin loaded" << std::endl;
        return false;
    }
    
    std::cout << "Rendering MIDI through VST plugin with JUCE..." << std::endl;
    std::cout << "Sample rate: " << sampleRate << " Hz" << std::endl;
    std::cout << "Channels: " << numChannels << std::endl;
    
    // Parse the MIDI file
    auto midiFile = parseMidiData(midiData);
    if (!midiFile) {
        std::cerr << "Failed to parse MIDI data" << std::endl;
        return false;
    }
    
    // Prepare the plugin for playback
    vstInstance->prepareToPlay(sampleRate, 512);
    
    // Determine the length of the MIDI sequence
    double totalTimeInSeconds = 0.0;
    for (int track = 0; track < midiFile->getNumTracks(); ++track) {
        const juce::MidiMessageSequence* sequence = midiFile->getTrack(track);
        if (sequence->getEndTime() > totalTimeInSeconds) {
            totalTimeInSeconds = sequence->getEndTime();
        }
    }
    
    // Add 2 seconds for reverb/release tail
    totalTimeInSeconds += 2.0;
    
    // Calculate total number of samples
    int totalSamples = static_cast<int>(totalTimeInSeconds * sampleRate);
    
    // Create audio buffer for processing
    juce::AudioBuffer<float> tempBuffer(numChannels, 512);
    audioData.resize(totalSamples * numChannels);
    
    // Convert MIDI file to a sequence of MIDI messages
    juce::MidiBuffer midiBuffer;
    for (int track = 0; track < midiFile->getNumTracks(); ++track) {
        const juce::MidiMessageSequence* sequence = midiFile->getTrack(track);
        for (int i = 0; i < sequence->getNumEvents(); ++i) {
            auto event = sequence->getEventPointer(i);
            int samplePosition = static_cast<int>(event->message.getTimeStamp() * sampleRate);
            midiBuffer.addEvent(event->message, samplePosition);
        }
    }
    
    // Process audio in blocks
    int samplesRemaining = totalSamples;
    int currentPosition = 0;
    
    juce::MidiBuffer::Iterator it(midiBuffer);
    juce::MidiMessage message;
    int samplePosition = 0;
    
    juce::MidiBuffer blockMidi;
    
    while (samplesRemaining > 0) {
        // Clear the temp buffer
        tempBuffer.clear();
        
        // Get number of samples to process this iteration
        int blockSize = std::min(512, samplesRemaining);
        
        // Get MIDI events for this block
        blockMidi.clear();
        while (it.getNextEvent(message, samplePosition) && 
               samplePosition < currentPosition + blockSize) {
            blockMidi.addEvent(message, samplePosition - currentPosition);
        }
        
        // Process the block
        juce::AudioBuffer<float> inputBuffer(numChannels, blockSize);
        juce::AudioBuffer<float> outputBuffer(numChannels, blockSize);
        
        // Clear the buffers
        inputBuffer.clear();
        outputBuffer.clear();
        
        // Process audio
        juce::MidiBuffer blockMidiCopy(blockMidi);
        vstInstance->processBlock(outputBuffer, blockMidiCopy);
        
        // Copy processed audio to our output vector
        for (int channel = 0; channel < numChannels; ++channel) {
            const float* channelData = outputBuffer.getReadPointer(channel);
            for (int sample = 0; sample < blockSize; ++sample) {
                audioData[(currentPosition + sample) * numChannels + channel] = channelData[sample];
            }
        }
        
        // Update position
        currentPosition += blockSize;
        samplesRemaining -= blockSize;
    }
    
    // Clean up
    vstInstance->releaseResources();
    
    std::cout << "Rendering complete. Generated " << audioData.size() / numChannels 
              << " samples (" << audioData.size() / numChannels / sampleRate 
              << " seconds)" << std::endl;
    
    return true;
}

#else
//===== Dummy implementation (no JUCE) =====

bool VstRenderer::renderDummyAudio(float sampleRate, int numChannels) {
    std::cout << "Generating dummy audio (sine wave)..." << std::endl;
    
    // Generate 5 seconds of audio at the given sample rate
    size_t numSamples = static_cast<size_t>(sampleRate * 5 * numChannels);
    audioData.resize(numSamples);
    
    // Simple melody using sine waves
    const float frequencies[] = {261.63f, 293.66f, 329.63f, 349.23f, 392.00f, 440.00f, 493.88f, 523.25f};
    const float noteDuration = 0.5f; // half second per note
    const float amplitude = 0.5f;
    
    for (size_t i = 0; i < numSamples; i += numChannels) {
        float timeInSeconds = static_cast<float>(i / numChannels) / sampleRate;
        int noteIndex = static_cast<int>(timeInSeconds / noteDuration) % 8;
        float noteTime = fmod(timeInSeconds, noteDuration);
        
        // Apply envelope (simple attack/decay)
        float envelope = 1.0f;
        if (noteTime < 0.05f) {
            // Attack
            envelope = noteTime / 0.05f;
        } else if (noteTime > noteDuration - 0.1f) {
            // Release
            envelope = (noteDuration - noteTime) / 0.1f;
        }
        
        float sample = amplitude * envelope * 
                      std::sin(2.0f * 3.14159f * frequencies[noteIndex] * timeInSeconds);
        
        // Write to all channels
        for (int c = 0; c < numChannels; ++c) {
            audioData[i + c] = sample;
        }
    }
    
    std::cout << "Generated " << audioData.size() / numChannels 
              << " samples (" << audioData.size() / numChannels / sampleRate 
              << " seconds) of dummy audio" << std::endl;
    
    return true;
}
#endif