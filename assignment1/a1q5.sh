#!/bin/bash

# get an input number from user
read -p "Please enter a number: " num

# input validattion
# input allowed: 0 or positive integer, possibly with leading zeros
re='^[0-9]+$'
if ! [[ "$num" =~ $re ]]
then
    echo "error: input should be 0 or positive integer"
    exit 1
fi

# remove leading zeros
num=$((10#$num))

# determine the number of digits in the number
numOfDigits=${#num}

# copy the input number to preserve it
tmp=$num

# initialize sum
sum=0

# loop through each digit and sum up the power of number of digits
while ! [ "$tmp" -eq 0 ]; do
    # get the digit from the least significant position by % 10
    digit=$((tmp % 10))
    # calculate the digit to the power of the number of digits using basic calculator
    power=$(echo "$digit ^ $numOfDigits" | bc)
    # add the power value to the sum
    sum=$((sum + power))
    # remove the digit on the least significant position by / 10 for the next round
    tmp=$((tmp / 10))
done

# determine whether the number is an Armstrong number by checking if the sum is equal to the number itself
if [[ "$sum" == "$num" ]]
then echo "$num is an Armstrong number"
else echo "$num is not an Armstrong number"
fi