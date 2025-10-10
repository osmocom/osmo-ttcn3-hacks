#!/bin/sh -ex

wait_for_port.py -p 8080

pyhss_api_helper.py add_default_apn
