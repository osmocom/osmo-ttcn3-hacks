#include "IPL4asp_PortType.hh"
#include "IPL4asp_PT.hh"
#include "GTPv1U_CodecPort.hh"

namespace GTPv1U__CodecPort__CtrlFunct {

  IPL4asp__Types::Result f__GTPU__listen(
    GTPv1U__CodecPort::GTPU__PT& portRef,
    const IPL4asp__Types::HostName& locName,
    const IPL4asp__Types::PortNumber& locPort,
    const IPL4asp__Types::ProtoTuple& proto,
    const IPL4asp__Types::OptionList& options)
  {
    return f__IPL4__PROVIDER__listen(portRef, locName, locPort, proto, options);
  }

}
