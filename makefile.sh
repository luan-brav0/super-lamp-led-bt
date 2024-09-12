#!/bin/bash

SKETCH_PATH="/path/to/your/sketch"
BOARD="arduino:avr:uno"
PORT="/dev/ttyUSB0"

# Compile the sketch
arduino-cli compile --fqbn $BOARD $SKETCH_PATH

# Upload the sketch
arduino-cli upload -p $PORT --fqbn $BOARD $SKETCH_PATH

