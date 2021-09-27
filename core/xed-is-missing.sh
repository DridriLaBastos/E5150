#!/bin/sh

# This script checks if intel xed is installed locally and accessible, if not, it will be compiled
echo "int main (void) {}" | gcc -x c -lxed -
echo $?