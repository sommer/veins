# This file contains some bash functions that can be 
# used in scripts.

# Preprocess the scenario
function preprocess_scenario {
    sed '
s/#.*//g; /./!d
' $1 > $2
}
