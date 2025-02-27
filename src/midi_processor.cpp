#include "midi_processor.h"
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <sstream>
#include <algorithm>
#include <iomanip>

MidiProcessor::MidiProcessor() : trackCount(0), ticksPerQuarterNote(0) {
}

MidiProcessor::~MidiProcessor() {
}

bool MidiProcessor::loadMidiFile(const std::string& filePath) {
    std::ifstream file(filePath, std::ios::binary);
    if (!file) {
        std::cerr << "Could not open MIDI file: " << filePath << std::endl;
        return false;
    }
    
    // Clear previous data
    midiData.clear();
    
    // Read entire file into memory
    file.seekg(0, std::ios::end);
    size_t fileSize = file.tellg();
    file.seekg(0, std::ios::beg);
    
    if (fileSize < 14) {
        std::cerr << "MIDI file too small: " << filePath << std::endl;
        return false;
    }
    
    midiData.resize(fileSize);
    file.read(reinterpret_cast<char*>(midiData.data()), fileSize);
    
    // Basic validation of MIDI header
    if (midiData[0] != 'M' || midiData[1] != 'T' || 
        midiData[2] != 'h' || midiData[3] != 'd') {
        std::cerr << "Invalid MIDI file format: Missing MThd header" << std::endl;
        midiData.clear();
        return false;
    }
    
    // Check header length (should be 6 for standard MIDI)
    uint32_t headerLength = (midiData[4] << 24) | (midiData[5] << 16) |
                           (midiData[6] << 8) | midiData[7];
    if (headerLength != 6) {
        std::cerr << "Warning: Unusual MIDI header length: " << headerLength << std::endl;
    }
    
    // Parse format type (0 = single track, 1 = multiple tracks, synchronized, 2 = multiple tracks, independent)
    uint16_t format = (midiData[8] << 8) | midiData[9];
    trackCount = (midiData[10] << 8) | midiData[11];
    ticksPerQuarterNote = (midiData[12] << 8) | midiData[13];
    
    // Validate MIDI format
    if (format > 2) {
        std::cerr << "Invalid MIDI format type: " << format << std::endl;
        midiData.clear();
        return false;
    }
    
    // Validate track count
    if (format == 0 && trackCount != 1) {
        std::cerr << "Warning: Format 0 MIDI should have exactly 1 track, found " << trackCount << std::endl;
    }
    
    // Log MIDI file information
    std::cout << "Loaded MIDI file: " << filePath << std::endl;
    std::cout << "Format: " << format << std::endl;
    std::cout << "Track count: " << trackCount << std::endl;
    std::cout << "Ticks per quarter note: " << ticksPerQuarterNote << std::endl;
    
    // Validate that we have at least one MTrk chunk
    bool foundTrack = false;
    size_t pos = 14; // Skip the header
    
    while (pos + 8 <= fileSize) {  // Need at least 8 bytes for chunk header
        if (midiData[pos] == 'M' && midiData[pos+1] == 'T' && 
            midiData[pos+2] == 'r' && midiData[pos+3] == 'k') {
            foundTrack = true;
            
            // Get track length
            uint32_t trackLength = (midiData[pos+4] << 24) | (midiData[pos+5] << 16) |
                                  (midiData[pos+6] << 8) | midiData[pos+7];
            
            std::cout << "Found track of length " << trackLength << " bytes" << std::endl;
            
            // Skip to next chunk
            pos += 8 + trackLength;
        } else {
            // Unknown chunk, try to skip it if we can read its length
            if (pos + 4 <= fileSize) {
                uint32_t chunkLength = (midiData[pos+4] << 24) | (midiData[pos+5] << 16) |
                                      (midiData[pos+6] << 8) | midiData[pos+7];
                std::cerr << "Warning: Unknown chunk at position " << pos << 
                          " with ID " << 
                          static_cast<char>(midiData[pos]) << 
                          static_cast<char>(midiData[pos+1]) << 
                          static_cast<char>(midiData[pos+2]) << 
                          static_cast<char>(midiData[pos+3]) << 
                          " and length " << chunkLength << std::endl;
                pos += 8 + chunkLength;
            } else {
                // Can't read length, just increment by 1 and hope for the best
                pos++;
            }
        }
    }
    
    if (!foundTrack) {
        std::cerr << "No track chunks found in MIDI file" << std::endl;
        midiData.clear();
        return false;
    }
    
    // Print a hexdump of the first few bytes for debugging
    std::cout << "MIDI file header hexdump:" << std::endl;
    std::stringstream ss;
    for (size_t i = 0; i < std::min(size_t(32), fileSize); ++i) {
        ss << std::setfill('0') << std::setw(2) << std::hex << static_cast<int>(midiData[i]) << " ";
        if ((i + 1) % 16 == 0) ss << std::endl;
    }
    std::cout << ss.str() << std::endl;
    
    return true;
}

const std::vector<uint8_t>& MidiProcessor::getMidiData() const {
    return midiData;
}

int MidiProcessor::getTrackCount() const {
    return trackCount;
}

int MidiProcessor::getTicksPerQuarterNote() const {
    return ticksPerQuarterNote;
}