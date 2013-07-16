#!/bin/sh

set -e # This will cause the shell to exit immediately if a simple command exits with a nonzero exit value.

make
make BUILD=debug
