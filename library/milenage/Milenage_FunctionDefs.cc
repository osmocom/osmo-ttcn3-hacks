/* AKA USIM Utility functions */

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdint.h>

#include <Addfunc.hh>
#include <Encdec.hh>
#include <Boolean.hh>
#include <Integer.hh>
#include <Octetstring.hh>
#include <Bitstring.hh>

#include "milenage.h"

namespace Milenage__Functions {

/* 3GPP TS 33.102 Figure 9, 3GPP TS 35.206 Annex 3 */
INTEGER f__milenage__check(const OCTETSTRING& opc, const OCTETSTRING& k,
			   const OCTETSTRING& sqn, const OCTETSTRING& _rand, const OCTETSTRING& autn,
			   OCTETSTRING& ik, OCTETSTRING& ck, OCTETSTRING& res, OCTETSTRING& auts)
{
	TTCN_Buffer ttcn_buf_opc(opc);
	TTCN_Buffer ttcn_buf_k(k);
	TTCN_Buffer ttcn_buf_sqn(sqn);
	TTCN_Buffer ttcn_buf_rand(_rand);
	TTCN_Buffer ttcn_buf_autn(autn);
	uint8_t buf_ik[16];
	uint8_t buf_ck[16];
	uint8_t buf_res[8];
	uint8_t buf_auts[14];
	size_t res_len = 0;
	int rc;

	rc = milenage_check(ttcn_buf_opc.get_data(),
			    ttcn_buf_k.get_data(),
			    ttcn_buf_sqn.get_data(),
			    ttcn_buf_rand.get_data(),
			    ttcn_buf_autn.get_data(),
			    buf_ik, buf_ck,
			    buf_res, &res_len,
			    buf_auts);

	ik = OCTETSTRING(16, static_cast<const unsigned char*>(&buf_ik[0]));
	ck = OCTETSTRING(16, static_cast<const unsigned char*>(&buf_ck[0]));
	res = OCTETSTRING(res_len, static_cast<const unsigned char*>(&buf_res[0]));
	auts = OCTETSTRING(14, static_cast<const unsigned char*>(&buf_auts[0]));

	return INTEGER(rc);
}

}
