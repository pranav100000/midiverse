#pragma once

#include <string>
#include <vector>

class AudioWriter {
public:
    AudioWriter();
    ~AudioWriter();

    bool writeWavFile(const std::string& filePath, const std::vector<float>& audioData, 
                     float sampleRate, int numChannels, int bitDepth = 16);
    
private:
    bool writeWavHeader(FILE* file, int dataSize, float sampleRate, 
                       int numChannels, int bitDepth);
};