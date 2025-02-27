#include "audio_writer.h"
#include <iostream>
#include <cstdio>
#include <cstring>

AudioWriter::AudioWriter() {
}

AudioWriter::~AudioWriter() {
}

bool AudioWriter::writeWavFile(const std::string& filePath, const std::vector<float>& audioData, 
                             float sampleRate, int numChannels, int bitDepth) {
    if (audioData.empty()) {
        std::cerr << "No audio data to write" << std::endl;
        return false;
    }
    
    FILE* file = fopen(filePath.c_str(), "wb");
    if (!file) {
        std::cerr << "Could not open file for writing: " << filePath << std::endl;
        return false;
    }
    
    // Calculate sizes
    int bytesPerSample = bitDepth / 8;
    int numSamples = static_cast<int>(audioData.size());
    int dataSize = numSamples * bytesPerSample;
    
    // Write WAV header
    if (!writeWavHeader(file, dataSize, sampleRate, numChannels, bitDepth)) {
        fclose(file);
        return false;
    }
    
    // Convert float samples to the specified bit depth
    std::vector<uint8_t> rawData(dataSize);
    
    for (int i = 0; i < numSamples; ++i) {
        float sample = audioData[i];
        
        // Clamp sample to [-1.0, 1.0]
        if (sample > 1.0f) sample = 1.0f;
        if (sample < -1.0f) sample = -1.0f;
        
        // Convert to integer and write to buffer
        if (bitDepth == 16) {
            int16_t pcm = static_cast<int16_t>(sample * 32767.0f);
            memcpy(&rawData[i * bytesPerSample], &pcm, bytesPerSample);
        } else if (bitDepth == 24) {
            int32_t pcm = static_cast<int32_t>(sample * 8388607.0f);
            uint8_t bytes[3];
            bytes[0] = pcm & 0xFF;
            bytes[1] = (pcm >> 8) & 0xFF;
            bytes[2] = (pcm >> 16) & 0xFF;
            memcpy(&rawData[i * bytesPerSample], bytes, bytesPerSample);
        } else if (bitDepth == 32) {
            int32_t pcm = static_cast<int32_t>(sample * 2147483647.0f);
            memcpy(&rawData[i * bytesPerSample], &pcm, bytesPerSample);
        } else {
            std::cerr << "Unsupported bit depth: " << bitDepth << std::endl;
            fclose(file);
            return false;
        }
    }
    
    // Write audio data
    size_t written = fwrite(rawData.data(), 1, dataSize, file);
    fclose(file);
    
    if (written != static_cast<size_t>(dataSize)) {
        std::cerr << "Failed to write all audio data" << std::endl;
        return false;
    }
    
    std::cout << "Successfully wrote WAV file: " << filePath << std::endl;
    std::cout << "  Sample rate: " << sampleRate << " Hz" << std::endl;
    std::cout << "  Channels: " << numChannels << std::endl;
    std::cout << "  Bit depth: " << bitDepth << " bits" << std::endl;
    std::cout << "  Duration: " << numSamples / numChannels / sampleRate << " seconds" << std::endl;
    
    return true;
}

bool AudioWriter::writeWavHeader(FILE* file, int dataSize, float sampleRate, 
                               int numChannels, int bitDepth) {
    // RIFF header
    fwrite("RIFF", 1, 4, file);
    
    // File size (will be filled later)
    int fileSizeMinusRiff = 36 + dataSize;  // 36 = size of the rest of the header
    fwrite(&fileSizeMinusRiff, 4, 1, file);
    
    // WAV marker & format chunk
    fwrite("WAVEfmt ", 1, 8, file);
    
    // Format chunk size
    int formatChunkSize = 16;
    fwrite(&formatChunkSize, 4, 1, file);
    
    // Audio format (1 = PCM)
    short audioFormat = 1;
    fwrite(&audioFormat, 2, 1, file);
    
    // Number of channels
    short numChannelsShort = static_cast<short>(numChannels);
    fwrite(&numChannelsShort, 2, 1, file);
    
    // Sample rate
    int sampleRateInt = static_cast<int>(sampleRate);
    fwrite(&sampleRateInt, 4, 1, file);
    
    // Byte rate
    int byteRate = sampleRateInt * numChannels * (bitDepth / 8);
    fwrite(&byteRate, 4, 1, file);
    
    // Block align
    short blockAlign = numChannels * (bitDepth / 8);
    fwrite(&blockAlign, 2, 1, file);
    
    // Bits per sample
    short bitsPerSample = static_cast<short>(bitDepth);
    fwrite(&bitsPerSample, 2, 1, file);
    
    // Data chunk marker
    fwrite("data", 1, 4, file);
    
    // Data size
    fwrite(&dataSize, 4, 1, file);
    
    return true;
}