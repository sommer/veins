#!/bin/bash

TAG=$(git describe --tags --exact-match 2> /dev/null)
if [[ ($? -eq 0) && ($TAG =~ ^veins-) ]]; then
    echo $TAG | sed -n 's/^veins-\(.*\)$/\1/p'

else
    git rev-parse --short HEAD
fi
