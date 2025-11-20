#!/bin/sh -ex
wait_for_port.py -p 8080

pyhss_api_helper.py \
	add_default_apns

# Create a test subscriber with IMSI=001010000000000
pyhss_api_helper.py \
	add_subscr \
	--imsi 001010000000000 \
	--msisdn 100 \
	--auc-id 1 \
	--algo 0 \
	--ki 3c6e0b8a9c15224a8228b9a98ca1531d \
	--opc 762a2206fe0b4151ace403c86a11e479
