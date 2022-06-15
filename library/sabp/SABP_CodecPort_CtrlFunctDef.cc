#include "IPL4asp_PortType.hh"
#include "SABP_CodecPort.hh"
#include "IPL4asp_PT.hh"

namespace SABP__CodecPort__CtrlFunct {

  IPL4asp__Types::Result f__IPL4__listen(
    SABP__CodecPort::SABP__CODEC__PT& portRef,
    const IPL4asp__Types::HostName& locName,
    const IPL4asp__Types::PortNumber& locPort,
    const IPL4asp__Types::ProtoTuple& proto,
    const IPL4asp__Types::OptionList& options)
  {
    return f__IPL4__PROVIDER__listen(portRef, locName, locPort, proto, options);
  }
  
  IPL4asp__Types::Result f__IPL4__connect(
    SABP__CodecPort::SABP__CODEC__PT& portRef,
    const IPL4asp__Types::HostName& remName,
    const IPL4asp__Types::PortNumber& remPort,
    const IPL4asp__Types::HostName& locName,
    const IPL4asp__Types::PortNumber& locPort,
    const IPL4asp__Types::ConnectionId& connId,
    const IPL4asp__Types::ProtoTuple& proto,
    const IPL4asp__Types::OptionList& options)
  {
    return f__IPL4__PROVIDER__connect(portRef, remName, remPort,
                                      locName, locPort, connId, proto, options);
  }

  IPL4asp__Types::Result f__IPL4__close(
    SABP__CodecPort::SABP__CODEC__PT& portRef, 
    const IPL4asp__Types::ConnectionId& connId, 
    const IPL4asp__Types::ProtoTuple& proto)
  {
      return f__IPL4__PROVIDER__close(portRef, connId, proto);
  }

  IPL4asp__Types::Result f__IPL4__setUserData(
    SABP__CodecPort::SABP__CODEC__PT& portRef,
    const IPL4asp__Types::ConnectionId& connId,
    const IPL4asp__Types::UserData& userData)
  {
    return f__IPL4__PROVIDER__setUserData(portRef, connId, userData);
  }
  
  IPL4asp__Types::Result f__IPL4__getUserData(
    SABP__CodecPort::SABP__CODEC__PT& portRef,
    const IPL4asp__Types::ConnectionId& connId,
    IPL4asp__Types::UserData& userData)
  {
    return f__IPL4__PROVIDER__getUserData(portRef, connId, userData);
  }

  void f__IPL4__setGetMsgLen(
    SABP__CodecPort::SABP__CODEC__PT& portRef,
    const IPL4asp__Types::ConnectionId& connId,
    Socket__API__Definitions::f__getMsgLen& f,
    const Socket__API__Definitions::ro__integer& msgLenArgs)
  {
    return f__IPL4__PROVIDER__setGetMsgLen(portRef, connId, f, msgLenArgs);
  }


}

