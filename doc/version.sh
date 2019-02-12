#!/bin/bash

TAG=$(git describe --tags --always 2> /dev/null)
if [[ ($? -eq 0) && ($TAG =~ ^veins-) ]]; then
    echo $TAG | sed -n 's/^veins-\(.*\)$/\1/p'
else
    echo ""
fi
