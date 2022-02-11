
/* Utility functions that I'm used to from C but for which I couldn't find TTCN-3 implementations
 *
 * (C) 2017 Harald Welte <laforge@gnumonks.org>
 * All rights reserved.
 *
 * Released under the terms of GNU General Public License, Version 2 or
 * (at your option) any later version.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>

#include <Charstring.hh>
#include <Octetstring.hh>

namespace Native__Functions {

OCTETSTRING f__inet6__addr(const CHARSTRING& in)
{
	char buf[INET6_ADDRSTRLEN];
	TTCN_Buffer ttcn_buffer(in);
	int ret;

	ret = inet_pton(AF_INET6, (const char *)ttcn_buffer.get_data(), buf);
	if(ret < 1)
		fprintf(stderr, "inet_pton failed: %d %s\n", ret, strerror(errno));

	return OCTETSTRING(16, (const unsigned char *)&buf[0]);
}

OCTETSTRING f__inet__addr(const CHARSTRING& in)
{
	TTCN_Buffer ttcn_buffer(in);
	in_addr_t ia;

	ia = inet_addr((const char *)ttcn_buffer.get_data());

	return OCTETSTRING(4, (const unsigned char *)&ia);
}

OCTETSTRING f__inet__haddr(const CHARSTRING& in)
{
	TTCN_Buffer ttcn_buffer(in);
	in_addr_t ia;

	ia = inet_addr((const char *)ttcn_buffer.get_data());
	ia = ntohl(ia);

	return OCTETSTRING(4, (const unsigned char *)&ia);
}

CHARSTRING f__inet__ntoa(const OCTETSTRING& in)
{
	TTCN_Buffer ttcn_buffer(in);
	const struct in_addr ia = *(const struct in_addr *)ttcn_buffer.get_data();
	const char *str = inet_ntoa(ia);

	return CHARSTRING(str);
}

CHARSTRING f__inet__hntoa(const OCTETSTRING& in)
{
	TTCN_Buffer ttcn_buffer(in);
	struct in_addr ia = *(const in_addr *)ttcn_buffer.get_data();
	ia.s_addr = htonl(ia.s_addr);
	const char *str = inet_ntoa(ia);

	return CHARSTRING(str);
}

CHARSTRING f__inet6__ntoa(const OCTETSTRING& in)
{
	char buf[INET6_ADDRSTRLEN] = { 0 };
	TTCN_Buffer ttcn_buffer(in);

	const void *src = (const void *)ttcn_buffer.get_data();
	const char *str = inet_ntop(AF_INET6, src, buf, sizeof(buf));
	if (str == NULL)
		fprintf(stderr, "inet_ntop failed: %s\n", strerror(errno));

	return CHARSTRING((const char *)buf);
}

CHARSTRING f__str__tolower(const CHARSTRING& in)
{
	TTCN_Buffer ttcn_buffer(in);
	TTCN_Buffer buf_out;
	CHARSTRING out;
	unsigned int i;

	const char *in_str = (const char *)ttcn_buffer.get_data();
	for (i = 0; i < strlen(in_str); i++)
		buf_out.put_c((unsigned char) tolower(in_str[i]));
	buf_out.get_string(out);
	return out;
}

CHARSTRING f__str__toupper(const CHARSTRING& in)
{
	TTCN_Buffer ttcn_buffer(in);
	TTCN_Buffer buf_out;
	CHARSTRING out;
	unsigned int i;

	const char *in_str = (const char *)ttcn_buffer.get_data();
	for (i = 0; i < strlen(in_str); i++)
		buf_out.put_c((unsigned char) toupper(in_str[i]));
	buf_out.get_string(out);
	return out;
}



} // namespace
