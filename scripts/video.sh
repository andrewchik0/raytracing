#!/bin/bash

# Set parameters
input="../tmp/video%d.png"   # Input frame pattern (video0.png, video1.png, ...)
output="../render.mp4"       # Output video file
fps=30                       # Frames per second
crf=18                       # Quality (lower is better, 0-51, 18 is visually lossless)

# Check if ffmpeg is installed
if ! command -v ffmpeg &> /dev/null; then
    echo "Error: ffmpeg not found. Please install it and make sure it's in your PATH."
    exit 1
fi

# Encode video
ffmpeg -r $fps -i $input -c:v libx264 -crf $crf -pix_fmt yuv420p $output

if [ $? == 0 ]; then
    echo "Video encoding complete: $output"
else
    echo "Failed to encode video: $output"
fi
