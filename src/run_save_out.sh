#!/bin/bash

# Function to perform cleanup
cleanup() {
    echo "Cleaning up..."
    # Add any cleanup actions here if needed
    exit 1
}

# Trap SIGINT signal (Ctrl+C)
trap cleanup SIGINT

# Initialize counters
num=0
error_num=0

# Create output directory if it does not exist
mkdir -p ../out

for instance in ../instances/nlsp_*.inp; do
    ((num++))
    echo "Running instance $instance"
    echo "Instance processing number $num"
    output_file="../out/$(basename "$instance" .inp).out"
    gtimeout 60s ./nlsp < "$instance" > "$output_file"
    if [ ! -s "$output_file" ]; then
        ((error_num++))
        echo "Error processing instance $instance (Error $error_num)"
        rm "$output_file"
    fi
    echo "................"
done
