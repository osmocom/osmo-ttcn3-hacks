#!/bin/sh
find . -type l -not -path "./bin/*" -exec rm \{\} \;
