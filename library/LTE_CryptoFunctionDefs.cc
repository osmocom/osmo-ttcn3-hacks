/* Utility functions from ogslib imported to TTCN-3
 *
 * (C) 2019 Harald Welte <laforge@gnumonks.org>
 * All rights reserved.
 *
 * Released under the terms of GNU General Public License, Version 2 or
 * (at your option) any later version.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdint.h>

#include <Boolean.hh>
#include <Integer.hh>
#include <Octetstring.hh>
#include <Bitstring.hh>

#include "key_derivation.h"

namespace LTE__CryptoFunctions {

OCTETSTRING f__kdf__kasme(const OCTETSTRING& ck, const OCTETSTRING& ik, const OCTETSTRING& plmn_id,
			  const OCTETSTRING& sqn, const OCTETSTRING& ak)
{
	TTCN_Buffer ttcn_buf_ck(ck);
	TTCN_Buffer ttcn_buf_ik(ik);
	TTCN_Buffer ttcn_buf_plmn_id(plmn_id);
	TTCN_Buffer ttcn_buf_sqn(sqn);
	TTCN_Buffer ttcn_buf_ak(ak);
	uint8_t kasme[32];

	hss_auc_kasme(ttcn_buf_ck.get_data(), ttcn_buf_ik.get_data(), ttcn_buf_plmn_id.get_data(),
			ttcn_buf_sqn.get_data(), ttcn_buf_ak.get_data(), kasme);
	return OCTETSTRING(sizeof(kasme), kasme);
}

OCTETSTRING f__kdf__nas__int(const INTEGER& alg_id, const OCTETSTRING &kasme)
{
	TTCN_Buffer ttcn_buf_kasme(kasme);
	uint8_t knas[16];

	mme_kdf_nas(MME_KDF_NAS_INT_ALG, (int)alg_id, (const uint8_t*) ttcn_buf_kasme.get_data(), knas);
	return OCTETSTRING(sizeof(knas), knas);
}

OCTETSTRING f__kdf__nas__enc(const INTEGER& alg_id, const OCTETSTRING &kasme)
{
	TTCN_Buffer ttcn_buf_kasme(kasme);
	uint8_t knas[16];

	mme_kdf_nas(MME_KDF_NAS_ENC_ALG, (int)alg_id, (const uint8_t*) ttcn_buf_kasme.get_data(), knas);
	return OCTETSTRING(sizeof(knas), knas);
}


OCTETSTRING f__kdf__enb(const OCTETSTRING &kasme, const INTEGER &ul_count)
{
	TTCN_Buffer ttcn_buf_kasme(kasme);
	uint8_t kenb[32];

	mme_kdf_enb(ttcn_buf_kasme.get_data(), (int)ul_count, kenb);
	return OCTETSTRING(sizeof(kenb), kenb);
}

OCTETSTRING f__kdf__nh(const OCTETSTRING &kasme, const OCTETSTRING &sync_inp)
{
	TTCN_Buffer ttcn_buf_kasme(kasme);
	TTCN_Buffer ttcn_buf_sync_inp(sync_inp);
	uint8_t kenb[32];

	mme_kdf_nh(ttcn_buf_kasme.get_data(), ttcn_buf_sync_inp.get_data(), kenb);
	return OCTETSTRING(sizeof(kenb), kenb);
}

OCTETSTRING f__kdf__nas__token(const OCTETSTRING &kasme, const INTEGER &ul_count)
{
	TTCN_Buffer ttcn_buf_kasme(kasme);
	uint8_t nas_token[32];

	mme_kdf_nas_token(ttcn_buf_kasme.get_data(), (int)ul_count, nas_token);
	return OCTETSTRING(sizeof(nas_token), nas_token);
}


} // namespace
