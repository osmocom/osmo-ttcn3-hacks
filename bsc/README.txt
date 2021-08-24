Integration Tests for OsmoBSC
-----------------------------

This test suite tests OsmoBSC while emulating both multiple BTS + MS as
well as the MSC.

In terms of external entities, you will need to run
* osmo-mgw in order to properly terminate the MGCP commands by the BSC
* osmo-stp as STP between the simulated MSC and osmo-bsc
* 3x osmo-bts-omldummy as this test suite only implements RSL and no OML

The included jenkins.sh script, together with the Dockerfiles from
http://git.osmocom.org/docker-playground/ can be used to run both the
osmo-bsc-under-test as well as the extenal entities and the tester.


Further Test Ideas
------------------

This is a random list of things about things possible to test.
Asterisks '*' are TODO, while 'x' means already implemented.

= exhaustion of resources

x send many CHAN RQD (any) and count if all channels get actually allocated
xx verify BTS_CTR_CHREQ_TOTAL reflects number sent on RSL
** verify how quickly they get released again
** verify that CHAN RQD with same RA + FN get only on allocation
x verify that we get IMM_ASS_REJ once channels are exhausted
xx verify BTS_CTR_CHREQ_NO_CHANNEL increments accordigly
* verify for particular channel type / cause values
* verify that IMM.ASS.REJ has increasing back-off
* verify how BSC reacts to AGCH overload (DELETE IND)

= paging

x page by TMSI vs. page by IMSI
x verify if CCCH_LOAD_IND(PCH) slot count is observed
x verify paging expiration
x paging with different identity (lac/cgi/...) on A interface -> expect on right BSC(s)

= assignment

x CSD call
x CTM telephony
x CIC on AoIP
x missing CodecList IE
* with no CIC and no AoIP
* with IPv6 Address in AoIP
* with LCLS information
x supported/unsupported/invalid ciphers
* supported/unsupported/invalid codecs
* inconsistent channel type + codec + codec-list
* on full BTS -> fail

= hand-over

* intra-BSC HO
* handover detect when not expected

= erroneous channel release

x no response to CHAN ACT
x CONN FAIL IND from BTS
xx verify counter increment of BTS_CTR_CHAN_RF_FAIL
* no (or late?) response to RF CHAN REL
* no (or late?) response to RLL RELEASE REQ
x RLL messages on not-activated channels

= misc

* SMS-CB
x behavior in case of CHAN ACT NACK
* EST REQ for SAPI3 originating from core
* behavior of BSC in various error cases  (ERR IND)
** BTS_CTR_CHAN_RLL_ERR increment on T200_EXPIRED
* MODE MODIFY with ACK / NACK / no response
* invalid message type / IE type
** verify BSSAP CONFUSION is sent in all applicable cases

= IPA voice related

* expect IPA CRCX, send UDP flows, expect them on AoIP
* expect RTCP 
* missing RTP flow?
* missing RTCP?
* missing / drop-outs in RTP flow[s]
* no response to IPA CRCX / MDCX / DLCX

= counters

* test each and every counter in BSC, validate via CTRL interface
xx RSL/OML failure: drop link, expect BTS_CTR_BTS_xxx_FAIL +1
xx paging
xxx send PAGING from MSC side
xxx expect BSC_CTR_PAGING_ATTEMPTED increase by one
xxx expect BSC_CTR_PAGING_EXPIRED on T3113 expiration
** BTS_CTR_CODEC_* on CHAN_ACT_ACK
* new counter ideas
** number of SCCP CR timeouts
** number of incoming RESET from MSC

= VTY based/corresponding tests

* changes in BCCH FILLING
* changes in SACCH FILLING

= dynamic TS switching

* TBD

