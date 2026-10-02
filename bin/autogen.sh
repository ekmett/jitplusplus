#!/bin/sh
set -eu
autoreconf -fi
./configure "$@"
make
make check
