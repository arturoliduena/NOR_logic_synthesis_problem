#!/bin/bash

# Function to perform cleanup
cleanup() {
    echo "Cleaning up..."
    # Add any cleanup actions here if needed
    exit 1
}

# Trap SIGINT signal (Ctrl+C)
trap cleanup SIGINT

for instance in ../instances/nlsp_*.inp; do
    output_file="../out/$(basename "$instance" .inp).out"
    gtimeout 60s ./nlsp < "$instance" > "$output_file"
    if [ ! -s "$output_file" ]; then
        rm "$output_file"
    fi
    echo "................"
done
