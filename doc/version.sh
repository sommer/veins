#!/bin/bash

# Generates the version number used for the doxygen documentation
#
# Uses git describe to analyze the current HEAD of the repository.
# Due to --tags, it tries to refer to the latest tag in the history and
# describes the difference in commits to this tag.
# Due to --aways, the command produces an output even if no tag exits.
#
# Example: veins-5a1-16-gd81b4a14a0
# The current HEAD points at the commit d81b4a14a0, which is 16 commits
# newer than the tag veins-5a1 it is based on.
#
# In case git describe fails, the version is empty.

TAG=$(git describe --tags --always 2> /dev/null)
if [[ ($? -eq 0) && ($TAG =~ ^veins-) ]]; then
    echo $TAG | sed -n 's/^veins-\(.*\)$/\1/p'
else
    echo ""
fi
