#!/bin/bash

for x in $(cat list)
do
	cp ../tasksets/$x ./
done
