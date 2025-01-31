#!/bin/bash

# Check if the correct number of arguments is provided
if [ "$#" -ne 2 ]; then
    echo "Error: Two arguments required - <filesdir> and <searchstr>"
    exit 1
fi

# Assign arguments to variables
filesdir="$1"
searchstr="$2"

# Check if the provided directory exists and is a directory
if [ ! -d "$filesdir" ]; then
    echo "Error: $filesdir is not a valid directory"
    exit 1
fi

# Count the number of files in the directory and subdirectories
file_count=$(find "$filesdir" -type f | wc -l)

# Count the number of matching lines
matching_lines=$(grep -r "$searchstr" "$filesdir" 2>/dev/null | wc -l)

# Print the results
echo "The number of files are $file_count and the number of matching lines are $matching_lines"

