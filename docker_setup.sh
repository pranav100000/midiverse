#!/bin/bash
set -e

# Create directories
mkdir -p input output

# Copy test MIDI generator to input
cp examples/generate_test_midi.py input/

# Create a dummy VST file for testing
touch input/dummy.vst

echo "Setup complete!"
echo ""
echo "To build the Docker image:"
echo "  docker compose build"
echo ""
echo "To generate test MIDI files:"
echo "  docker compose run --rm midiverse python3 /app/input/generate_test_midi.py"
echo ""
echo "To render a MIDI file:"
echo "  docker compose run --rm midiverse /app/input/test_scale.mid /app/input/dummy.vst -o /app/output/result.wav"
echo ""
echo "Generated audio files will be in the ./output directory"