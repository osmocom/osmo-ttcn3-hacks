Integration Tests for OsmoBSC which we can perform from TTCN-3

= exhaustion of resources

* send many CHAN RQD (any) and count if all channels get actually allocated
** verify BTS_CTR_CHREQ_TOTAL reflects number sent on RSL
** verify how quickly they get released again
** verify that CHAN RQD with same RA + FN get only on allocation
* verify that we get IMM_ASS_REJ once channels are exhausted
** verify BTS_CTR_CHREQ_NO_CHANNEL increments accordigly
* verify for particular channel type / cause values
* verify that IMM.ASS.REJ has increasing back-off
* verify how BSC reacts to AGCH overload (DELETE IND)


= paging

* page by TMSI vs. page by IMSI
* verify if CCCH_LOAD_IND(PCH) slot count is observed
* verify paging expiration
* paging with different identity (lac/cgi/...) on A interface -> expect on right BSC(s)

= hand-over

* intra-BSC HO
* handover detect when not expected

= erroneous channel release

* no response to CHAN ACT
* CONN FAIL IND from BTS
** verify counter increment of BTS_CTR_CHAN_RF_FAIL
* no (or late?) response to RF CHAN REL
* no (or late?) response to RLL RELEASE REQ
* RLL messages on not-activated channels

= misc

* SMS-CB
* behavior in case of CHAN ACT NACK
* EST REQ for SAPI3 originating from core
* behavior of BSC in various error cases  (ERR IND)
** BTS_CTR_CHAN_RLL_ERR increment on T200_EXPIRED
* MODE MODIFY with ACK / NACK / no response

= IPA voice related

* expect IPA CRCX, send UDP flows, expect them on AoIP
* expect RTCP 
* missing RTP flow?
* missing RTCP?
* missing / drop-outs in RTP flow[s]
* no response to IPA CRCX / MDCX / DLCX

= counters

* test each and every counter in BSC, validate via CTRL interface
** RSL/OML failure: drop link, expect BTS_CTR_BTS_xxx_FAIL +1
** paging
*** send PAGING from MSC side
*** expect BSC_CTR_PAGING_ATTEMPTED increase by one
*** expect BSC_CTR_PAGING_EXPIRED on T3113 expiration
** BTS_CTR_CODEC_* on CHAN_ACT_ACK
* new counter ideas
** number of SCCP CR timeouts
** number of incoming RESET from MSC

= VTY based/corresponding tests

* changes in BCCH FILLING
* changes in SACCH FILLING

