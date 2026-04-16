#!/usr/bin/bash

# Use this when you wish to view your server's steady-state.

set -euo pipefail

EXEC="bin/proj2-server"
PARSER="lib/util/server-parser"

if [[ $# -ne 3 ]]; then
    echo "Usage: $0 <hostname> <readers count> <solvers count>" >&2
    exit 1
fi

DATAGRAMHOST="$1"
READERS="$2"
SOLVER="$3"

stdbuf -oL -eL "$EXEC" "$DATAGRAMHOST" "$READERS" "$SOLVER" 2>&1 | "$PARSER"
