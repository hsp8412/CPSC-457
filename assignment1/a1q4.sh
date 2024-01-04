#!/bin/bash

# # list all files(for testing)
# files=$(find ./ -maxdepth 4 -type f -exec du -ch {} +)
# echo "$files"
# echo "-----------------------------"

# -maxdepth 4: only check 3 levels of subdir
# -type f: only find files
# -exec ... {} +: for each of the file found, execute an extra command
# du -ch: calculate disk usage, count total and display in human readable form
# grep total$: get the line that ends with "total"
# awk: get the value from the first column of the line(the value of size)
size=$(find ./ -maxdepth 4 -type f -exec du -ch {} + | grep total$ | awk '{print $1}')

echo "The total size: $size"