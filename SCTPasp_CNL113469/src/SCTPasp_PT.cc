/******************************************************************************
* Copyright (c) 2005, 2014  Ericsson AB
* All rights reserved. This program and the accompanying materials
* are made available under the terms of the Eclipse Public License v1.0
* which accompanies this distribution, and is available at
* http://www.eclipse.org/legal/epl-v10.html
*
* Contributors:
*  Peter Dimitrov- initial implementation and initial documentation
*  Adam Delic
*  Eduard Czimbalmos
*  Endre Kulcsar
*  Gabor Bettesch
*  Gabor Szalai
*  Tamas Buti
*  Zoltan Medve
******************************************************************************/
//
//  File:               SCTPasp_PT.cc
//  Description:        SCTPasp test port source
//  Rev:                R11A
//  Prodnr:             CNL 113 469
//


#include "SCTPasp_PT.hh"

#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <stdarg.h>
#include <memory.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <limits.h>

#define BUFLEN 1024
#define MAP_LENGTH 10
#ifdef SCTP_ADAPTION_LAYER
  #ifdef LKSCTP_1_0_7
    #undef LKSCTP_1_0_7
    #error LKSCTP_1_0_7 defined but the lksctp older than 1.0.7. Use only -DUSE_SCTP, version is automatically selected
  #endif  
  #ifdef LKSCTP_1_0_9
    #error LKSCTP_1_0_9 defined but the lksctp older than 1.0.7. Use only -DUSE_SCTP, version is automatically selected
    #undef LKSCTP_1_0_9
  #endif  
#else 
// 1.0.7 or newer
  #ifdef SCTP_AUTH_CHUNK 
    // 1.0.9 or newer
    #ifdef LKSCTP_1_0_7
      #undef LKSCTP_1_0_7
      #error LKSCTP_1_0_7 defined but the lksctp newer than 1.0.7. Use only -DUSE_SCTP, version is automatically selected
    #endif
    #ifndef LKSCTP_1_0_9
      #define LKSCTP_1_0_9
    #endif  
  #else
  // 1.0.7
    #ifdef LKSCTP_1_0_9
      #undef LKSCTP_1_0_9
      #error LKSCTP_1_0_9 defined but the lksctp older than 1.0.9. Use only -DUSE_SCTP, version is automatically selected
    #endif
    #ifndef LKSCTP_1_0_7
      #define LKSCTP_1_0_7
    #endif  
    
  #endif

#endif

namespace SCTPasp__PortType {

struct SCTPasp__PT_PROVIDER::fd_map_item
{   // used by map operations
  int fd; // socket descriptor
  boolean erased;
  boolean processing_message; // if true only part of the message is received
  boolean einprogress; // connection establishment is in progress
  void * buf; // buffer
  ssize_t buflen; // length of the buffer
  ssize_t nr; // number of received bytes
  struct sockaddr_in  sin; // storing remote address
};


struct SCTPasp__PT_PROVIDER::fd_map_server_item // server item
{   // used by map operations
  int fd; // socket descriptor
  boolean erased;
  struct in_addr local_IP_address;
  unsigned short local_port;
};


SCTPasp__PT_PROVIDER::SCTPasp__PT_PROVIDER(const char *par_port_name)
  : PORT(par_port_name)
{
  simple_mode = FALSE;
  reconnect = FALSE;
  reconnect_max_attempts = 6;
  server_mode = FALSE;
  debug = FALSE;
  server_backlog = 1;
  local_IP_address.s_addr = INADDR_ANY;
  (void) memset(&initmsg, 0, sizeof(struct sctp_initmsg));
  initmsg.sinit_num_ostreams = 64;
  initmsg.sinit_max_instreams = 64;
  initmsg.sinit_max_attempts = 0;
  initmsg.sinit_max_init_timeo = 0;
  (void) memset(&events, 0, sizeof (events));
  events.sctp_data_io_event = TRUE;
  events.sctp_association_event = TRUE;
  events.sctp_address_event = TRUE;
  events.sctp_send_failure_event = TRUE;
  events.sctp_peer_error_event = TRUE;
  events.sctp_shutdown_event = TRUE;
  events.sctp_partial_delivery_event = TRUE;
#if defined(LKSCTP_1_0_7) || defined(LKSCTP_1_0_9)
  events.sctp_adaptation_layer_event = TRUE;
#else
  events.sctp_adaption_layer_event = TRUE;
#endif
  local_port_is_present = FALSE;
  peer_IP_address_is_present = FALSE;
  peer_port_is_present = FALSE;

  fd_map=NULL;
  list_len=0;

  fd_map_server=NULL;
  list_len_server=0;

  fd = -1;
  FD_ZERO(&readfds);
  FD_ZERO(&writefds);
  local_port=-1;
  peer_port=-1;
  receiving_fd=-1;
}


SCTPasp__PT_PROVIDER::~SCTPasp__PT_PROVIDER()
{
  for(int i=0;i<list_len;i++) map_delete_item(i);
  Free(fd_map);

  if(!simple_mode)
  {
  for(int i=0;i<list_len_server;i++) map_delete_item_server(i);
  Free(fd_map_server);
  }
}


void SCTPasp__PT_PROVIDER::set_parameter(const char *parameter_name,
  const char *parameter_value)
{

  if(strcmp(parameter_name, "simple_mode") == 0)
  {
  if (strcasecmp(parameter_value,"yes") == 0)
    simple_mode = TRUE;
  else if(strcasecmp(parameter_value,"no") == 0)
    simple_mode = FALSE;
  else
    error("set_parameter(): Invalid parameter value: %s for parameter %s. Only yes and no can be used!" ,
    parameter_value, parameter_name);
  }
  else if(strcmp(parameter_name, "reconnect") == 0)
  {
  if (strcasecmp(parameter_value,"yes") == 0)
    reconnect = TRUE;
  else if(strcasecmp(parameter_value,"no") == 0)
    reconnect = FALSE;
  else
    error("set_parameter(): Invalid parameter value: %s for parameter %s. Only yes and no can be used!" ,
    parameter_value, parameter_name);
  }
  else if(strcmp(parameter_name, "reconnect_max_attempts") == 0)
  {
  int value;
  if ( (sscanf(parameter_value, "%d", &value) == 1) && (value>=0) )
    reconnect_max_attempts = value;
  else
    error("set_parameter(): Invalid parameter value: %s for parameter %s. It should be positive integer!" ,
    parameter_value, parameter_name);
  }
  else if(strcmp(parameter_name, "server_mode") == 0)
  {
  if (strcasecmp(parameter_value,"yes") == 0)
    server_mode = TRUE;
  else if(strcasecmp(parameter_value,"no") == 0)
    server_mode = FALSE;
  else
    error("set_parameter(): Invalid parameter value: %s for parameter %s. Only yes and no can be used!" ,
    parameter_value, parameter_name);
  }
  else if(strcmp(parameter_name, "debug") == 0)
  {
  if (strcasecmp(parameter_value,"yes") == 0)
    debug = TRUE;
  else if(strcasecmp(parameter_value,"no") == 0)
    debug = FALSE;
  else
    error("set_parameter(): Invalid parameter value: %s for parameter %s. Only yes and no can be used!" ,
    parameter_value, parameter_name);
  }
  else if(strcmp(parameter_name, "server_backlog") == 0)
  {
  int value;
  if ( (sscanf(parameter_value, "%d", &value) == 1) && (value>=0) )
    server_backlog = value;
  else
    error("set_parameter(): Invalid parameter value: %s for parameter %s. It should be positive integer!" ,
    parameter_value, parameter_name);
  }
  else if(strcmp(parameter_name, "local_IP_address") == 0)
  {
    local_IP_address = get_in_addr((const char *) parameter_value);
  }
  else if(strcmp(parameter_name, "local_port") == 0)
  {
  unsigned short value;
  if (sscanf(parameter_value, "%hu", &value) == 1)
  {
    local_port = value;
    local_port_is_present = TRUE;
  }
  else
    error("set_parameter(): Invalid parameter value: %s for parameter %s. It should be positive integer!" ,
    parameter_value, parameter_name);
  }
  else if(strcmp(parameter_name, "peer_IP_address") == 0)
  {
  peer_IP_address = get_in_addr((const char *) parameter_value);
  peer_IP_address_is_present = TRUE;
  }
  else if(strcmp(parameter_name, "peer_port") == 0)
  {
  unsigned short value;
  if (sscanf(parameter_value, "%hu", &value) == 1)
  {
    peer_port = value;
    peer_port_is_present = TRUE;
  }
  else
    error("set_parameter(): Invalid parameter value: %s for parameter %s. It should be positive integer!" ,
    parameter_value, parameter_name);
  }
  else if(strcmp(parameter_name, "sinit_num_ostreams") == 0)
  {
  long value;
  if ( (sscanf(parameter_value, "%ld", &value) == 1) && (value>=0) )
    initmsg.sinit_num_ostreams = value;
  else
    error("set_parameter(): Invalid parameter value: %s for parameter %s. It should be positive integer!" ,
    parameter_value, parameter_name);
  }
  else if(strcmp(parameter_name, "sinit_max_instreams") == 0)
  {
  long value;
  if ( (sscanf(parameter_value, "%ld", &value) == 1) && (value>=0) )
    initmsg.sinit_max_instreams = value;
  else
    error("set_parameter(): Invalid parameter value: %s for parameter %s. It should be positive integer!" ,
    parameter_value, parameter_name);
  }
  else if(strcmp(parameter_name, "sinit_max_attempts") == 0)
  {
  long value;
  if ( (sscanf(parameter_value, "%ld", &value) == 1) && (value>=0) )
    initmsg.sinit_max_attempts = value;
  else
    error("set_parameter(): Invalid parameter value: %s for parameter %s. It should be positive integer!" ,
    parameter_value, parameter_name);
  }
  else if(strcmp(parameter_name, "sinit_max_init_timeo") == 0)
  {
  long value;
  if ( (sscanf(parameter_value, "%ld", &value) == 1) && (value>=0) )
    initmsg.sinit_max_init_timeo = value;
  else
    error("set_parameter(): Invalid parameter value: %s for parameter %s. It should be positive integer!" ,
    parameter_value, parameter_name);
  }
  else if(strcmp(parameter_name, "sctp_association_event") == 0)
  {
  if (strcasecmp(parameter_value,"enabled") == 0)
    events.sctp_association_event = TRUE;
  else if(strcasecmp(parameter_value,"disabled") == 0)
    events.sctp_association_event = FALSE;
  else
    error("set_parameter(): Invalid parameter value: %s for parameter %s. It should be enabled or disabled!" ,
    parameter_value, parameter_name);
  }
  else if(strcmp(parameter_name, "sctp_address_event") == 0)
  {
  if (strcasecmp(parameter_value,"enabled") == 0)
    events.sctp_address_event = TRUE;
  else if(strcasecmp(parameter_value,"disabled") == 0)
    events.sctp_address_event = FALSE;
  else
    error("set_parameter(): Invalid parameter value: %s for parameter %s. It should be enabled or disabled!" ,
    parameter_value, parameter_name);
  }
  else if(strcmp(parameter_name, "sctp_send_failure_event") == 0)
  {
  if (strcasecmp(parameter_value,"enabled") == 0)
    events.sctp_send_failure_event = TRUE;
  else if(strcasecmp(parameter_value,"disabled") == 0)
    events.sctp_send_failure_event = FALSE;
  else
    error("set_parameter(): Invalid parameter value: %s for parameter %s. It should be enabled or disabled!" ,
    parameter_value, parameter_name);
  }
  else if(strcmp(parameter_name, "sctp_peer_error_event") == 0)
  {
  if (strcasecmp(parameter_value,"enabled") == 0)
    events.sctp_peer_error_event = TRUE;
  else if(strcasecmp(parameter_value,"disabled") == 0)
    events.sctp_peer_error_event = FALSE;
  else
    error("set_parameter(): Invalid parameter value: %s for parameter %s. It should be enabled or disabled!" ,
    parameter_value, parameter_name);
  }
  else if(strcmp(parameter_name, "sctp_shutdown_event") == 0)
  {
  if (strcasecmp(parameter_value,"enabled") == 0)
    events.sctp_shutdown_event = TRUE;
  else if(strcasecmp(parameter_value,"disabled") == 0)
    events.sctp_shutdown_event = FALSE;
  else
    error("set_parameter(): Invalid parameter value: %s for parameter %s. It should be enabled or disabled!" ,
    parameter_value, parameter_name);
  }
  else if(strcmp(parameter_name, "sctp_partial_delivery_event") == 0)
  {
  if (strcasecmp(parameter_value,"enabled") == 0)
    events.sctp_partial_delivery_event = TRUE;
  else if(strcasecmp(parameter_value,"disabled") == 0)
    events.sctp_partial_delivery_event = FALSE;
  else
    error("set_parameter(): Invalid parameter value: %s for parameter %s. It should be enabled or disabled!" ,
    parameter_value, parameter_name);
  }
  else if(strcmp(parameter_name, "sctp_adaption_layer_event") == 0)
  {
  if (strcasecmp(parameter_value,"enabled") == 0)
#if defined(LKSCTP_1_0_7) || defined(LKSCTP_1_0_9)
    events.sctp_adaptation_layer_event = TRUE;
#else
    events.sctp_adaption_layer_event = TRUE;
#endif
   else if(strcasecmp(parameter_value,"disabled") == 0)
#if defined(LKSCTP_1_0_7) || defined(LKSCTP_1_0_9)
    events.sctp_adaptation_layer_event = FALSE;
#else
    events.sctp_adaption_layer_event = FALSE;
#endif
   else
    error("set_parameter(): Invalid parameter value: %s for parameter %s. It should be enabled or disabled!" ,
    parameter_value, parameter_name);
  }
  else
  TTCN_warning("%s: unknown & unhandled parameter: %s",
  get_name(), parameter_name);
  errno = 0;
}


void SCTPasp__PT_PROVIDER::Event_Handler(const fd_set *read_fds,
  const fd_set *write_fds, const fd_set */*error_fds*/,
  double /*time_since_last_call*/)
{
  // Accepting new client
  if(!simple_mode)
  {
    for(int i=0;i<list_len_server;i++)
    {
      if(!fd_map_server[i].erased && FD_ISSET(fd_map_server[i].fd, read_fds))
      {
        int newclient_fd;
        struct sockaddr_in peer_address;
        socklen_t addrlen = sizeof(peer_address);
        if ((newclient_fd = accept(fd_map_server[i].fd, (struct sockaddr *)&peer_address, &addrlen)) == -1)
          error("Event handler: accept error (server mode)!");
        else
        {
          map_put_item(newclient_fd);
          setNonBlocking(newclient_fd);
          FD_SET(newclient_fd, &readfds);
          incoming_message(SCTPasp__Types::ASP__SCTP__Connected(
                        INTEGER(newclient_fd),
                        CHARSTRING(inet_ntoa(fd_map_server[i].local_IP_address)),
                        INTEGER(fd_map_server[i].local_port),
                        CHARSTRING(inet_ntoa(peer_address.sin_addr)),
                        INTEGER(ntohs(peer_address.sin_port))));
          Install_Handler(&readfds, NULL, NULL, 0.0);
        }
      }
    }
  }
  else
  {
    if(server_mode && FD_ISSET(fd, read_fds))
    {
      int newclient_fd;
      struct sockaddr_in peer_address;
      socklen_t addrlen = sizeof(peer_address);
      if ((newclient_fd = accept(fd, (struct sockaddr *)&peer_address, &addrlen)) == -1)
        error("Event handler: accept error (server mode)!");
      else
      {
        map_put_item(newclient_fd);
        setNonBlocking(newclient_fd);
        FD_SET(newclient_fd, &readfds);
        Install_Handler(&readfds, NULL, NULL, 0.0);
      }
    }
  }
  // Receiving data
  for(int i=0;i<list_len;i++)
  {
    if(!simple_mode && !fd_map[i].erased && fd_map[i].einprogress &&
      FD_ISSET(fd_map[i].fd, write_fds))
    {
      if (connect(fd_map[i].fd, (struct sockaddr *)&fd_map[i].sin,
        sizeof (fd_map[i].sin)) == -1)
      {
        if(errno == EISCONN)
        {
          SCTPasp__Types::ASP__SCTP__RESULT asp_sctp_result;
          asp_sctp_result.client__id() = fd_map[i].fd;
          asp_sctp_result.error__status() = FALSE;
          asp_sctp_result.error__message() = OMIT_VALUE;
          incoming_message(asp_sctp_result);
          fd_map[i].einprogress = FALSE;
          FD_CLR(fd_map[i].fd, &writefds);
          FD_SET(fd_map[i].fd, &readfds);
          Install_Handler(&readfds, &writefds, NULL, 0.0);
          errno = 0;
          log("Connection successfully established to (%s):(%d)",
            inet_ntoa(peer_IP_address), peer_port);
        }
        else
        {
          close(fd_map[i].fd);
          fd = -1;
          TTCN_warning("Connect error!");
          SCTPasp__Types::ASP__SCTP__RESULT asp_sctp_result;
          asp_sctp_result.client__id() = fd_map[i].fd;
          asp_sctp_result.error__status() = TRUE;
          asp_sctp_result.error__message() = strerror(errno);
          incoming_message(asp_sctp_result);
          FD_CLR(fd_map[i].fd, &writefds);
          map_delete_item_fd(fd_map[i].fd);
          Install_Handler(&readfds, &writefds, NULL, 0.0);
          errno = 0;
          log("Connection establishment to (%s):(%d) failed !",
            inet_ntoa(peer_IP_address), peer_port);
        }
      }
    }

    if(!fd_map[i].erased && FD_ISSET(fd_map[i].fd, read_fds))
    {
      log("Calling Event_Handler.");
      receiving_fd = fd_map[i].fd;

      struct cmsghdr   *cmsg;
      struct sctp_sndrcvinfo  *sri;
      char cbuf[sizeof (*cmsg) + sizeof (*sri)];
      struct msghdr   msg;
      struct iovec   iov;
      size_t   cmsglen = sizeof (*cmsg) + sizeof (*sri);

      if ( !fd_map[i].processing_message )
      {
        fd_map[i].buf = Malloc(BUFLEN);
        fd_map[i].buflen = BUFLEN;
        iov.iov_base = fd_map[i].buf;
        iov.iov_len = fd_map[i].buflen;
      }
      else
      {
        // Set the next read offset
        log("Event_Handler: setting the next read offset.");
        iov.iov_base = (char *)fd_map[i].buf + fd_map[i].nr;
        iov.iov_len = fd_map[i].buflen - fd_map[i].nr;
      }

      // Set up the msghdr structure for receiving
      memset(&msg, 0, sizeof (msg));
      msg.msg_iov = &iov;
      msg.msg_iovlen = 1;
      msg.msg_control = cbuf;
      msg.msg_controllen = cmsglen;

      memset(cbuf, 0, sizeof (*cmsg) + sizeof (*sri));
      cmsg = (struct cmsghdr *)cbuf;
      sri = (struct sctp_sndrcvinfo *)(cmsg + 1);

      return_value_t value = getmsg(receiving_fd, &msg);
      switch(value)
      {
        case WHOLE_MESSAGE_RECEIVED:
          fd_map[i].processing_message = FALSE;
          // Intercept notifications here
          if (msg.msg_flags & MSG_NOTIFICATION)
          {
            log("Calling event_handler for an incoming notification.");
            handle_event(fd_map[i].buf);
          }
          else
          {
            log("Incoming data.");
            unsigned int ui = ntohl(sri->sinfo_ppid);
            INTEGER i_ppid;
            if (ui <= (unsigned long)INT_MAX)
              i_ppid = ui;
            else {
              char sbuf[16];
              sprintf(sbuf, "%u", ui);
              i_ppid = INTEGER(sbuf);
            }
            incoming_message(SCTPasp__Types::ASP__SCTP(
                    INTEGER(receiving_fd),
                    INTEGER(sri->sinfo_stream),
                    i_ppid,
                    OCTETSTRING(fd_map[i].nr,(const unsigned char *)fd_map[i].buf)));
          }
          Free(fd_map[i].buf);
          fd_map[i].buf = NULL;
          break;
        case PARTIAL_RECEIVE:
          fd_map[i].processing_message = TRUE;
          break;
        case EOF_OR_ERROR:
          if (!server_mode) fd = -1; // setting closed socket to -1 in client mode (and reconnect mode)
          FD_CLR(receiving_fd, &readfds);
          Install_Handler(&readfds, NULL, NULL, 0.0);
          map_delete_item(i);
          if (events.sctp_association_event) incoming_message(SCTPasp__Types::ASP__SCTP__ASSOC__CHANGE(
                  INTEGER(receiving_fd),
                  SCTPasp__Types::SAC__STATE(SCTP_COMM_LOST)));
          log("getmsg() returned with NULL. Socket is closed.");
          if (reconnect) forced_reconnect(reconnect_max_attempts);

          break;
      }//endswitch
    }// endif
  }// endfor
}


void SCTPasp__PT_PROVIDER::user_map(const char *system_port)
{
  log("Calling user_map(%s).",system_port);
  if(simple_mode)
  {
    if ( server_mode && reconnect )
    {
      error("user_map(): server mode and reconnect mode are mutually exclusive!");
    }
    if ( server_mode && !local_port_is_present )
    {
      error("user_map(): in server mode local_port must be defined!");
    }

    // Server mode: turns on listening
    if (server_mode)
    {
      log("Running in SERVER_MODE.");
      create_socket();
      if (listen(fd, server_backlog) == -1) error("Listen error!");
      log("Listening @ (%s):(%d)", inet_ntoa(local_IP_address), local_port);
      FD_SET(fd, &readfds);
      Install_Handler(&readfds, NULL, NULL, 0.0);
    } else if (reconnect) {
      log("Running in RECONNECT MODE.");
      forced_reconnect(reconnect_max_attempts+1);
    } else {
      log("Running in CLIENT MODE.");
    }
  }
  else
  {
    log("Running in NORMAL MODE.");
  }
  log("Leaving user_map().");
}


void SCTPasp__PT_PROVIDER::user_unmap(const char *system_port)
{
  log("Calling user_unmap(%s).",system_port);
  Uninstall_Handler();
  FD_ZERO(&readfds);
  FD_ZERO(&writefds);
  if(!simple_mode)
  {
    for(int i=0;i<list_len;i++) map_delete_item(i);
    for(int i=0;i<list_len_server;i++) map_delete_item_server(i);
  }
  else
  {
    for(int i=0;i<list_len;i++) map_delete_item(i);
    if(server_mode) close(fd);
  }
  log("Leaving user_unmap().");
}


void SCTPasp__PT_PROVIDER::user_start()
{
  log("Calling user_start().");
  log("Leaving user_start().");
}


void SCTPasp__PT_PROVIDER::user_stop()
{
  log("Calling user_stop().");
  log("Leaving user_stop().");
}


void SCTPasp__PT_PROVIDER::outgoing_send(const SCTPasp__Types::ASP__SCTP__Connect& send_par)
{
  log("Calling outgoing_send (ASP_SCTP_CONNECT).");
  if(simple_mode)
  {
    if (server_mode)
      error("ASP_SCTP_CONNECT is not allowed in server mode!");
  }
  if( !peer_IP_address_is_present && !send_par.peer__hostname().ispresent() )
    error("Peer IP address should be defined!");

  if( !peer_port_is_present && !send_par.peer__portnumber().ispresent() )
    error("Peer port should be defined!");
  if(!simple_mode)
  {
    boolean temp_bool = local_port_is_present;
    local_port_is_present = FALSE;
    create_socket();  // creating client socket
    local_port_is_present = temp_bool;
  }
  else
  {
    if (fd == -1) create_socket();  // checking if there is an open socket
    else if(FD_ISSET(fd, &readfds)) // Active connection
      error("ASP_SCTP_CONNECT called during active connection.");
  }
  struct sockaddr_in  sin;
  if(send_par.peer__hostname().ispresent())
  {
    peer_IP_address = get_in_addr((const char *)(const CHARSTRING&)send_par.peer__hostname());
  }
  if(send_par.peer__portnumber().ispresent())
    peer_port = (int) (const INTEGER&) send_par.peer__portnumber();

  sin.sin_family = AF_INET;
  sin.sin_port = htons(peer_port);
  sin.sin_addr.s_addr = peer_IP_address.s_addr;
  log("Connecting to (%s):(%d)", inet_ntoa(peer_IP_address), peer_port);
  // setting non-blocking mode
  if(!simple_mode) setNonBlocking(fd);
  if (connect(fd, (struct sockaddr *)&sin, sizeof (sin)) == -1)
  {
    if(errno == EINPROGRESS && !simple_mode)
    {
      map_put_item(fd);
      int i = map_get_item(fd);
      fd_map[i].einprogress = TRUE;
      fd_map[i].sin.sin_family = AF_INET;
      fd_map[i].sin.sin_port = htons(peer_port);;
      fd_map[i].sin.sin_addr.s_addr= peer_IP_address.s_addr;
      FD_SET(fd, &writefds);
      Install_Handler(&readfds, &writefds, NULL, 0.0);
      log("Connection in progress to (%s):(%d)", inet_ntoa(peer_IP_address),
        peer_port);
    }
    else
    {
      close(fd);
      fd = -1;
      TTCN_warning("Connect error!");
      SCTPasp__Types::ASP__SCTP__RESULT asp_sctp_result;
      asp_sctp_result.client__id() = OMIT_VALUE;
      asp_sctp_result.error__status() = TRUE;
      asp_sctp_result.error__message() = strerror(errno);
      incoming_message(asp_sctp_result);
    }
    errno = 0;
  }
  else
  {
    SCTPasp__Types::ASP__SCTP__RESULT asp_sctp_result;
    asp_sctp_result.client__id() = fd;
    asp_sctp_result.error__status() = FALSE;
    asp_sctp_result.error__message() = OMIT_VALUE;
    incoming_message(asp_sctp_result);
    map_put_item(fd);
    if(simple_mode) setNonBlocking(fd);
    FD_SET(fd, &readfds);
    Install_Handler(&readfds, NULL, NULL, 0.0);
    log("Connection successfully established to (%s):(%d)", inet_ntoa(peer_IP_address), peer_port);
  }
  log("Leaving outgoing_send (ASP_SCTP_CONNECT).");
}


void SCTPasp__PT_PROVIDER::outgoing_send(const SCTPasp__Types::ASP__SCTP__ConnectFrom& send_par)
{
  log("Calling outgoing_send (ASP_SCTP_CONNECTFROM).");
  if(!simple_mode)
  {
    if( !peer_IP_address_is_present && !send_par.peer__hostname().ispresent() )
      error("Peer IP address should be defined!");
    if( !peer_port_is_present && !send_par.peer__portnumber().ispresent() )
      error("Peer port should be defined!");
    // work around for create_socket()
    unsigned short temp = local_port; // saving global variables
    boolean temp_bool = local_port_is_present;
    struct in_addr temp_local_IP_address = local_IP_address;
    local_port = (int) (const INTEGER&) send_par.local__portnumber();
    local_port_is_present = TRUE;
    if(send_par.local__hostname().ispresent())
    {
      local_IP_address = get_in_addr((const char *)(const CHARSTRING&)send_par.local__hostname());
    }
    create_socket();  // creating client socket

    struct sockaddr_in  sin;
    if(send_par.peer__hostname().ispresent())
    {
      peer_IP_address = get_in_addr((const char *)(const CHARSTRING&)send_par.peer__hostname());
    }
    if(send_par.peer__portnumber().ispresent())
      peer_port = (int) (const INTEGER&) send_par.peer__portnumber();

    sin.sin_family = AF_INET;
    sin.sin_port = htons(peer_port);
    sin.sin_addr.s_addr = peer_IP_address.s_addr;
    log("Connecting to (%s):(%d)", inet_ntoa(peer_IP_address), peer_port);
    // setting non-blocking mode
    setNonBlocking(fd);
    if (connect(fd, (struct sockaddr *)&sin, sizeof (sin)) == -1)
    {
      if(errno == EINPROGRESS)
      {
        map_put_item(fd);
        int i = map_get_item(fd);
        fd_map[i].einprogress = TRUE;
        fd_map[i].sin.sin_family = AF_INET;
        fd_map[i].sin.sin_port = htons(peer_port);;
        fd_map[i].sin.sin_addr.s_addr= peer_IP_address.s_addr;
        FD_SET(fd, &writefds);
        Install_Handler(&readfds, &writefds, NULL, 0.0);
        log("Connection in progress to (%s):(%d)", inet_ntoa(peer_IP_address),
          peer_port);
      }
      else
      {
        close(fd);
        fd = -1;
        TTCN_warning("Connect error!");
        SCTPasp__Types::ASP__SCTP__RESULT asp_sctp_result;
        asp_sctp_result.client__id() = OMIT_VALUE;
        asp_sctp_result.error__status() = TRUE;
        asp_sctp_result.error__message() = strerror(errno);
        incoming_message(asp_sctp_result);
      }
      errno = 0;
    }
    else
    {
      SCTPasp__Types::ASP__SCTP__RESULT asp_sctp_result;
      asp_sctp_result.client__id() = fd;
      asp_sctp_result.error__status() = FALSE;
      asp_sctp_result.error__message() = OMIT_VALUE;
      incoming_message(asp_sctp_result);
      map_put_item(fd);
      FD_SET(fd, &readfds);
      Install_Handler(&readfds, NULL, NULL, 0.0);
      log("Connection successfully established to (%s):(%d)", inet_ntoa(peer_IP_address), peer_port);
    }
    local_port = temp; // restoring global variables
    local_port_is_present = temp_bool;
    local_IP_address = temp_local_IP_address;
  }
  log("Leaving outgoing_send (ASP_SCTP_CONNECTFROM).");
}


void SCTPasp__PT_PROVIDER::outgoing_send(const SCTPasp__Types::ASP__SCTP__Listen& send_par)
{
  log("Calling outgoing_send (ASP_SCTP_LISTEN).");
  if(!simple_mode)
  {
    // work around for create_socket()
    unsigned short temp = local_port; // saving global variables
    boolean temp_bool = local_port_is_present;
    struct in_addr temp_local_IP_address = local_IP_address;
    local_port = (int) (const INTEGER&) send_par.local__portnumber();
    local_port_is_present = TRUE;
    if(send_par.local__hostname().ispresent())
    {
      local_IP_address = get_in_addr((const char *)(const CHARSTRING&)send_par.local__hostname());
    }
    create_socket();
    if (listen(fd, server_backlog) == -1) error("Listen error!");
    map_put_item_server(fd, local_IP_address, local_port);
    log("Listening @ (%s):(%d)", inet_ntoa(local_IP_address), local_port);
    local_port = temp; // restoring global variables
    local_port_is_present = temp_bool;
    local_IP_address = temp_local_IP_address;
    FD_SET(fd, &readfds);
    Install_Handler(&readfds, NULL, NULL, 0.0);
#ifdef SCTP_REPORT_LISTEN_RESULT
    SCTPasp__Types::ASP__SCTP__RESULT asp_sctp_result;
    asp_sctp_result.client__id() = fd;
    asp_sctp_result.error__status() = FALSE;
    asp_sctp_result.error__message() = OMIT_VALUE;
    incoming_message(asp_sctp_result);
#endif
  }
  log("Leaving outgoing_send (ASP_SCTP_LISTEN).");
}


void SCTPasp__PT_PROVIDER::outgoing_send(const SCTPasp__Types::ASP__SCTP__SetSocketOptions& send_par)
{
  log("Calling outgoing_send (ASP_SCTP_SETSOCKETOPTIONS).");
  if(simple_mode)
  {
    if (fd == -1) create_socket(); // checking if there is an open socket
  }
  switch (send_par.get_selection())
  {
    case SCTPasp__Types::ASP__SCTP__SetSocketOptions::ALT_Sctp__init:
    {
      (void) memset(&initmsg, 0, sizeof(struct sctp_initmsg));
      const SCTPasp__Types::SCTP__INIT& init = send_par.Sctp__init();
      initmsg.sinit_num_ostreams = (int) init.sinit__num__ostreams();
      initmsg.sinit_max_instreams = (int) init.sinit__max__instreams();
      initmsg.sinit_max_attempts = (int) init.sinit__max__attempts();
      initmsg.sinit_max_init_timeo = (int) init.sinit__max__init__timeo();
      log("Setting SCTP socket options (initmsg).");
      if (setsockopt(fd, IPPROTO_SCTP, SCTP_INITMSG, &initmsg,
        sizeof(struct sctp_initmsg)) < 0)
      {
         TTCN_warning("Setsockopt error!");
         SCTPasp__Types::ASP__SCTP__RESULT asp_sctp_result;
         asp_sctp_result.client__id() = fd;
         asp_sctp_result.error__status() = TRUE;
         asp_sctp_result.error__message() = strerror(errno);
         incoming_message(asp_sctp_result);
         errno = 0;
      }
      else
      {
        SCTPasp__Types::ASP__SCTP__RESULT asp_sctp_result;
        asp_sctp_result.client__id() = fd;
        asp_sctp_result.error__status() = FALSE;
        asp_sctp_result.error__message() = OMIT_VALUE;
        incoming_message(asp_sctp_result);
      }
      break;
    }
    case SCTPasp__Types::ASP__SCTP__SetSocketOptions::ALT_Sctp__events:
    {
      const SCTPasp__Types::SCTP__EVENTS& event = send_par.Sctp__events();
      events.sctp_data_io_event = (boolean) event.sctp__data__io__event();
      events.sctp_association_event = (boolean) event.sctp__association__event();
      events.sctp_address_event = (boolean) event.sctp__address__event();
      events.sctp_send_failure_event = (boolean) event.sctp__send__failure__event();
      events.sctp_peer_error_event = (boolean) event.sctp__peer__error__event();
      events.sctp_shutdown_event = (boolean) event.sctp__shutdown__event();
      events.sctp_partial_delivery_event = (boolean) event.sctp__partial__delivery__event();
#if defined(LKSCTP_1_0_7) || defined(LKSCTP_1_0_9)
      events.sctp_adaptation_layer_event = (boolean) event.sctp__adaption__layer__event();
#else
      events.sctp_adaption_layer_event = (boolean) event.sctp__adaption__layer__event();
#endif
      break;
    }
    case SCTPasp__Types::ASP__SCTP__SetSocketOptions::ALT_So__linger:
    {
      struct linger so_linger;
      (void) memset(&so_linger, 0, sizeof (so_linger));
      const SCTPasp__Types::SO__LINGER& so = send_par.So__linger();
      so_linger.l_onoff =  (int) so.l__onoff();
      so_linger.l_linger =  (int) so.l__linger();
      // Setting a socket level option
      log("Setting SCTP socket options (so_linger).");
      if (setsockopt(fd, SOL_SOCKET, SCTP_EVENTS, &so_linger, sizeof (so_linger)) < 0)
      {
        TTCN_warning("Setsockopt error!");
        SCTPasp__Types::ASP__SCTP__RESULT asp_sctp_result;
        asp_sctp_result.client__id() = fd;
        asp_sctp_result.error__status() = TRUE;
        asp_sctp_result.error__message() = strerror(errno);
        incoming_message(asp_sctp_result);
        errno = 0;
      }
      else
      {
        SCTPasp__Types::ASP__SCTP__RESULT asp_sctp_result;
        asp_sctp_result.client__id() = fd;
        asp_sctp_result.error__status() = FALSE;
        asp_sctp_result.error__message() = OMIT_VALUE;
        incoming_message(asp_sctp_result);
      }
      break;
    }
    case SCTPasp__Types::ASP__SCTP__SetSocketOptions::ALT_Sctp__rtoinfo:
    {
      struct sctp_rtoinfo sctp_rtoinfo;
      (void) memset(&sctp_rtoinfo, 0, sizeof (sctp_rtoinfo));
      const SCTPasp__Types::SCTP__RTOINFO& rto = send_par.Sctp__rtoinfo();
      int local_fd = (int) rto.client__id();
      sctp_rtoinfo.srto_initial =  (int) rto.srto__initial();
      sctp_rtoinfo.srto_max =  (int) rto.srto__max();
      sctp_rtoinfo.srto_min =  (int) rto.srto__min();
      // Setting a SCTP level socket option
      log("Setting SCTP socket options (sctp_rtoinfo).");
      if (setsockopt(local_fd, IPPROTO_SCTP, SCTP_RTOINFO, &sctp_rtoinfo,
        sizeof (sctp_rtoinfo)) < 0)
      {
        TTCN_warning("Setsockopt error!");
        SCTPasp__Types::ASP__SCTP__RESULT asp_sctp_result;
        asp_sctp_result.client__id() = local_fd;
        asp_sctp_result.error__status() = TRUE;
        asp_sctp_result.error__message() = strerror(errno);
        incoming_message(asp_sctp_result);
        errno = 0;
      }
      else
      {
        SCTPasp__Types::ASP__SCTP__RESULT asp_sctp_result;
        asp_sctp_result.client__id() = local_fd;
        asp_sctp_result.error__status() = FALSE;
        asp_sctp_result.error__message() = OMIT_VALUE;
        incoming_message(asp_sctp_result);
      }
      break;
    }
    default:
      error("Setsocketoptions error: UNBOUND value!");
      break;
  }
  log("Leaving outgoing_send (ASP_SCTP_SETSOCKETOPTIONS).");
}


void SCTPasp__PT_PROVIDER::outgoing_send(const SCTPasp__Types::ASP__SCTP__Close& send_par)
{
  log("Calling outgoing_send (ASP_SCTP_CLOSE).");
  if(!simple_mode)
  {
    if(send_par.client__id().ispresent())
    {
      int local_fd = (int) (const INTEGER&) send_par.client__id();
      log("NORMAL MODE: closing client/server socket (fd = %d).", local_fd);
      map_delete_item_fd(local_fd);
      map_delete_item_fd_server(local_fd);
      FD_CLR(local_fd, &readfds);
      Install_Handler(&readfds, &writefds, NULL, 0.0);
    }
    else
    {   // if OMIT is given then all sockets will be closed
      log("NORMAL MODE: closing all sockets.");
      for(int i=0;i<list_len;i++) map_delete_item(i);
      for(int i=0;i<list_len_server;i++) map_delete_item_server(i);
      FD_ZERO(&readfds);
      FD_ZERO(&writefds);
      Install_Handler(&readfds, &writefds, NULL, 0.0); // ???
    }
  }
  else
  {
    if (server_mode)
    {   // closing the given connection
      if(send_par.client__id().ispresent())
      {
        int local_fd = (int) (const INTEGER&) send_par.client__id();
        log("SERVER MODE: closing client socket (fd = %d).", local_fd);
        map_delete_item_fd(local_fd);
        FD_CLR(local_fd, &readfds);
        Install_Handler(&readfds, NULL, NULL, 0.0);
      }
      else
      {   // if OMIT is given in server mode then all clients will be closed
        log("SERVER MODE: closing all client sockets.");
        for(int i=0;i<list_len;i++) map_delete_item(i);
        FD_ZERO(&readfds);
        FD_SET(fd, &readfds); // leaving only the listening socket in the fdset
        Install_Handler(&readfds, NULL, NULL, 0.0);
      }
    }
    else
    {   // closing the connection to the server
      if ( send_par.client__id().ispresent() )
        error("In client mode the client_id field of ASP_SCTP_Close should be set to OMIT!");
      log("CLIENT MODE: closing socket (fd = %d).", fd);
      map_delete_item_fd(fd);
      FD_CLR(fd, &readfds);
      fd=-1;
      Install_Handler(&readfds, NULL, NULL, 0.0);
    }
  }
  log("Leaving outgoing_send (ASP_SCTP_CLOSE).");
}


void SCTPasp__PT_PROVIDER::outgoing_send(const SCTPasp__Types::ASP__SCTP& send_par)
{
  log("Calling outgoing_send (ASP_SCTP).");
  struct cmsghdr   *cmsg;
  struct sctp_sndrcvinfo  *sri;
  char cbuf[sizeof (*cmsg) + sizeof (*sri)];
  struct msghdr   msg;
  struct iovec   iov;
  const unsigned char *buf;

  buf = (const unsigned char *)send_par.data();
  iov.iov_len = send_par.data().lengthof();

  memset(&msg, 0, sizeof (msg));
  iov.iov_base = (char *)buf;
  msg.msg_iov = &iov;
  msg.msg_iovlen = 1;
  msg.msg_control = cbuf;
  msg.msg_controllen = sizeof (*cmsg) + sizeof (*sri);

  memset(cbuf, 0, sizeof (*cmsg) + sizeof (*sri));
  cmsg = (struct cmsghdr *)cbuf;
  sri = (struct sctp_sndrcvinfo *)(cmsg + 1);

  cmsg->cmsg_len = sizeof (*cmsg) + sizeof (*sri);
  cmsg->cmsg_level = IPPROTO_SCTP;
  cmsg->cmsg_type  = SCTP_SNDRCV;

  sri->sinfo_stream = (int) send_par.sinfo__stream();

  int target;
  if(!simple_mode)
  {
    if (!send_par.client__id().ispresent())
      error("In NORMAL mode the client_id field of ASP_SCTP should be set to a valid value and not to omit!");
    target = (int) (const INTEGER&) send_par.client__id();
    if ( (map_get_item(target)==-1) && (map_get_item_server(target)==-1)) error("Bad client id! %d",target);
  }
  else
  {
    if (server_mode)
    {
      if (!send_par.client__id().ispresent())
        error("In server mode the client_id field of ASP_SCTP should be set to a valid value and not to omit!");
    }
    else // client mode
    {
      if (send_par.client__id().ispresent())
        error("In client mode the client_id field of ASP_SCTP should be set to OMIT!");
    }
    target = fd;
    if (server_mode)
      target = (int) (const INTEGER&) send_par.client__id();
    if (map_get_item(target)==-1) error("Bad client id! %d",target);
  }

  uint32_t ui;
  if (send_par.sinfo__ppid().get_val().is_native() && send_par.sinfo__ppid() > 0)
    ui = (int)send_par.sinfo__ppid();
  else {
    OCTETSTRING os = int2oct(send_par.sinfo__ppid(), 4);
    unsigned char* p = (unsigned char*)&ui;
    *(p++) = os[3].get_octet();
    *(p++) = os[2].get_octet();
    *(p++) = os[1].get_octet();
    *(p++) = os[0].get_octet();
  }
  sri->sinfo_ppid = htonl(ui);

  log("Sending SCTP message to file descriptor %d.", target);
  if (sendmsg(target, &msg, 0) < 0)
  {
    SCTPasp__Types::ASP__SCTP__SENDMSG__ERROR asp_sctp_sendmsg_error;
    if (server_mode) asp_sctp_sendmsg_error.client__id() = target;
    else asp_sctp_sendmsg_error.client__id() = OMIT_VALUE;
    asp_sctp_sendmsg_error.sinfo__stream() = send_par.sinfo__stream();
    asp_sctp_sendmsg_error.sinfo__ppid() = send_par.sinfo__ppid();
    asp_sctp_sendmsg_error.data() = send_par.data();
    incoming_message(asp_sctp_sendmsg_error);
    TTCN_warning("Sendmsg error! Strerror=%s", strerror(errno));

    errno = 0;
  }
  log("Leaving outgoing_send (ASP_SCTP).");
}


SCTPasp__PT_PROVIDER::return_value_t SCTPasp__PT_PROVIDER::getmsg(int fd, struct msghdr *msg)
{
  log("Calling getmsg().");
  int index = map_get_item(fd);
  if ( !fd_map[index].processing_message ) fd_map[index].nr = 0;

  ssize_t value = recvmsg(fd, msg, 0);
  if (value <= 0) // EOF or error
  {
    log("Leaving getmsg(): EOF or error.");
    errno = 0;
    return EOF_OR_ERROR;
  }
  fd_map[index].nr += value;
  log("getmsg(): [%d] bytes received. Receiving buffer now has [%d] bytes.", value, fd_map[index].nr);
  // Whole message is received, return it.
  if (msg->msg_flags & MSG_EOR)
  {
    log("Leaving getmsg(): whole message is received.");
    return WHOLE_MESSAGE_RECEIVED;
  }

  // Maybe we need a bigger buffer, do realloc().
  if (fd_map[index].buflen == fd_map[index].nr)
  {
    log("getmsg(): resizing receiving buffer: [%d] bytes -> [%d] bytes",
      fd_map[index].buflen, (fd_map[index].buflen * 2));
    fd_map[index].buf = Realloc(fd_map[index].buf, fd_map[index].buflen * 2);
    fd_map[index].buflen *= 2;
  }
  log("Leaving getmsg(): part of the message is received.");
  return PARTIAL_RECEIVE;
}


void SCTPasp__PT_PROVIDER::handle_event(void *buf)
{
  union sctp_notification  *snp;
  snp = (sctp_notification *)buf;
  switch (snp->sn_header.sn_type)
  {
    case SCTP_ASSOC_CHANGE:
    {
      log("incoming SCTP_ASSOC_CHANGE event.");
      struct sctp_assoc_change *sac;
      sac = &snp->sn_assoc_change;

// #ifdef LKSCTP_1_0_7
      SCTPasp__Types::SAC__STATE sac_state_ttcn;
      switch(sac->sac_state)
      {
       case SCTP_COMM_UP:
         sac_state_ttcn = SCTPasp__Types::SAC__STATE::SCTP__COMM__UP;
         break;

       case SCTP_COMM_LOST:
         sac_state_ttcn = SCTPasp__Types::SAC__STATE::SCTP__COMM__LOST;
         break;

       case SCTP_RESTART:
         sac_state_ttcn = SCTPasp__Types::SAC__STATE::SCTP__RESTART;
         break;

       case SCTP_SHUTDOWN_COMP:
         sac_state_ttcn = SCTPasp__Types::SAC__STATE::SCTP__SHUTDOWN__COMP;
         break;

       case SCTP_CANT_STR_ASSOC:
         sac_state_ttcn = SCTPasp__Types::SAC__STATE::SCTP__CANT__STR__ASSOC;
         break;

      default:
         sac_state_ttcn = SCTPasp__Types::SAC__STATE::SCTP__UNKNOWN__SAC__STATE;
         TTCN_warning("Unexpected sac_state value received %d", sac->sac_state);
         break;
      }
// #endif

      if(sac->sac_state == SCTP_COMM_LOST)
      {
        if(simple_mode)
        {
          if (!server_mode) fd = -1; // setting closed socket to -1 in client mode (and reconnect mode)
          FD_CLR(receiving_fd, &readfds);
          Install_Handler(&readfds, NULL, NULL, 0.0);
          map_delete_item_fd(receiving_fd);
        }
        else
        {
          FD_CLR(receiving_fd, &readfds);
          Install_Handler(&readfds, NULL, NULL, 0.0);
          map_delete_item_fd(receiving_fd);
          map_delete_item_fd_server(receiving_fd);
        }
      }
      if (events.sctp_association_event) incoming_message(SCTPasp__Types::ASP__SCTP__ASSOC__CHANGE(
                  INTEGER(receiving_fd),
		  sac_state_ttcn
                  ));

      if(simple_mode)
      {
        if (reconnect && (sac->sac_state == SCTP_COMM_LOST) ) forced_reconnect(reconnect_max_attempts);
      }
      break;
    }
    case SCTP_PEER_ADDR_CHANGE:{    
      log("incoming SCTP_PEER_ADDR_CHANGE event.");
      struct sctp_paddr_change *spc;
      spc = &snp->sn_paddr_change;
// #ifdef LKSCTP_1_0_7
      SCTPasp__Types::SPC__STATE spc_state_ttcn;
      switch(spc->spc_state)
      {
       case SCTP_ADDR_AVAILABLE:
         spc_state_ttcn = SCTPasp__Types::SPC__STATE::SCTP__ADDR__AVAILABLE;
         break;

       case SCTP_ADDR_UNREACHABLE:
         spc_state_ttcn = SCTPasp__Types::SPC__STATE::SCTP__ADDR__UNREACHABLE;
         break;

       case SCTP_ADDR_REMOVED:
         spc_state_ttcn = SCTPasp__Types::SPC__STATE::SCTP__ADDR__REMOVED;
         break;

       case SCTP_ADDR_ADDED:
         spc_state_ttcn = SCTPasp__Types::SPC__STATE::SCTP__ADDR__ADDED;
         break;

       case SCTP_ADDR_MADE_PRIM:
         spc_state_ttcn = SCTPasp__Types::SPC__STATE::SCTP__ADDR__MADE__PRIM;
         break;
#if  defined(LKSCTP_1_0_7) || defined(LKSCTP_1_0_9)
       case SCTP_ADDR_CONFIRMED:
         spc_state_ttcn = SCTPasp__Types::SPC__STATE::SCTP__ADDR__CONFIRMED;
         break;
#endif
      default:
         spc_state_ttcn = SCTPasp__Types::SPC__STATE::SCTP__UNKNOWN__SPC__STATE;
         TTCN_warning("Unexpected spc_state value received %d", spc->spc_state);
         break;
      }
// #endif
      if (events.sctp_address_event) incoming_message(SCTPasp__Types::ASP__SCTP__PEER__ADDR__CHANGE(
                  INTEGER(receiving_fd),
		              spc_state_ttcn
                  ));
      break;
      }
    case SCTP_REMOTE_ERROR:
      log("incoming SCTP_REMOTE_ERROR event.");
      //struct sctp_remote_error *sre;
      //sre = &snp->sn_remote_error;
      if (events.sctp_peer_error_event) incoming_message(SCTPasp__Types::ASP__SCTP__REMOTE__ERROR(INTEGER(receiving_fd)));
      break;
    case SCTP_SEND_FAILED:
      log("incoming SCTP_SEND_FAILED event.");
      //struct sctp_send_failed *ssf;
      //ssf = &snp->sn_send_failed;
      if (events.sctp_send_failure_event) incoming_message(SCTPasp__Types::ASP__SCTP__SEND__FAILED(INTEGER(receiving_fd)));
      break;
    case SCTP_SHUTDOWN_EVENT:
      log("incoming SCTP_SHUTDOWN_EVENT event.");
      //struct sctp_shutdown_event *sse;
      //sse = &snp->sn_shutdown_event;
      if (events.sctp_shutdown_event) incoming_message(SCTPasp__Types::ASP__SCTP__SHUTDOWN__EVENT(INTEGER(receiving_fd)));
      break;
#if  defined(LKSCTP_1_0_7) || defined(LKSCTP_1_0_9)
    case SCTP_ADAPTATION_INDICATION:
      log("incoming SCTP_ADAPTION_INDICATION event.");
      //struct sctp_adaptation_event *sai;
      //sai = &snp->sn_adaptation_event;
      if (events.sctp_adaptation_layer_event) incoming_message(SCTPasp__Types::ASP__SCTP__ADAPTION__INDICATION(INTEGER(receiving_fd)));
      break;
#else
    case SCTP_ADAPTION_INDICATION:
      log("incoming SCTP_ADAPTION_INDICATION event.");
      //struct sctp_adaption_event *sai;
      //sai = &snp->sn_adaption_event;
      if (events.sctp_adaption_layer_event) incoming_message(SCTPasp__Types::ASP__SCTP__ADAPTION__INDICATION(INTEGER(receiving_fd)));
      break;
#endif
    case SCTP_PARTIAL_DELIVERY_EVENT:
      log("incoming SCTP_PARTIAL_DELIVERY_EVENT event.");
      //struct sctp_pdapi_event *pdapi;
      //pdapi = &snp->sn_pdapi_event;
      if (events.sctp_partial_delivery_event) incoming_message(SCTPasp__Types::ASP__SCTP__PARTIAL__DELIVERY__EVENT(INTEGER(receiving_fd)));
      break;
    default:
      TTCN_warning("Unknown notification type!");
      break;
  }
}


void SCTPasp__PT_PROVIDER::log(const char *fmt, ...)
{
  if(debug)
  {
    va_list ap;
    va_start(ap, fmt);
    TTCN_Logger::begin_event(TTCN_DEBUG);
    TTCN_Logger::log_event("SCTPasp Test Port (%s): ", get_name());
    TTCN_Logger::log_event_va_list(fmt, ap);
    TTCN_Logger::end_event();
    va_end(ap);

  }
}


void SCTPasp__PT_PROVIDER::error(const char *fmt, ...)
{
  va_list ap;
  va_start(ap, fmt);
  TTCN_Logger::begin_event(TTCN_ERROR);
  TTCN_Logger::log_event("SCTPasp Test Port (%s): ", get_name());
  TTCN_Logger::log_event_va_list(fmt, ap);
  TTCN_Logger::end_event();
  va_end(ap);
  TTCN_error("Fatal error in SCTPasp Test Port %s (see above).", get_name());
}


void SCTPasp__PT_PROVIDER::forced_reconnect(int attempts)
{
  struct sockaddr_in  sin;
  sin.sin_family = AF_INET;
  sin.sin_port = htons(peer_port);
  sin.sin_addr.s_addr = peer_IP_address.s_addr;
  log("[reconnect] Connecting to (%s):(%d)", inet_ntoa(peer_IP_address), peer_port);
  unsigned int sleep_interval = 1;
  int i;
  for(i = 0; i < attempts; i++)
  {
    create_socket();
    if (connect(fd, (struct sockaddr *)&sin, sizeof (sin)) == -1)
    {
      close(fd);
      fd = -1;
      TTCN_warning("Connect error!");
      errno = 0;
      if( ((i % 2 ) == 0) && (i != 0)) sleep_interval *= 2;
      sleep(sleep_interval);

    }
    else
    {
      map_put_item(fd);
      setNonBlocking(fd);
      FD_SET(fd, &readfds);
      Install_Handler(&readfds, NULL, NULL, 0.0);
      log("[reconnect] Connection successfully established to (%s):(%d)", inet_ntoa(peer_IP_address), peer_port);
      break;
    }
  }
  if (i == attempts) error("Forced reconnect failed! Remote end is unreachable!");

}


void SCTPasp__PT_PROVIDER::map_put_item(int fd)
{
  int i=0;
  while((i<list_len) && !fd_map[i].erased) i++; // searching for the free item
  if(i==list_len)
  {  // list is full add new elemnts
    list_len+=MAP_LENGTH;
    fd_map=(fd_map_item *)Realloc(fd_map,(list_len)*sizeof(fd_map_item));
    for(int k=i;k<list_len;k++)
    {  // init new elements
      fd_map[k].fd=-1;
      fd_map[k].erased=TRUE;
      fd_map[k].einprogress=FALSE;
      fd_map[k].buf=NULL;
      fd_map[k].buflen=0;
      fd_map[k].processing_message=FALSE;
      fd_map[k].nr=0;
      fd_map[k].sin.sin_family=AF_INET;
      fd_map[k].sin.sin_port=0;
      fd_map[k].sin.sin_addr.s_addr=0;
    }
  }
  fd_map[i].fd=fd;        // adding new connection
  fd_map[i].erased=FALSE;
}


int SCTPasp__PT_PROVIDER::map_get_item(int fd)
{
  for(int i = 0; i < list_len; i++)
    if( !(fd_map[i].erased) && (fd_map[i].fd == fd) ) return i;
  return(-1);
}


void SCTPasp__PT_PROVIDER::map_delete_item_fd(int fd)
{
  if(fd!=-1)
    for(int i = 0; i < list_len; i++)
      if( fd_map[i].fd == fd )
      {
        map_delete_item(i);
        break;
      }
}


void SCTPasp__PT_PROVIDER::map_delete_item(int index)
{
  if((index>=list_len) || (index<0)) error("map_delete_item: index out of range (0-%d): %d",list_len-1,index);

  if(fd_map[index].fd!=-1) close(fd_map[index].fd);
  fd_map[index].fd=-1;
  fd_map[index].erased=TRUE;
  fd_map[index].einprogress=FALSE;
  if(fd_map[index].buf) Free(fd_map[index].buf);
  fd_map[index].buf=NULL;
  fd_map[index].buflen=0;
  fd_map[index].processing_message=FALSE;
  fd_map[index].nr=0;
  fd_map[index].sin.sin_family=AF_INET;
  fd_map[index].sin.sin_port=0;
  fd_map[index].sin.sin_addr.s_addr=0;
}


void SCTPasp__PT_PROVIDER::map_put_item_server(int fd, struct in_addr local_IP_address, unsigned short local_port)
{
  int i=0;
  while((i<list_len_server) && !fd_map_server[i].erased) i++; // searching for the free item
  if(i==list_len_server)
  {  // list is full add new elemnts
    list_len_server+=MAP_LENGTH;
    fd_map_server=(fd_map_server_item *)Realloc(fd_map_server,(list_len_server)*sizeof(fd_map_server_item));
    for(int k=i;k<list_len_server;k++)
    {  // init new elements
      fd_map_server[k].fd=-1;
      fd_map_server[k].erased=TRUE;
      fd_map_server[k].local_IP_address.s_addr = INADDR_ANY;
      fd_map_server[k].local_port = 0;
    }
  }
  fd_map_server[i].fd=fd;        // adding new connection
  fd_map_server[i].erased=FALSE;
  fd_map_server[i].local_IP_address = local_IP_address;
  fd_map_server[i].local_port = local_port;

}


int SCTPasp__PT_PROVIDER::map_get_item_server(int fd)
{
  for(int i = 0; i < list_len_server; i++)
    if( !(fd_map_server[i].erased) && (fd_map_server[i].fd == fd) ) return i;
  return(-1);
}


void SCTPasp__PT_PROVIDER::map_delete_item_fd_server(int fd)
{
  if(fd!=-1)
    for(int i = 0; i < list_len_server; i++)
      if( fd_map_server[i].fd == fd )
      {
        map_delete_item_server(i);
        break;
      }
}


void SCTPasp__PT_PROVIDER::map_delete_item_server(int index)
{
  if((index>=list_len_server) || (index<0)) error("map_delete_item: index out of range (0-%d): %d",list_len_server-1,index);

  if(fd_map_server[index].fd!=-1) close(fd_map_server[index].fd);
  fd_map_server[index].fd=-1;
  fd_map_server[index].erased=TRUE;
  fd_map_server[index].local_IP_address.s_addr = INADDR_ANY;
  fd_map_server[index].local_port = 0;
}


void SCTPasp__PT_PROVIDER::create_socket()
{
  struct sockaddr_in  sin;

  log("Creating SCTP socket.");
  if ((fd = socket(AF_INET, SOCK_STREAM, IPPROTO_SCTP)) == -1)
    error("Socket error: cannot create socket!");

  if ( local_port_is_present ) {
    sin.sin_family = AF_INET;
    sin.sin_port = htons(local_port);
    sin.sin_addr.s_addr = local_IP_address.s_addr;
    log("Binding SCTP socket: bind address (%s):(%d)",
      inet_ntoa(local_IP_address),local_port);
    if (bind(fd, (struct sockaddr *)&sin, sizeof (sin)) == -1)
    {
      close(fd);
      fd = -1;
      error("Bind error!");
    }
  }

  log("Setting SCTP socket options (initmsg).");
  if (setsockopt(fd, IPPROTO_SCTP, SCTP_INITMSG, &initmsg,
    sizeof(struct sctp_initmsg)) < 0)
  {
    TTCN_warning("Setsockopt error!");
    errno = 0;
  }

  log("Setting SCTP socket options (events).");
  if (setsockopt(fd, IPPROTO_SCTP, SCTP_EVENTS, &events, sizeof (events)) < 0)
  {
    TTCN_warning("Setsockopt error!");
    errno = 0;
  }
}


in_addr SCTPasp__PT_PROVIDER::get_in_addr(const char *hostname)
{
  struct hostent *h;
  if ((h=gethostbyname(hostname)) == NULL)
    error("Gethostbyname error!");
  if(h->h_addr == NULL) error("Gethostbyname error! h->h_addr is NULL!");
  return *((struct in_addr *)h->h_addr);
}

void SCTPasp__PT_PROVIDER::setNonBlocking(int fd)
{
  int flags = fcntl(fd, F_GETFL);
  flags |= O_NONBLOCK;
  int result = fcntl(fd, F_SETFL, flags);
  if (result==-1) error("SCTPasp__PT::setNonBlocking(): Fcntl() error!");
}


}
