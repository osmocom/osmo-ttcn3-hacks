#!/bin/sh
find . -type l -not -path "./bin/*" -not -path "./M3UA_CNL113537/*" -not -path "./SCCP_CNL113341/*" -exec rm \{\} \;
