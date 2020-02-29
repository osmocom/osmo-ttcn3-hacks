#!/bin/sh

FILES="*.ttcn USB_PT.cc Native_FunctionDefs.cc "

../regen-makefile.sh SIMTRACE_Tests.ttcn $FILES

#sed -i -e 's/^LINUX_LIBS = -lxml2/LINUX_LIBS = -lxml2 -lusb/' Makefile
sed -i -e '/^LINUX_LIBS/ s/$/ `pkg-config --libs libusb-1.0`/' Makefile
sed -i -e '/^CPPFLAGS/ s/$/ `pkg-config --cflags libusb-1.0`/' Makefile
