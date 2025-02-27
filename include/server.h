#pragma once

#include <string>
#include <crow.h>
#include "midi_processor.h"
#include "vst_renderer.h"
#include "audio_writer.h"

class Server {
public:
    Server(int port = 8080);
    ~Server();

    void start();
    void stop();

private:
    int port;
    MidiProcessor midiProcessor;
    VstRenderer vstRenderer;
    AudioWriter audioWriter;
    crow::SimpleApp app; // Store the app instance
    
    void setupRoutes();
    std::string handleRenderRequest(const std::string& midiFilePath, 
                                  const std::string& vstPath,
                                  float sampleRate = 44100,
                                  int numChannels = 2,
                                  int bitDepth = 16);
};