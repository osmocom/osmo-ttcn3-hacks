#include "UD_PortType.hh"
#include "L1CTL_PortType.hh"
#include "UD_PT.hh"

namespace L1CTL__PortType__CtrlFunct {

	void f__L1CTL__setGetMsgLen(
		L1CTL__PortType::L1CTL__PT& portRef,
		const INTEGER& id,
		Socket__API__Definitions::f__getMsgLen& f,
		const Socket__API__Definitions::ro__integer& msgLenArgs) {
			f__UD__PT_PROVIDER__setGetMsgLen(portRef, id, f, msgLenArgs);
	}

}
