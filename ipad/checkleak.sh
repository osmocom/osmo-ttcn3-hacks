#!/bin/sh

grep "====> free" onomondo-ipa.log | tail -n 1 | grep " 0 bytes total"
