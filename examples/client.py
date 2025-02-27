#!/usr/bin/env python3
"""
Example client for the Midiverse service.
This script demonstrates how to render a MIDI file through a VST plugin.
"""

import argparse
import json
import os
import requests
import sys

def render_midi(url, midi_file, vst_path, sample_rate=44100, 
               num_channels=2, bit_depth=16):
    """
    Request the Midiverse service to render a MIDI file using a VST plugin.
    
    Args:
        url: Base URL of the Midiverse service
        midi_file: Path to MIDI file
        vst_path: Path to VST plugin
        sample_rate: Audio sample rate (default: 44100)
        num_channels: Number of audio channels (default: 2)
        bit_depth: Audio bit depth (default: 16)
        
    Returns:
        Path to the rendered audio file
    """
    # Check if files exist
    if not os.path.exists(midi_file):
        raise FileNotFoundError(f"MIDI file not found: {midi_file}")
    
    if not os.path.exists(vst_path):
        raise FileNotFoundError(f"VST plugin not found: {vst_path}")
    
    # Build request
    endpoint = f"{url}/render"
    payload = {
        "midiFile": os.path.abspath(midi_file),
        "vstPath": os.path.abspath(vst_path),
        "sampleRate": sample_rate,
        "numChannels": num_channels,
        "bitDepth": bit_depth
    }
    
    # Send request
    try:
        print(f"Sending render request to {endpoint}...")
        response = requests.post(endpoint, json=payload)
        response.raise_for_status()
    except requests.exceptions.RequestException as e:
        print(f"Error: {e}")
        if hasattr(e, 'response') and e.response is not None:
            print(f"Server response: {e.response.text}")
        sys.exit(1)
    
    # Parse response
    data = response.json()
    if data.get("status") != "success":
        print(f"Error: Render failed - {data.get('error', 'Unknown error')}")
        sys.exit(1)
    
    output_file = data.get("outputFile")
    print(f"Render successful! Output file: {output_file}")
    
    return output_file

def download_file(url, output_file, filename):
    """
    Download the rendered audio file.
    
    Args:
        url: Base URL of the Midiverse service
        output_file: Server path of the rendered file
        filename: Local filename to save to
    """
    # Extract just the filename from the output path
    server_filename = os.path.basename(output_file)
    endpoint = f"{url}/download/{server_filename}"
    
    try:
        print(f"Downloading file from {endpoint}...")
        response = requests.get(endpoint, stream=True)
        response.raise_for_status()
        
        total_size = int(response.headers.get('content-length', 0))
        block_size = 8192
        downloaded = 0
        
        with open(filename, 'wb') as f:
            for chunk in response.iter_content(chunk_size=block_size):
                if chunk:
                    f.write(chunk)
                    downloaded += len(chunk)
                    
                    # Show progress
                    if total_size > 0:
                        done = int(50 * downloaded / total_size)
                        sys.stdout.write("\r[%s%s] %d%%" % 
                                        ('=' * done, ' ' * (50-done), 
                                        int(100 * downloaded / total_size)))
                        sys.stdout.flush()
        
        print(f"\nDownload complete! Saved to {filename}")
    except requests.exceptions.RequestException as e:
        print(f"Error downloading file: {e}")
        sys.exit(1)

def main():
    parser = argparse.ArgumentParser(description='Render MIDI files through VST plugins')
    parser.add_argument('midi_file', help='Path to MIDI file')
    parser.add_argument('vst_path', help='Path to VST plugin')
    parser.add_argument('--url', default='http://localhost:8080', 
                       help='Midiverse service URL (default: http://localhost:8080)')
    parser.add_argument('--sample-rate', type=int, default=44100, 
                       help='Audio sample rate (default: 44100)')
    parser.add_argument('--channels', type=int, default=2, 
                       help='Number of audio channels (default: 2)')
    parser.add_argument('--bit-depth', type=int, default=16, 
                       help='Audio bit depth (default: 16)')
    parser.add_argument('--output', '-o', 
                       help='Output filename (default: based on MIDI filename)')
    
    args = parser.parse_args()
    
    # Default output filename
    if not args.output:
        midi_basename = os.path.splitext(os.path.basename(args.midi_file))[0]
        args.output = f"{midi_basename}_rendered.wav"
    
    # Check service availability
    try:
        print(f"Checking health at {args.url}/health...")
        health_response = requests.get(f"{args.url}/health", timeout=5)
        print(f"Health response status: {health_response.status_code}")
        print(f"Health response content: {health_response.text}")
        
        if health_response.status_code != 200 or health_response.text != "OK":
            print(f"Warning: Service health check failed: {health_response.status_code} {health_response.text}")
            choice = input("Continue anyway? (y/n): ")
            if choice.lower() != 'y':
                sys.exit(1)
    except requests.exceptions.RequestException as e:
        print(f"Error: Could not connect to Midiverse service at {args.url}")
        print(f"Error details: {e}")
        print("\nPossible solutions:")
        print("1. Make sure the server is running (./midiverse)")
        print("2. Check if another application is using port 8080")
        print("3. Try using a different port: --url http://localhost:8081 and start server with ./midiverse 8081")
        sys.exit(1)
    
    # Render MIDI to audio
    output_file = render_midi(
        args.url, 
        args.midi_file, 
        args.vst_path, 
        args.sample_rate, 
        args.channels, 
        args.bit_depth
    )
    
    # Download the rendered audio
    download_file(args.url, output_file, args.output)
    
    print(f"Successfully rendered {args.midi_file} to {args.output}")

if __name__ == "__main__":
    main()