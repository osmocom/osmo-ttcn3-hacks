
#include <string.h>
#include <stdarg.h>
#include "S1AP_PDU_Descriptions.hh"

extern "C" {
#include <fftranscode/transcode.h>
}

namespace S1AP__Types {

TTCN_Module S1AP__EncDec("S1AP_EncDec", __DATE__, __TIME__);

OCTETSTRING enc__S1AP__PDU(const S1AP__PDU__Descriptions::S1AP__PDU &pdu)
{
	uint8_t *aper_buf;
	int aper_buf_len;
	TTCN_Buffer TTCN_buf;
	TTCN_buf.clear();

	/* Encode from abstract data type into BER/DER */
	pdu.encode(S1AP__PDU__Descriptions::S1AP__PDU_descr_, TTCN_buf,
		   TTCN_EncDec::CT_BER, BER_ENCODE_DER);

	aper_buf_len = fftranscode_ber2aper(FFTRANSC_T_S1AP, &aper_buf, TTCN_buf.get_data(), TTCN_buf.get_len());
	if (aper_buf_len < 0) {
		TTCN_error("fftranscode failed.");
	}

	/* make octetstring from output buffer */
	OCTETSTRING ret_val(aper_buf_len, aper_buf);

	/* release dynamically-allocated output buffer */
	fftranscode_free(aper_buf);

	return ret_val;
}

S1AP__PDU__Descriptions::S1AP__PDU dec__S1AP__PDU(const OCTETSTRING &stream)
{
	uint8_t *ber_buf;
	int ber_buf_len;

	/* First, decode APER + re-encode as BER */
	ber_buf_len = fftranscode_aper2ber(FFTRANSC_T_S1AP, &ber_buf, (const unsigned char *)stream, stream.lengthof());
	if (ber_buf_len < 0) {
		TTCN_error("fftranscode failed.");
	}

	/* Then, re-encode from BER to TITAN representation */
	S1AP__PDU__Descriptions::S1AP__PDU ret_dcc;
	TTCN_Buffer TTCN_buf;
	TTCN_buf.clear();
	TTCN_buf.put_s(ber_buf_len, ber_buf);

	ret_dcc.decode(S1AP__PDU__Descriptions::S1AP__PDU_descr_, TTCN_buf,
			TTCN_EncDec::CT_BER, BER_ACCEPT_ALL);

	fftranscode_free(ber_buf);

	return ret_dcc;
}

}
