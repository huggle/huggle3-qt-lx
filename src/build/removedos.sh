#!/bin/bash

for file in `find . | grep -E '\.cpp$|\.h$'`
do
	dos2unix "$file" 
done
