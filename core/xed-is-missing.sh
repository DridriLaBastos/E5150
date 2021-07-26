#!/bin/sh

echo "int main (void) {}" | gcc -x c -lxed -
echo $?