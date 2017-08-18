/* Utility functions that I'm used to from C but for which I couldn't find TTCN-3 implementations
 *
 * (C) 2017 by Harald Welte <laforge@gnumonks.org>
 */

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <Charstring.hh>
#include <Octetstring.hh>

namespace Native__Functions {

OCTETSTRING f__inet__addr(const CHARSTRING& in)
{
	TTCN_Buffer ttcn_buffer(in);
	in_addr_t ia;

	ia = inet_addr((const char *)ttcn_buffer.get_data());

	return OCTETSTRING(4, (const unsigned char *)&ia);
}

} // namespace
