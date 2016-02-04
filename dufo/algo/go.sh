#!/bin/bash

for tsname in $(ls ./tasksets/)
do
	echo $tsname
	./demo ./tasksets/$tsname 
done
