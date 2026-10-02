#!/bin/sh
# Build the original diagnostic disassembler with a Python 3 generator.
# Usage: sh bin/build-udis86.sh /absolute/install/prefix
set -eu
prefix=${1:?usage: build-udis86.sh /absolute/install/prefix}
case "$prefix" in /*) ;; *) echo 'prefix must be absolute' >&2; exit 1;; esac
work=$(mktemp -d)
trap 'rm -rf "$work"' EXIT HUP INT TERM
git clone https://github.com/vmt/udis86.git "$work/udis86"
cd "$work/udis86"
git checkout 56ff6c87c11de0ffa725b14339004820556e343d
# The generator's table indices require integer division under Python 3.
sed -i 's@ / @ // @g' scripts/ud_opcode.py
autoreconf -fi
./configure --prefix="$prefix" --without-docs --with-python=python3
make
make install
