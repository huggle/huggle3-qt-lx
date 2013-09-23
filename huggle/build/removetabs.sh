#!/bin/bash

for file in `find . | grep -E '\.cpp$'`
do
	expand -t 4 "$file" > "$file"_temp
	mv "$file"_temp "$file"
done
