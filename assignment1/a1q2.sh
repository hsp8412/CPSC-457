#!/bin/bash

# count total number of files and directories
total_count=$(($(ls -l | wc -l) -1))
echo "Total number of files and directories: $total_count"

echo "-----"

# count total number of files
files_count=$(ls -l | grep -c "^-")
echo "Files: $files_count"

# display files and permissions
files_list=$(ls -l | grep "^-" | awk '{print $1, $9}')
echo "$files_list"

echo "-----"

# count total number of directories
dirs_count=$(ls -l | grep -c "^d")
echo "Directories: $dirs_count"

# display directories and permissions
dirs_list=$(ls -l | grep "^d" | awk '{print $1, $9}')
echo "$dirs_list"









