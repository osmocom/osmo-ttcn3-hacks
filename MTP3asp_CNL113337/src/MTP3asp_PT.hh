///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// Copyright Test Competence Center (TCC) ETH 2009                           //
//                                                                           //
// The copyright to the computer  program(s) herein  is the property of TCC. //
// The program(s) may be used and/or copied only with the written permission //
// of TCC or in accordance with  the terms and conditions  stipulated in the //
// agreement/contract under which the program(s) have been supplied          //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////
//
//  File:         MTP3asp_PT.hh
//  Description:  Implementation of port MTP3asp_PT
//                This test port is written to connect ttcn to SEA 
//                according to specification ITU-T SS7 MTP3, ANSI, TCC, MPT, IETF, 
//  Reference:    ITU-T Recommendation Q.704, RFC3332 
//  Rev:          R11A01
//  Prodnr:       CNL 113 337
//  Updated:      2009-04-03
//  Contact:      http://ttcn.ericsson.se


#ifndef MTP3asp_PT_HH
#define MTP3asp_PT_HH

#include <TTCN3.hh>

#ifdef TARGET_TEST
#include "Abstract_Socket.hh"
#endif

#ifndef TARGET_TEST
#include "mphclib.h"
#endif


#define MAXSIZE 1532 //+32 needed for M3UA
#define MTP3_ON  1
#define MTP3_OFF 0
namespace MTP3asp__Types {
  class ASP__MTP3__TRANSFERind;
  class ASP__MTP3__PAUSE;
  class ASP__MTP3__RESUME;
  class ASP__MTP3__STATUS;
  class ASP__MTP3__TRANSFERreq;
}
namespace MTP3asp__PortType {

class MTP3asp__PT_PROVIDER : public PORT
#ifdef TARGET_TEST
    , public Abstract_Socket
#endif
 {
protected:
  // pointer to member: user_map/unmap methods
  typedef void (MTP3asp__PT_PROVIDER::*usermap_t)(const char *system_port);
  // pointer to member: user_connect methods
  typedef void (MTP3asp__PT_PROVIDER::*userconnect_t)();
  // pointer to member: interpreter methods
#ifndef TARGET_TEST
  typedef void (MTP3asp__PT_PROVIDER::*interpreter_t)(unsigned char* inbuffer,int length,int from_channel,CONNECTION* con);

  void M3UA_interpreter(unsigned char* inbuffer,int length,int from_channel,CONNECTION* con);
  void M3UA_user_map(const char *system_port);
  void M3UA_user_connect();
  void M3UA_user_unmap(const char *system_port);
  void MTP3_interpreter(unsigned char* inbuffer,int length,int from_channel,CONNECTION* con);
  void MTP3_user_map(const char *system_port);  // common for MTP3 ITU and MTP3 ANSI
  void MTP3_user_connect();
  void MTP3_user_unmap(const char *system_port);// common for MTP3 ITU and MTP3 ANSI
  void STC_user_connect();
  void STC_interpreter(unsigned char* inbuffer,int length,int from_channel,CONNECTION* con);
  void STC_user_map(const char *system_port);
  void STC_user_unmap(const char *system_port);
#endif
#ifdef TARGET_TEST
  //Map and unmap for target
  void Target_user_map(const char *system_port);
  void Target_user_unmap(const char *system_port);
  void TargetSTC_user_map(const char *system_port);
#endif
  
public:
  MTP3asp__PT_PROVIDER(const char *par_port_name=NULL);
  ~MTP3asp__PT_PROVIDER();
  
  typedef enum { MTP3itu, MTP3ansi, MTP3ttc, MTP3mpt, M3UA, TargetM3UA, MTP3bttc,MTP3iup, STC, TargetSTC } MTPServiceType_t ;

  MTPServiceType_t MTPServiceType ; // ctor default is MTP3itu
  void set_parameter(const char *parameter_name, const char *parameter_value);
  void error(const char *msg, ...);
  void log(const char *msg, ...);
#ifndef TARGET_TEST
  void user_connect();

  void set_channel(int chnl) {channel=chnl;};
  void MTP3_open_channel(boolean http);
  void MTP3_close_connection();

  boolean wait_for_open();
  int conn_state;

  interpreter_t interpreter; // pointer to interpreter members, ctor default is MTP3_ITU_interpreter
  void doInterpret(unsigned char* inbuffer,int length,int from_channel,CONNECTION* con)
  { (this->*interpreter)(inbuffer,length,from_channel, con); }
#endif
  //parameter handling
  boolean dynamicConnection, connectionUp;
  char *hostname;
  int httpport;
  char *entityname;
  int MTP_fd;

protected: 
  void user_map(const char *system_port);
  void user_unmap(const char *system_port);

  void debuglog(const char *msg, ...);
  void warn(const char *msg, ...);
  void close_log_event();

  usermap_t user_map_p ; //  pointer to user_map members, default is MTP3itu
  usermap_t user_unmap_p; // pointer to user_unmap members, default is MTP3itu
  userconnect_t user_connect_p;

  void user_start();
  void user_stop();
  
  void outgoing_send(const MTP3asp__Types::ASP__MTP3__TRANSFERreq& send_par);
  virtual void incoming_message
    (const MTP3asp__Types::ASP__MTP3__TRANSFERind& incoming_par) = 0;
  virtual void incoming_message
    (const MTP3asp__Types::ASP__MTP3__PAUSE& incoming_par) = 0;
  virtual void incoming_message
    (const MTP3asp__Types::ASP__MTP3__RESUME& incoming_par) = 0;
  virtual void incoming_message
    (const MTP3asp__Types::ASP__MTP3__STATUS& incoming_par) = 0;
#ifndef TARGET_TEST
  void encode_56bLSB_int(unsigned char *to, unsigned long long int from);
  long long unsigned int decode_56bLSB_int(const unsigned char *from);
  void encode_48bLSB_int(unsigned char *to, unsigned long long int from);
  long long unsigned int decode_48bLSB_int(const unsigned char *from);
  void encode_40bLSB_int(unsigned char *to, unsigned long long int from);
  long long unsigned int decode_40bLSB_int(const unsigned char *from);
  void encode_32bLSB_int(unsigned char *to, unsigned int from);
  unsigned int decode_32bLSB_int(const unsigned char *from);
  void encode_24bLSB_int(unsigned char *to, int from);
  unsigned int decode_24bLSB_int(const unsigned char *from);
  void encode_16bLSB_int(unsigned char *to, int from);
  unsigned int decode_16bLSB_int(const unsigned char *from);
  void encode_32b_int(unsigned char *to, unsigned int from);
  unsigned int decode_32b_int(const unsigned char *from);
  void encode_24b_int(unsigned char *to, unsigned int from);
  unsigned int decode_24b_int(const unsigned char *from);
  void encode_16b_int(unsigned char *to, int from);
  unsigned int decode_16b_int(const unsigned char *from);
  void skip_par_after_tag(unsigned char* inbuffer, int &offset);

  void GetPointCodes(unsigned int &sls,unsigned int &opc,unsigned int &dpc, unsigned char* msg);
  void GetPointCodesIUP(unsigned int &cic,unsigned int &opc,unsigned int &dpc, unsigned char* msg);
  void SetPointCodes(unsigned int sls, unsigned int opc, unsigned int dpc, unsigned char* msg);
  void SetPointCodesIUP(unsigned int cic, unsigned int opc, unsigned int dpc, unsigned char* msg);
  int  ChangePointCodes(unsigned char* outbuff, unsigned char *inbuff, int len);
  void Set_M3UA_SingleAPC(unsigned int pc, unsigned char* apc_par); //for ITU and ANSI
  bool Check_M3UA_SingleITUAPC(unsigned int pc, unsigned char* apc_par);
  void Send_DAVA_DUNA_to_APCinDAUD(unsigned int dava_sep, unsigned char* inbuffer, int &offset );
  void processing_MTP3_management_msg(unsigned char* inbuff,int len);
  void processing_MTP3_test_msg(unsigned char* inbuff,int len);
  int processing_M3UA_MGMT_msg(unsigned char* inbuff,int len);
  int processing_M3UA_Transfer_msg(unsigned char* inbuff,int len);
  int processing_M3UA_SSNM_msg(unsigned char* inbuff,int len);
  int processing_M3UA_ASPSM_msg(unsigned char* inbuff,int len);
  int processing_M3UA_ASPTM_msg(unsigned char* inbuff,int len);
  int processing_M3UA_RKM_msg(unsigned char* inbuff,int len);
  int processing_M3UA_unsupported_msg_class(unsigned char* inbuff,int len);
  int  Check_PcMatch(unsigned int opc, unsigned int dpc, unsigned char *buff);
  void send_msg(unsigned char *outbuff, int length);
  void send_M3UA_error_msg(unsigned int error_code, unsigned int add_par );
  void Check_TestPort_Variables();
  void Check_TestPort_Variables_STC();
#endif
#ifdef TARGET_TEST
  void Check_Target_TestPort_Variables();
  void Check_Target_TestPort_Variables_STC();

  //Functions for abstract socket handling
  void message_incoming(const unsigned char*, int length, int client_id = -1);
  void Add_Fd_Read_Handler(int fd) { Handler_Add_Fd_Read(fd); }
  void Add_Fd_Write_Handler(int fd) { Handler_Add_Fd_Write(fd); }
  void Remove_Fd_Read_Handler(int fd) { Handler_Remove_Fd_Read(fd); }
  void Remove_Fd_Write_Handler(int fd) { Handler_Remove_Fd_Write(fd); }
  void Remove_Fd_All_Handlers(int fd) { Handler_Remove_Fd(fd); }
  void Handler_Uninstall() { Uninstall_Handler(); }
  void Timer_Set_Handler(double call_interval, boolean is_timeout = TRUE,
    boolean call_anyway = TRUE, boolean is_periodic = TRUE) {
    Handler_Set_Timer(call_interval, is_timeout, call_anyway, is_periodic);
  }
  const char* local_address_name() { return "localIPAddr";}
  const char* local_port_name()    { return "localPort";}
  const char* remote_address_name(){ return "M3UAtarget_TCP_IPAddr";}
  const char* remote_port_name()   { return "M3UAtarget_TCP_Port";}
  const char* halt_on_connection_reset_name(){ return "halt_on_connection_reset";}
  const char* server_mode_name()   { return "client_mode";}
  const char* socket_debugging_name(){ return "socket_debugging";}
  const char* nagling_name()       { return "nagling";}
  const char* server_backlog_name(){ return "server_backlog";}
  const PacketHeaderDescr* Get_Header_Descriptor() const {return header_descr;}
#endif

private:
    void Handle_Fd_Event(int fd, boolean is_readable, boolean is_writable, boolean is_error);
    void Handle_Timeout(double time_since_last_call);

    unsigned char M3UA_version;
    int channel;
    unsigned char buffer[MAXSIZE];
    char *destinationname;
    char *iid_string;
    boolean Ni_is_set;
    int Loop,Filter,Sut_Pc,Tester_Pc;
    typedef enum { AssocDown, AssocEstabl, AssocInac, AssocActive} M3UAStateType_t;
    M3UAStateType_t M3UAState;
    int mtp3_ni; // network indicator in case of MTP3
    int stored_bttc_octet; // used for storage of an additional first byte in MTP3bttc
    
#ifndef TARGET_TEST
    MPH_IID iid;
    CONNECTION *myConnection;
#endif
    bool forward_resume, forward_pause, forward_status;
#ifdef TARGET_TEST
    int Tcp_is_up;
    PacketHeaderDescr *header_descr;
    bool is_packet_hdr_length_offset, is_packet_hdr_nr_bytes_in_length,
	    is_packet_hdr_byte_order;
    int packet_hdr_length_offset, packet_hdr_nr_bytes_in_length;
    PacketHeaderDescr::HeaderByteOrder packet_hdr_byte_order;
#endif
};

extern BOOLEAN f__MTP3__SEA__connect__extern
  (MTP3asp__PT_PROVIDER& portRef, const CHARSTRING& Hostname,
   const INTEGER& Port, const CHARSTRING& EntityName,const BOOLEAN& Http);

extern BOOLEAN f__MTP3__SEA__disconnect__extern
  (MTP3asp__PT_PROVIDER& portRef);

}
#endif
