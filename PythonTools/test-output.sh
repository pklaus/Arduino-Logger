#!/bin/bash

# This script appends lines with values to a text file

FILE=./values.txt

while true; do 
	#DATE=`date +%Y%m%d`
	#HOUR=`date +%H`
  echo "$RANDOM $RANDOM $RANDOM $RANDOM" >> $FILE
	sleep 1
done

