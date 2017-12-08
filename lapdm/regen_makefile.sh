#!/bin/sh

FILES="*.ttcn UD_PT.cc UD_PT.hh RLCMAC_EncDec.cc"

ttcn3_makefilegen -f L1CTL_Test.ttcn $FILES
sed -i -e 's/# TTCN3_DIR = /TTCN3_DIR = \/usr/' Makefile
sed -i -e 's/LDFLAGS = /LDFLAGS = -L \/usr\/lib\/titan `pkg-config --libs libnetfilter_conntrack`/' Makefile
#sed -i -e 's/TTCN3_LIB = ttcn3-parallel/TTCN3_LIB = ttcn3/' Makefile
sed -i -e 's/CPPFLAGS = -D$(PLATFORM) -I$(TTCN3_DIR)\/include/CPPFLAGS = -D$(PLATFORM) -I$(TTCN3_DIR)\/include -I\/usr\/include\/titan/' Makefile

# for TITAN 6.3.0
sed -i -e 's/TTCN3_DIR = /TTCN3_DIR = \/usr/' Makefile
sed -i -e 's/\/bin\/compiler/\/bin\/ttcn3_compiler/' Makefile
