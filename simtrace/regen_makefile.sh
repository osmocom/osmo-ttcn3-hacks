#!/bin/sh

NAME=SIMTRACE_Tests

FILES="
	*.ttcn
	Native_FunctionDefs.cc
	USB_PT.cc
"

export CPPFLAGS_TTCN3="
"

../regen-makefile.sh -e $NAME $FILES

#sed -i -e 's/^LINUX_LIBS = -lxml2/LINUX_LIBS = -lxml2 -lusb/' Makefile
sed -i -e '/^LINUX_LIBS/ s/$/ `pkg-config --libs libusb-1.0`/' Makefile
sed -i -e '/^CPPFLAGS/ s/$/ `pkg-config --cflags libusb-1.0`/' Makefile
