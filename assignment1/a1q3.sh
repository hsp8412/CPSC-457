#!/bin/bash

# get the input from user
read -p "Please enter n, the number of input numbers: " numOfNums

# input validation
# input allowed: positive integer, possibly with leading zeros
numOfNumsRe='^[0-9]+$'
if ! [[ "$numOfNums" =~ $numOfNumsRe ]]
then
    echo "error: input should be an integer greater than 3"
    exit 1
fi

# remove leading zeros
numOfNums=$((10#$numOfNums))

# input validation
# check if the input is larger than 3
if [ "$numOfNums" -le 3 ]
then echo "error: input should be an integer greater than 3"
    exit 1
fi

# initialize sum
sum=0

# regular expression for input number validation
numRe="^-?[0-9]+(\.[0-9]+)?$"

# loop numOfNums times to get input numbers
for((i=1; i<=numOfNums; i++))
do
    # get a number from user
    read -p "Enter number #$i: " num
    
    # validate the input number
    if ! [[ "$num" =~ $numRe ]]
    then
        echo "error: $num is not a valid number"
        exit 1
    fi
    
    # add number to the sum
    sum=$(echo "$sum + $num" | bc)
done

# print result
echo "The sum is: $sum"