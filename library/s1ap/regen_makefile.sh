#!/bin/sh

FILES="*.asn *.ttcn S1AP_EncDec.cc"

../../regen-makefile.sh $FILES

sed -i -e '/^CPPFLAGS/ s/$/ `pkg-config --cflags libfftranscode`/' Makefile
sed -i -e '/^LDFLAGS/ s/$/ `pkg-config --libs libfftranscode`/' Makefile
sed -i -e '/^LINUX_LIBS/ s/$/ `pkg-config --libs libfftranscode`/' Makefile
