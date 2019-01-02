#!/bin/bash

git describe --tags --exact-match &> /dev/null && git describe --tags --exact-match | sed -n '/^veins-\(.*\)*$/!{q100}' && git describe --tags --exact-match | sed 's/^veins-\(.*\)*$/\1/' || git rev-parse --short HEAD
