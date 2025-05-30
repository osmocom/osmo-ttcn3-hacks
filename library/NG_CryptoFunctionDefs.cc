/* AKA USIM Utility functions */

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdint.h>
#include <gnutls/crypto.h>

#include <Addfunc.hh>
#include <Encdec.hh>
#include <Boolean.hh>
#include <Integer.hh>
#include <Octetstring.hh>
#include <Bitstring.hh>

#include "key_derivation.h"

namespace NG__CryptoFunctions {

/* 3GPP TS 33.501 A.4 RES* and XRES* derivation function */
OCTETSTRING f__kdf__xres__star(const OCTETSTRING &ssn, const OCTETSTRING& ck, const OCTETSTRING& ik,
			       const OCTETSTRING& rand, const OCTETSTRING& xres)
{
	TTCN_Buffer ttcn_buf_ssn(ssn);
	TTCN_Buffer ttcn_buf_ck(ck);
	TTCN_Buffer ttcn_buf_ik(ik);
	TTCN_Buffer ttcn_buf_rand(rand);
	TTCN_Buffer ttcn_buf_xres(xres);
	uint8_t xres_star[16];

	kdf_xres_star(ttcn_buf_ssn.get_data(), ttcn_buf_ssn.get_len(),
		      ttcn_buf_ck.get_data(),
		      ttcn_buf_ik.get_data(),
		      ttcn_buf_rand.get_data(),
		      ttcn_buf_xres.get_data(), ttcn_buf_xres.get_len(),
		      xres_star);
	return OCTETSTRING(sizeof(xres_star), xres_star);
}

}
