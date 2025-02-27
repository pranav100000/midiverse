#!/usr/bin/env python3
"""
Generate a simple test MIDI file with a C major scale.
This is useful for testing the Midiverse service without needing to have a MIDI file handy.
"""

import os
import argparse
from mido import Message, MidiFile, MidiTrack

def generate_scale(output_file='test_scale.mid', tempo=120):
    """
    Generate a MIDI file with a simple C major scale.
    
    Args:
        output_file: Path to save the generated MIDI file
        tempo: Tempo in beats per minute
    """
    mid = MidiFile()
    track = MidiTrack()
    mid.tracks.append(track)
    
    # Add tempo event (microseconds per quarter note)
    microseconds_per_beat = int(60000000 / tempo)
    track.append(Message('control_change', control=81, value=tempo, time=0))
    
    # Set instrument to piano
    track.append(Message('program_change', program=0, time=0))
    
    # Set volume
    track.append(Message('control_change', control=7, value=100, time=0))
    
    # C major scale (C4 to C5)
    notes = [60, 62, 64, 65, 67, 69, 71, 72]
    
    # Add note on/off events
    for note in notes:
        track.append(Message('note_on', note=note, velocity=64, time=480))
        track.append(Message('note_off', note=note, velocity=64, time=480))
    
    # Add a final note
    track.append(Message('note_on', note=60, velocity=64, time=480))
    track.append(Message('note_off', note=60, velocity=64, time=960))
    
    # Save the MIDI file
    mid.save(output_file)
    print(f"Generated MIDI file: {os.path.abspath(output_file)}")

def generate_chord_progression(output_file='test_chords.mid', tempo=100):
    """
    Generate a MIDI file with a simple chord progression (C-G-Am-F).
    
    Args:
        output_file: Path to save the generated MIDI file
        tempo: Tempo in beats per minute
    """
    mid = MidiFile()
    track = MidiTrack()
    mid.tracks.append(track)
    
    # Add tempo event
    microseconds_per_beat = int(60000000 / tempo)
    track.append(Message('control_change', control=81, value=tempo, time=0))
    
    # Set instrument to piano
    track.append(Message('program_change', program=0, time=0))
    
    # Set volume
    track.append(Message('control_change', control=7, value=100, time=0))
    
    # Chord progression: C-G-Am-F
    chords = [
        [60, 64, 67],  # C major
        [55, 59, 62],  # G major
        [57, 60, 64],  # A minor
        [53, 57, 60]   # F major
    ]
    
    # Play each chord
    for chord in chords:
        # Note on for all notes in chord
        for i, note in enumerate(chord):
            track.append(Message('note_on', note=note, velocity=64, time=0 if i > 0 else 0))
        
        # Note off for all notes (after 1 beat)
        for i, note in enumerate(chord):
            track.append(Message('note_off', note=note, velocity=64, time=0 if i > 0 else 960))
    
    # Save the MIDI file
    mid.save(output_file)
    print(f"Generated MIDI file: {os.path.abspath(output_file)}")

def main():
    parser = argparse.ArgumentParser(description='Generate test MIDI files')
    parser.add_argument('--scale', action='store_true', help='Generate a C major scale')
    parser.add_argument('--chords', action='store_true', help='Generate a chord progression')
    parser.add_argument('--output', '-o', help='Output filename')
    parser.add_argument('--tempo', type=int, default=120, help='Tempo in BPM')
    
    args = parser.parse_args()
    
    # Default to generating both if none specified
    if not (args.scale or args.chords):
        args.scale = True
        args.chords = True
    
    if args.scale:
        scale_output = args.output if args.output and not args.chords else 'test_scale.mid'
        generate_scale(scale_output, args.tempo)
    
    if args.chords:
        chord_output = args.output if args.output and not args.scale else 'test_chords.mid'
        generate_chord_progression(chord_output, args.tempo)
    
    print("\nTo use these MIDI files with the Midiverse service:")
    print("python client.py test_scale.mid /path/to/your/vst.plugin")

if __name__ == "__main__":
    main()