#!/bin/sh

FILES="*.ttcn SCCP_EncDec.cc  SCTPasp_PT.cc  TCCConversion.cc TCCInterface.cc"

ttcn3_makefilegen -l -f MSC_Tests.ttcn $FILES
sed -i -e 's/# TTCN3_DIR = /TTCN3_DIR = \/usr/' Makefile
sed -i -e 's/LDFLAGS = /LDFLAGS = -L \/usr\/lib\/titan /' Makefile
#sed -i -e 's/TTCN3_LIB = ttcn3-parallel/TTCN3_LIB = ttcn3/' Makefile
sed -i -e 's/CPPFLAGS = -D$(PLATFORM) -I$(TTCN3_DIR)\/include/CPPFLAGS = -D$(PLATFORM) -I$(TTCN3_DIR)\/include -I\/usr\/include\/titan/' Makefile

# for TITAN 6.3.0
sed -i -e 's/TTCN3_DIR = $/TTCN3_DIR = \/usr/' Makefile
sed -i -e 's/\/bin\/compiler/\/bin\/ttcn3_compiler/' Makefile
