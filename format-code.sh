#!/bin/bash

find "$@" -name "*.cc" -o -name "*.h" | xargs clang-format -style=file -i
find "$@" -name "*.cc" -o -name "*.h" | xargs uncrustify --replace --no-backup -c .uncrustify.cfg
