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

/* 3GPP TS 33.501 Annex A.2 */
OCTETSTRING f__kdf__kausf(const OCTETSTRING& ck, const OCTETSTRING& ik, const OCTETSTRING &ssn,
			  const OCTETSTRING& autn)
{
	TTCN_Buffer ttcn_buf_ck(ck);
	TTCN_Buffer ttcn_buf_ik(ik);
	TTCN_Buffer ttcn_buf_ssn(ssn);
	TTCN_Buffer ttcn_buf_autn(autn);
	uint8_t kausf[32];

	kdf_kausf(ttcn_buf_ck.get_data(), ttcn_buf_ik.get_data(),
		  ttcn_buf_ssn.get_data(), ttcn_buf_ssn.get_len(),
		  ttcn_buf_autn.get_data(), kausf);
	return OCTETSTRING(sizeof(kausf), kausf);
}

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

/* 3GPP TS 33.501 Annex A.6 */
OCTETSTRING f__kdf__kseaf(const OCTETSTRING& kausf, const OCTETSTRING &ssn)
{
	TTCN_Buffer ttcn_buf_kausf(kausf);
	TTCN_Buffer ttcn_buf_ssn(ssn);
	uint8_t kseaf[32];

	kdf_kseaf(ttcn_buf_kausf.get_data(),
		  ttcn_buf_ssn.get_data(), ttcn_buf_ssn.get_len(), kseaf);
	return OCTETSTRING(sizeof(kseaf), kseaf);
}

/* 3GPP TS 33.501 Annex A.7 */
OCTETSTRING f__kdf__kamf(const OCTETSTRING& kseaf, const OCTETSTRING &supi, const OCTETSTRING &abba)
{
	TTCN_Buffer ttcn_buf_kseaf(kseaf);
	TTCN_Buffer ttcn_buf_supi(supi);
	TTCN_Buffer ttcn_buf_abba(abba);
	uint8_t kamf[32];

	kdf_kamf(ttcn_buf_kseaf.get_data(),
		  ttcn_buf_supi.get_data(), ttcn_buf_supi.get_len(),
		  ttcn_buf_abba.get_data(), kamf);
	return OCTETSTRING(sizeof(kamf), kamf);
}

/* 3GPP TS 33.501 Annex A.8 */
OCTETSTRING f__kdf__ng__nas__algo(const OCTETSTRING& kamf, const OCTETSTRING &algo_type, const OCTETSTRING &algo_id)
{
	TTCN_Buffer ttcn_buf_kamf(kamf);
	TTCN_Buffer ttcn_buf_algo_type(algo_type);
	TTCN_Buffer ttcn_buf_algo_id(algo_id);
	uint8_t out[32];

	kdf_ng_nas_algo(ttcn_buf_kamf.get_data(),
			algo_type[0].get_octet(),
			algo_id[0].get_octet(), out);
	return OCTETSTRING(sizeof(out), out);
}

}
