#!/bin/sh

NAME=smdpp_Tests

FILES="
	*.ttcn
	*.asn
	Abstract_Socket.cc
	IPL4asp_PT.cc
	IPL4asp_discovery.cc
	Native_FunctionDefs.cc
	TCCConversion.cc
	TCCInterface.cc
	PIPEasp_PT.cc
	smdpp_Tests_Functions.cc
	rsp_client.cc
	bsp_crypto.cc
"
. ../_buildsystem/regen_makefile.inc.sh

# required for forkpty(3) used by PIPEasp
#sed -i -e '/^LINUX_LIBS/ s/$/ -lutil -lssl -lcrypto -lcurl/' Makefile

sed -i -e '/^CPPFLAGS/ s/$/ `pkg-config --cflags openssl libcurl` -Wno-deprecated -Wno-deprecated-declarations/' Makefile
sed -i -e '/^LDFLAGS/ s/$/ `pkg-config --libs openssl libcurl`/' Makefile
sed -i -e '/^LINUX_LIBS/ s/$/ `pkg-config --libs openssl libcurl`/' Makefile
