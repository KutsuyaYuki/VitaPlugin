#!/bin/bash

> code.txt  # Clear code.txt if it exists

for file in src/*; do
    if [ -f "$file" ]; then
        echo "$(basename "</q>$file<q>"):" >> code.txt
        echo '~~~' >> code.txt
        cat "$file" >> code.txt
        echo '~~~' >> code.txt
        echo >> code.txt
    fi
done
