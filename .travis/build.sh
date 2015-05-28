#!/bin/bash -x
# Make sure we exit if there is a failure
set -e

make clean
make
RETURN="$?"


if [ "${RETURN}" != "0" ]; then
    echo "Building iop failed!"
    exit 1
fi




