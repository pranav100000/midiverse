#!/usr/bin/env python3
"""
Python server for Midiverse

This server provides a simple HTTP API for the Midiverse library.
It's an alternative to the C++ server using Crow.
"""

import os
import sys
import json
import argparse
import subprocess
from flask import Flask, request, jsonify, send_file

app = Flask(__name__)

# Path to the midiverse executable
MIDIVERSE_PATH = os.path.join(os.path.dirname(os.path.dirname(os.path.abspath(__file__))), 
                             "build", "midiverse")

# Output directory
OUTPUT_DIR = os.path.join(os.path.dirname(os.path.dirname(os.path.abspath(__file__))), "output")

@app.route('/health')
def health():
    return "OK"

@app.route('/render', methods=['POST'])
def render():
    # Get JSON data
    data = request.json
    if not data:
        return jsonify({"status": "error", "message": "Invalid JSON data"}), 400
    
    # Extract parameters
    midi_file = data.get('midiFile')
    vst_path = data.get('vstPath')
    sample_rate = data.get('sampleRate', 44100)
    num_channels = data.get('numChannels', 2)
    bit_depth = data.get('bitDepth', 16)
    
    # Validate parameters
    if not midi_file or not vst_path:
        return jsonify({"status": "error", "message": "Missing required parameters: midiFile and vstPath"}), 400
    
    if not os.path.exists(midi_file):
        return jsonify({"status": "error", "message": f"MIDI file not found: {midi_file}"}), 404
    
    if not os.path.exists(vst_path):
        return jsonify({"status": "error", "message": f"VST plugin not found: {vst_path}"}), 404
    
    # Call midiverse to render audio
    try:
        # Make sure OUTPUT_DIR exists
        os.makedirs(OUTPUT_DIR, exist_ok=True)
        
        # Build the command
        midi_name = os.path.splitext(os.path.basename(midi_file))[0]
        vst_name = os.path.splitext(os.path.basename(vst_path))[0]
        output_file = f"{midi_name}_{vst_name}_{sample_rate}hz.wav"
        output_path = os.path.join(OUTPUT_DIR, output_file)
        
        # Use direct rendering
        # In a real implementation, we'd use ctypes to call the C++ library directly
        # For now, let's do a dummy implementation
        print(f"Rendering MIDI file: {midi_file}")
        print(f"Using VST plugin: {vst_path}")
        print(f"Output file: {output_path}")
        
        # Generate a sample audio file (this is just for testing)
        # In a real implementation, we'd call the C++ library
        with open(output_path, 'wb') as f:
            # Create a simple WAV header
            # RIFF header
            f.write(b'RIFF')
            f.write((36).to_bytes(4, byteorder='little'))  # File size minus 8
            f.write(b'WAVE')
            
            # Format chunk
            f.write(b'fmt ')
            f.write((16).to_bytes(4, byteorder='little'))  # Chunk size
            f.write((1).to_bytes(2, byteorder='little'))   # PCM format
            f.write((num_channels).to_bytes(2, byteorder='little'))  # Channels
            f.write((sample_rate).to_bytes(4, byteorder='little'))  # Sample rate
            bytes_per_sample = bit_depth // 8
            byte_rate = sample_rate * num_channels * bytes_per_sample
            f.write((byte_rate).to_bytes(4, byteorder='little'))  # Byte rate
            block_align = num_channels * bytes_per_sample
            f.write((block_align).to_bytes(2, byteorder='little'))  # Block align
            f.write((bit_depth).to_bytes(2, byteorder='little'))  # Bits per sample
            
            # Data chunk
            f.write(b'data')
            f.write((0).to_bytes(4, byteorder='little'))  # Data size (0 for now)
            
        return jsonify({
            "status": "success", 
            "outputFile": output_path
        })
        
    except Exception as e:
        return jsonify({"status": "error", "message": str(e)}), 500

@app.route('/download/<path:filename>')
def download(filename):
    filepath = os.path.join(OUTPUT_DIR, os.path.basename(filename))
    if not os.path.exists(filepath):
        return jsonify({"status": "error", "message": f"File not found: {filename}"}), 404
    
    return send_file(filepath, 
                    mimetype='audio/wav',
                    as_attachment=True,
                    download_name=os.path.basename(filename))

def main():
    parser = argparse.ArgumentParser(description='Midiverse Python Server')
    parser.add_argument('--port', type=int, default=8080, help='Port to listen on (default: 8080)')
    parser.add_argument('--host', default='localhost', help='Host to bind to (default: localhost)')
    args = parser.parse_args()
    
    print(f"Starting Midiverse Python Server on http://{args.host}:{args.port}")
    print(f"Press Ctrl+C to stop the server")
    
    # Run the Flask app
    app.run(host=args.host, port=args.port, debug=True)

if __name__ == '__main__':
    main()