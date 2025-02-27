#!/usr/bin/env python3
"""
Python wrapper for the Midiverse CLI tool
"""

import os
import sys
import argparse
import subprocess

def main():
    parser = argparse.ArgumentParser(description='Midiverse - MIDI to Audio Renderer')
    parser.add_argument('midi_file', help='Path to MIDI file')
    parser.add_argument('vst_path', help='Path to VST plugin')
    parser.add_argument('--output', '-o', help='Output filename (default: output.wav)')
    parser.add_argument('--rate', '-r', type=int, default=44100, help='Sample rate in Hz (default: 44100)')
    parser.add_argument('--channels', '-c', type=int, default=2, help='Number of channels (default: 2)')
    parser.add_argument('--bit-depth', '-b', type=int, default=16, help='Bit depth (default: 16)')
    
    args = parser.parse_args()
    
    # Get the path to the midiverse_cli executable
    script_dir = os.path.dirname(os.path.abspath(__file__))
    
    # Check if we're in Docker (look for build directory structure)
    if os.path.exists(os.path.join(script_dir, 'build', 'midiverse_cli')):
        cli_path = os.path.join(script_dir, 'build', 'midiverse_cli')
    elif os.path.exists(os.path.join(script_dir, 'midiverse_cli')):
        # In Docker, the executable might be directly in /app/build/
        cli_path = os.path.join(script_dir, 'midiverse_cli')
    else:
        # Fallback to standard path in Docker container
        cli_path = '/app/build/midiverse_cli'
    
    # Build the command
    cmd = [cli_path, args.midi_file, args.vst_path]
    
    if args.output:
        cmd.extend(['-o', args.output])
    
    cmd.extend(['-r', str(args.rate)])
    cmd.extend(['-c', str(args.channels)])
    cmd.extend(['-b', str(args.bit_depth)])
    
    # Run the command
    print(f"Running: {' '.join(cmd)}")
    subprocess.run(cmd)

if __name__ == '__main__':
    main()