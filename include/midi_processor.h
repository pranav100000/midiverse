#pragma once

#include <string>
#include <vector>

class MidiProcessor {
public:
    MidiProcessor();
    ~MidiProcessor();

    bool loadMidiFile(const std::string& filePath);
    const std::vector<uint8_t>& getMidiData() const;
    int getTrackCount() const;
    int getTicksPerQuarterNote() const;
    
private:
    std::vector<uint8_t> midiData;
    int trackCount;
    int ticksPerQuarterNote;
};