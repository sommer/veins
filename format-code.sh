#!/bin/bash

MINIMUM_VERSION_CF="6.0.0"
MINIMUM_VERSION_UF="0.68"

# check (minimum) versions of clang-format and uncrustify

CF=$(clang-format --version)
if [[ ($? -eq 0) && ($CF =~ ^clang-format) ]]; then
    VERSION_CF=$(echo $CF | sed -n 's/clang-format version \([0-9\.]\+\).*$/\1/p')
#    dpkg --compare-versions "$MINIMUM_VERSION_CF" "lt" "$VERSION_CF" # only works on debian-based systems
    printf "%s\n%s" "$MINIMUM_VERSION_CF" "$VERSION_CF" | sort -C -V
    if [[ $? -eq 1 ]]; then
        exit "Your version of clang-format is too old!"
    fi
else
    exit "Cannot find clang-format or version check does not work!"
fi

UF=$(uncrustify --version)
if [[ ($? -eq 0) && ($UF =~ ^Uncrustify-) ]]; then
    VERSION_UF=$(echo $UF | sed -n 's/Uncrustify-\([0-9\.]\+\).*$/\1/p')
#    dpkg --compare-versions "$MINIMUM_VERSION_UF" "lt" "$VERSION_UF" # only works on debian-based systems
    printf "%s\n%s" "$MINIMUM_VERSION_UF" "$VERSION_UF" | sort -C -V
    if [[ $? -eq 1 ]]; then
        exit "Your version of uncrustify is too old!"
    fi
else
    exit "Cannot find clang-format or version check does not work!"
fi

# actually format the code

find "$@" -name "*.cc" -o -name "*.h" | xargs clang-format -style=file -i
find "$@" -name "*.cc" -o -name "*.h" | xargs uncrustify --replace --no-backup -c .uncrustify.cfg
