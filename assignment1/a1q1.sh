#! /bin/bash

# ask user to input a number
read -p "Please enter a number: " number

# use regular expression and "=~" to check if the input is a number
# valid input: positive and negative integers, possibly with leading zeros
re='^-?[0-9]+$'
if ! [[ "$number" =~ $re ]]
then
    echo "error: $number is not a valid integer"
    exit 1
fi

# remove leading zeros
# check if the number starts with a "-"
if [[ "$number" == -* ]]
then
    # for negative number
    # ${number#-}: remove - from the beginning
    # $((10#${number#-})): interpret the number as a base-10 number (avoid default octal interpretation)
    number=-$((10#${number#-}))
else
    # for positive number
    number=$((10#$number))
fi

# handle the special case of "-0" input
# set the number to 0
if [ "$number" == -0 ]
then
    number=0
fi

# calculate the remainder: number % 2
remainder=$((number % 2))

# check if remainder is 0
if [ "$remainder" -eq 0 ]
then echo "$number is an even number"
else echo "$number is an odd number"
fi