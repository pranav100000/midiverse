#include "server.h"
#include <crow.h>
#include <filesystem>
#include <fstream>
#include <iostream>

namespace fs = std::filesystem;

Server::Server(int port) : port(port) {
}

Server::~Server() {
    stop();
}

void Server::start() {
    // Setup API routes
    setupRoutes();
    
    // Make output directory if it doesn't exist
    std::filesystem::path outputDir = "output";
    if (!std::filesystem::exists(outputDir)) {
        std::filesystem::create_directory(outputDir);
    }
    
    std::cout << "Starting server on port " << port << std::endl;
    
    // Set higher log level
    app.loglevel(crow::LogLevel::INFO);
    
    // Explicitly bind to 0.0.0.0 to accept connections from any interface
    app.bindaddr("0.0.0.0").port(port).multithreaded().run();
}

void Server::stop() {
    // Shutdown logic here
    // In a real-world app, you would use app.stop() here
}

void Server::setupRoutes() {
    CROW_ROUTE(app, "/health")
    ([]() {
        return "OK";
    });
    
    CROW_ROUTE(app, "/render")
    .methods(crow::HTTPMethod::POST)
    ([this](const crow::request& req) {
        crow::json::rvalue json_body = crow::json::load(req.body);
        
        if (!json_body) {
            return crow::response(400, "Invalid JSON body");
        }
        
        // Extract parameters
        std::string midiFilePath;
        std::string vstPath;
        float sampleRate = 44100;
        int numChannels = 2;
        int bitDepth = 16;
        
        try {
            if (json_body.has("midiFile")) midiFilePath = json_body["midiFile"].s();
            if (json_body.has("vstPath")) vstPath = json_body["vstPath"].s();
            if (json_body.has("sampleRate")) sampleRate = json_body["sampleRate"].d();
            if (json_body.has("numChannels")) numChannels = json_body["numChannels"].i();
            if (json_body.has("bitDepth")) bitDepth = json_body["bitDepth"].i();
        } catch (const std::exception& e) {
            return crow::response(400, std::string("Invalid parameters: ") + e.what());
        }
        
        // Validate required parameters
        if (midiFilePath.empty() || vstPath.empty()) {
            return crow::response(400, "Missing required parameters: midiFile and vstPath");
        }
        
        // Process the render request
        try {
            std::string outputPath = handleRenderRequest(midiFilePath, vstPath, sampleRate, numChannels, bitDepth);
            
            crow::json::wvalue result;
            result["status"] = "success";
            result["outputFile"] = outputPath;
            return crow::response(result);
        } catch (const std::exception& e) {
            return crow::response(500, std::string("Render failed: ") + e.what());
        }
    });
    
    // Add route for downloading rendered files
    CROW_ROUTE(app, "/download/<string>")
    ([](const std::string& filename) {
        // Security: Ensure the file exists and is within our output directory
        fs::path filePath = "output/" + filename;
        if (!fs::exists(filePath)) {
            return crow::response(404);
        }
        
        // Set appropriate headers for file download
        crow::response res;
        res.set_header("Content-Disposition", "attachment; filename=\"" + filename + "\"");
        res.set_header("Content-Type", "audio/wav");
        
        // Read file content
        std::ifstream file(filePath.string(), std::ios::binary);
        if (!file) {
            return crow::response(500, "Failed to open file");
        }
        
        std::string fileContent((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        res.body = fileContent;
        
        return res;
    });
}

std::string Server::handleRenderRequest(const std::string& midiFilePath, 
                                     const std::string& vstPath,
                                     float sampleRate,
                                     int numChannels,
                                     int bitDepth) {
    // Create output directory if it doesn't exist
    fs::path outputDir = "output";
    if (!fs::exists(outputDir)) {
        fs::create_directory(outputDir);
    }
    
    // Generate a unique output filename
    std::string outputFileName = fs::path(midiFilePath).stem().string() + "_" + 
                                 fs::path(vstPath).stem().string() + "_" +
                                 std::to_string(static_cast<int>(sampleRate)) + "hz.wav";
    std::string outputPath = (outputDir / outputFileName).string();
    
    // Load and process MIDI file
    if (!midiProcessor.loadMidiFile(midiFilePath)) {
        throw std::runtime_error("Failed to load MIDI file");
    }
    
    // Load VST plugin
    if (!vstRenderer.loadVst(vstPath)) {
        throw std::runtime_error("Failed to load VST plugin");
    }
    
    // Render MIDI through VST
    if (!vstRenderer.renderMidi(midiProcessor.getMidiData(), sampleRate, numChannels)) {
        throw std::runtime_error("Failed to render MIDI through VST");
    }
    
    // Write audio to file
    if (!audioWriter.writeWavFile(outputPath, vstRenderer.getAudioData(), 
                                 sampleRate, numChannels, bitDepth)) {
        throw std::runtime_error("Failed to write audio file");
    }
    
    return outputPath;
}