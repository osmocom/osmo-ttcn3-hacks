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

#include "snow-3g.h"

//#define DEBUG

#ifdef DEBUG
static __thread char hexd_buff[4096];
static const char hex_chars[] = "0123456789abcdef";

static const char *_osmo_hexdump_buf(char *out_buf, size_t out_buf_size, const unsigned char *buf, int len, const char *delim,
			     bool delim_after_last)
{
	int i;
	char *cur = out_buf;
	size_t delim_len;

	if (!out_buf || !out_buf_size)
		return "";

	delim = delim ? : "";
	delim_len = strlen(delim);

	for (i = 0; i < len; i++) {
		const char *delimp = delim;
		int len_remain = out_buf_size - (cur - out_buf) - 1;
		if (len_remain < (2 + delim_len)
		    && !(!delim_after_last && i == (len - 1) && len_remain >= 2))
			break;

		*cur++ = hex_chars[buf[i] >> 4];
		*cur++ = hex_chars[buf[i] & 0xf];

		if (i == (len - 1) && !delim_after_last)
			break;

		while (len_remain > 1 && *delimp) {
			*cur++ = *delimp++;
			len_remain--;
		}
	}
	*cur = '\0';
	return out_buf;
}

static char *_osmo_hexdump(const unsigned char *buf, int len)
{
	_osmo_hexdump_buf(hexd_buff, sizeof(hexd_buff), buf, len, "", true);
	return hexd_buff;
}
#endif

namespace Snow3G__Functions {


/* f8.
* Input key: 128 bit Confidentiality Key as OCT16.
* Input count:32-bit Count, Frame dependent input as INTEGER.
* Input bearer: 5-bit Bearer identity (in the LSB side) as BIT5.
* Input is_dlwnlink: Direction of transmission.
* Input data: length number of bits, input bit stream as OCTETSTRING.
* Output data: Output bit stream. Assumes data is suitably memory
* allocated.
* Encrypts/decrypts blocks of data between 1 and 2^32 bits in length as
* defined in Section 3.
*/
OCTETSTRING f__snow__3g__f8(const OCTETSTRING& key, const INTEGER& count, const INTEGER & bearer,
			    const BOOLEAN& is_downlink, const OCTETSTRING& data)
{
	TTCN_Buffer ttcn_buf_data(data);
	TTCN_Buffer ttcn_buf_key(key);
	uint32_t direction = (uint32_t)is_downlink;

	snow_3g_f8((u8 *)ttcn_buf_key.get_data(), (u32) count, (u32)bearer, direction,
		   (u8 *)ttcn_buf_data.get_data(), ttcn_buf_data.get_len());

	return OCTETSTRING(ttcn_buf_data.get_len(), ttcn_buf_data.get_data());
}

/* f9.
* Input key: 128 bit Integrity Key as OCT16.
* Input count:32-bit Count, Frame dependent input as UINT32.
* Input fresh: 32-bit Random number as UINT32.
* Input is_downlink:1 Direction of transmission.
* Input data: input bit stream.
* Output : 32 bit block used as MAC
* Generates 32-bit MAC using UIA2 algorithm as defined in Section 4.
*/

OCTETSTRING f__snow__3g__f9(const OCTETSTRING& key, const INTEGER& count, const INTEGER& fresh,
			    const BOOLEAN& is_downlink, const OCTETSTRING& data)
{
	TTCN_Buffer ttcn_buf_data(data);
	TTCN_Buffer ttcn_buf_key(key);
	uint32_t direction = (uint32_t)is_downlink;
	uint8_t tmp[4];
	TTCN_Buffer ttcn_buf_mac;

#ifdef DEBUG
	printf("F9: key=%s, count=%u, fresh=%u, direction=%u, ",
		_osmo_hexdump((u8 *)ttcn_buf_key.get_data(), ttcn_buf_key.get_len()), (u32) count,
		(u32) fresh, direction);
	printf("data=%s -> ", _osmo_hexdump(ttcn_buf_data.get_data(), ttcn_buf_data.get_len()));
#endif
	snow_3g_f9((u8 *)ttcn_buf_key.get_data(), (u32) count, (u32) fresh, direction,
		   (u8 *)ttcn_buf_data.get_data(), ttcn_buf_data.get_len()*8, tmp);
#ifdef DEBUG
	printf("%s\n", _osmo_hexdump(tmp, sizeof(tmp)));
#endif

	return OCTETSTRING(4, tmp);
}

} // namespace
