///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// Copyright Test Competence Center (TCC) ETH 2003                           //
//                                                                           //
// The copyright to the computer program(s) herein is the property of TCC.   //
// The program(s) may be used and/or copied only with the written permission //
// of TCC or in accordance with the terms and conditions stipulated in the   //
// agreement/contract under which the program(s) has been supplied.          //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////
//
// File: ROHC_EncDec.cc
// Description: ROHC
// Rev: R3A01
// Prodnr: CNL 113 426
// Updated: 2007-02-07
// Contact: http://ttcn.ericsson.se
//

/* Based on RFC 3095, July 2001 */

#include "IP_Types.hh"
#include "UDP_Types.hh"
#include "RTP_Types.hh"
#include "ROHC_Types.hh"

/* ============ Constants ============ */

/* Encoding and decoding is performed using static buffers */
#define MAX_PACKET_SIZE 1500
#define HEXDUMP_BUFFER_SIZE 1500

/* ROHC mode constants */
#define ROHC_mode_C 0
#define ROHC_mode_U 1
#define ROHC_mode_O 2
#define ROHC_mode_R 3

/* Inner and outer IP level numbers */
#define Inner_IP 1
#define Outer_IP 0

/* ============ Utilities ============ */

#define isIR(val) (((val) & 0xFE) == 0xFC)
#define isIRDYN(val) ((val) == 0xF8)
#define isUOR2(val) (((val) & 0xE0) == 0xC0)

/* Shifts the passed bit value from the position 0 to position N. */
#define ShiftUpBit(val, position) ((val) << (position)) & (0x01 << (position))

/* Shifts the passed bit value from the position N to position 0. */
#define ShiftDownBit(val, position) (((val) >> (position)) & 0x01)

/* Logs an event, when the 'value' field in the type 'typename' is not
  implemented. 'typename' must be provided in '"'s */
#define Log_not_implemented_union_field(typename, value) \
TTCN_error("Encoding of the selected union field (%u) in type " \
           typename " is not implemented", (value));

/* Prints a hexdump of the buffer 'buf' in length 'length'. */
#define Log_hexdump(buf, length) \
TTCN_logger.log(TTCN_DEBUG, "Buffer (length = %d):\n%s", \
                (length), debug_hexdump((buf), debug_buf, (length)));

/* Logs the name of the function on entering */
#define Log_function_name_on_enter() \
TTCN_logger.log(TTCN_DEBUG, "Entering %s", __FUNCTION__);

/* Logs the name of the function on leaving */
#define Log_function_name_on_leave() \
TTCN_logger.log(TTCN_DEBUG, "Leaving %s", __FUNCTION__);

/* Logs an object */
#define Log_object(o) \
TTCN_logger.begin_event(TTCN_DEBUG); \
(o).log(); \
TTCN_logger.end_event(); \

//**********************************************************
//   NAMESPACE
//**********************************************************

namespace ROHC__Types {

using namespace General__Types;
using namespace IP__Types;
using namespace UDP__Types;
using namespace RTP__Types;

/* Umbrella type for parsing an incoming ROHC packet */
typedef struct 
{
  ROHC__context & context;
  int cid;
  int compressed_ah_data_len[2];
  bool ipx_gre_cksum_present[2];
} t_dat;

/* TODO: this is not a thread-safe thing to do. For FT, it does not
  matter, but the buffer should be allocated dynamically. */
static char debug_buf[HEXDUMP_BUFFER_SIZE];

static char *
debug_hexdump(const unsigned char *src_buf, char *dest_buf, unsigned int len)
{
  unsigned int offset = 0;
  char val;

  memset(dest_buf, 0, HEXDUMP_BUFFER_SIZE);

  if (src_buf == NULL || len == 0)
    return dest_buf;

  for (unsigned int i = 0; i < len; i++)
  {
    if ((i % 16) == 0 && i > 0)
    {
      dest_buf[offset] = 0x0A;  /* newline after each line */
      offset++;
    }
    if ((i % 8) == 0)
    {
      dest_buf[offset] = 0x20;  /* space after 8 characters */
      offset++;
    }
    val = (src_buf[i] >> 4) & 0x0F;     /* First digit */
    val = (val < 10) ? (val + 48) : (val + 65 - 10);
    dest_buf[offset] = val;
    offset++;
    val = src_buf[i] & 0x0F;    /* Second digit */
    val = (val < 10) ? (val + 48) : (val + 65 - 10);
    dest_buf[offset] += val;
    offset++;
    dest_buf[offset] = 0x20;    /* space after the value */
    offset++;
  }

  return dest_buf;
}

/* ============ Consistency check routines ============ */
void
Check_field_value(char *text, int urherenow, int wannabe)
{
  if (wannabe != urherenow)
  {
    TTCN_logger.log(TTCN_WARNING, "Field '%s' must be %u, %u is given",
                    text, wannabe, urherenow);
  }
}

void
Check_field_presence(char *text, bool flag, bool presence)
{
  if (flag && !presence)
  {
    TTCN_logger.log(TTCN_WARNING, "Field '%s' must be present, when "
                    "its presence flag is set", text);
  }
  else if (!flag && presence)
  {
    TTCN_logger.log(TTCN_WARNING, "Field '%s' must not be present, when "
                    "its presence flag is unset", text);
  }
}

void
Check_feedback_code__size(const INT3b & code, const OPTIONAL < LIN1 > &size)
{
  if (size.ispresent() && code > 0)
  {
    TTCN_logger.log(TTCN_WARNING,
                    "Code field must be 0, when size field is specified");
  }
}

void
Check_feedback_optlen__optsize(const INT4b & len,
                               const OPTIONAL < OCTETSTRING > &data)
{
  if (data.ispresent() && (data().lengthof() != len))
  {
    int flen = len;
    int fdatalen = data().lengthof();
    TTCN_logger.log(TTCN_WARNING,
                    "Feedback option length (%u) is different from "
                    "the length of feedback option data (%u)", flen, fdatalen);
  }
  else if (!data.ispresent() && len > 0)
  {
    int flen = len;
    TTCN_logger.log(TTCN_WARNING,
                    "No feedback option data, but feedback option length "
                    "is not 0 (%u)", flen);
  }
}

void
Check_ps_bit__xi_format(const BIT1 & psbit,
                        const OPTIONAL < XI__list > &xilist)
{
  if (!xilist.ispresent())
    return;
  if (xilist().get_selection() == XI__list::ALT_xi__item8 && (*psbit) == 0)
  {
    TTCN_logger.log(TTCN_WARNING, "PS bit is 0, 4-bit indices must be used");
  }
  else if (xilist().get_selection() == XI__list::ALT_xi__item4 && (*psbit) == 1)
  {
    TTCN_logger.log(TTCN_WARNING, "PS bit is 1, 8-bit indices must be used");
  }
}

INTEGER
ComputeCRC(const unsigned char *buf, int length, int crclen)
{
  int crcval, poly, shiftmask;

  Log_function_name_on_enter();
  TTCN_logger.log(TTCN_DEBUG, "Calculating CRC-%d over:", crclen);
  Log_hexdump(buf, length);

  switch (crclen)
  {
  case 3:
    poly = 0x06;
    crcval = 0x07;
    shiftmask = 0x03;
    break;

  case 7:
    poly = 0x79;
    crcval = 0x7F;
    shiftmask = 0x3F;
    break;

  case 8:
    poly = 0xE0;
    crcval = 0xFF;
    shiftmask = 0x7F;
    break;

  case 32:
    poly = 0xEDB88320;
    crcval = 0xFFFFFFFF;
    shiftmask = 0x7FFFFFFF;
    break;

  default:
    poly = 0;
    TTCN_error("Unknown CRC length(%d)\n", crclen);
    break;
  }

  if (poly == 0)
    return 0;

  for (int i = 0; i < length; i++)
  {
    for (int j = 0; j < 8; j++)
    {
      if ((crcval & 0x01) ^ (ShiftDownBit(buf[i], j)))
      {
        crcval = (crcval >> 1) & shiftmask;
        crcval ^= poly;
      }
      else
      {
        crcval = (crcval >> 1) & shiftmask;
      }
    }
  }
  
  /* CRC-32 in ROHC needs to be negated at the end */
  if (crclen == 32)
  {
    crcval ^= 0xffffffff;
  }

  Log_function_name_on_leave();

  unsigned int ui = (unsigned int)crcval;
  char sbuf[16];
  sprintf(sbuf, "%u", ui);
  
  return INTEGER(sbuf);
}

/* ============ Encoding functions for base types ============ */

/** Returns the index of the innermost IP level, based on the number of
  IP levels in the context. */
int
getInnermostIPidx(t_dat *dat)
{
  // More than 1 levels: the innermost is always the index = 1
  if (dat->context.ip__ctx().size_of() > 1)
    return Inner_IP;
  else
    // The index is the one and only 0.
    return Outer_IP;
}

/* Returns the index of the inner or outer IP level in the context, based on the 
  number of the IP levels in the context. If the return value is negative,
  it indicates that the outer IP level does not exists. Negative value is 
  never returned, if the ip_level is the Inner IP level. */
int
getIPidx(t_dat *dat, int ip_level)
{
  if (ip_level == Outer_IP)
  {
    if (dat->context.ip__ctx().size_of() > 1)
      return Outer_IP;
    else 
      return -1;
  }
  else if (ip_level == Inner_IP)
    return getInnermostIPidx(dat);
  else 
    return ip_level;
}

/** Initializes the context of the specified IP level. */
void
initIPcontext(t_dat *dat, int ip_level, int ip_version)
{
  unsigned char ctemp = 0;
  IP__context & field = dat->context.ip__ctx()[ip_level];
  
  Log_function_name_on_enter();
  
  field.version() = INTEGER(ip_version);
  if (! field.rnd__bit().is_bound()) field.rnd__bit() = BOOLEAN(ctemp);
  if (! field.ah__present().is_bound()) field.ah__present() = BOOLEAN(ctemp);
  if (! field.gre__present().is_bound()) field.gre__present() = BOOLEAN(ctemp);
  if (! field.esp__present().is_bound()) field.esp__present() = BOOLEAN(ctemp);
  if (! field.ah__data__len().is_bound()) 
    field.ah__data__len() = INTEGER(ctemp);
  if (! field.gre__cksum__present().is_bound()) 
    field.gre__cksum__present() = BOOLEAN(ctemp);
  
  Log_object(dat->context);
  
  Log_function_name_on_leave();
}

/** Initializes the UDP context. */
void
initUDPcontext(t_dat *dat)
{
  unsigned char ctemp = 1;
  UDP__context & field = dat->context.udp__ctx();
  
  if (! field.udp__cksum().is_bound()) field.udp__cksum() = BOOLEAN(ctemp);
}

/** Initializes the specified CID's context. */
void
initCIDcontext(t_dat *dat)
{
  unsigned char ctemp = 0;
  ROHC__context & field = dat->context;
  
  if (! field.mode().is_bound()) field.mode() = INTEGER(cg__ROHC__mode__U);
  if (! field.profile().is_bound()) field.profile() = INTEGER(ctemp);
  if (! field.pkt().is_bound()) field.pkt() = Packet__type(Packet__type::IR);
  if (! field.ip__ctx().is_bound()) initIPcontext(dat, 0, 4);
  initUDPcontext(dat);
}

/** Initializes the umbrella type before the ROHC packet parsing starts. */
t_dat
initTDAT(ROHC__config & config, int cid)
{
  t_dat dat = { config.context()[cid], cid, {0, 0}, {false, false} };
  
  // Expect AH auth data and GRE checksum after the compressed header
  // by default. If the uncompressed form is used, these values will be
  // set to 0 or false during parsing.
  dat.compressed_ah_data_len[0] = 
    config.context()[cid].ip__ctx()[0].ah__data__len();
  dat.ipx_gre_cksum_present[0] = 
    config.context()[cid].ip__ctx()[0].gre__cksum__present();
  if (config.context()[cid].ip__ctx().size_of() > 1)
  {
    dat.compressed_ah_data_len[1] = 
      config.context()[cid].ip__ctx()[1].ah__data__len();
    dat.ipx_gre_cksum_present[1] = 
      config.context()[cid].ip__ctx()[1].gre__cksum__present();
  }
  return dat;  
}

/** Returns, whether there is any compressed IPv4 level, 
  where the RND bit is 0. */
bool
isAnyIPv4RNDunset(t_dat *dat)
{
  for (int i = 0; i < 2; i++)
  {
    int ip_idx = getIPidx(dat, i);
    
    if (ip_idx >= 0) // IP level exists
    {
      if (! dat->context.ip__ctx()[ip_idx].rnd__bit())
        return true;
    }
  }
  return false;
}

/* Encodes an OCTETSTING value */
int
Set_octetstring(unsigned char *buf, const OCTETSTRING & val)
{
  memcpy(&(buf[0]), val, val.lengthof());
  return val.lengthof();
}

/* Encodes a LIN2_BO_LAST value */
int
Set_LIN2_BO_LAST(unsigned char *buf, const LIN2__BO__LAST & val)
{
  buf[0] = (val >> 8) & 0xFF;
  buf[1] = val & 0xFF;
  return 2;
}

/* Encodes a LIN4_BO_LAST value */
int
Set_LIN4_BO_LAST(unsigned char *buf, const LIN4__BO__LAST & val)
{
  buf[0] = (val >> 24) & 0xFF;
  buf[1] = (val >> 16) & 0xFF;
  buf[2] = (val >> 8) & 0xFF;
  buf[3] = val & 0xFF;
  return 4;
}

/* Encodes a LIN1 value */
int
Set_LIN1(unsigned char *buf, const LIN1 & val)
{
  buf[0] = val & 0xFF;
  return 1;
}

/* Returns the length of the SDVL encoded CID value 'val'. If the value
  is invalid, exits via the ttcn_error(). */
int
SDVL_encoded_CID_length(int val)
{
  if (val <= 0x7F)
    return 1;
  else if (val <= 0x3FFF)
    return 2;
  else if (val <= 0x1FFFFF)
    return 3;
  else if (val <= 0x1FFFFFFF)
    return 4;
  else
    TTCN_error("Value %u is too big for the SDVL encoder "
               "(at most 29 bits can be used)", val);

  return 0;
}

int
Set_SDVL_field(unsigned char *buf, const INTEGER & val, int encoded_length)
{
  int len = 0;

  Log_function_name_on_enter();
  
  if ((val <= 0x7F && encoded_length == 0) || encoded_length == 1)
  {
    buf[len] = val & 0x7F;
    len += 1;
  }
  else if ((val <= 0x3FFF && encoded_length == 0) || encoded_length == 2)
  {
    buf[len] = 0x80 | ((val >> 8) & 0x7F);
    len += 1;
    buf[len] = val & 0xFF;
    len += 1;
  }
  else if ((val <= 0x1FFFFF && encoded_length == 0) || encoded_length == 3)
  {
    buf[len] = 0xC0 | ((val >> 16) & 0x3F);
    len += 1;
    buf[len] = (val >> 8) & 0xFF;
    len += 1;
    buf[len] = val & 0xFF;
    len += 1;
  }
  else if ((val <= 0x1FFFFFFF && encoded_length == 0) || encoded_length == 4)
  {
    buf[len] = 0xE0 | ((val >> 24) & 0x1F);
    len += 1;
    buf[len] = (val >> 16) & 0xFF;
    len += 1;
    buf[len] = (val >> 8) & 0xFF;
    len += 1;
    buf[len] = val & 0xFF;
    len += 1;
  }
  else
  {
    int tmp = val;
    TTCN_error("Value %u (specified length is %u) is too big for the "
               "SDVL encoder (at most 29 bits can be used)", 
               tmp, encoded_length);
  }

  Log_hexdump(buf, len);
  Log_function_name_on_leave();

  return len;
}

/* ============ Encoding wrappers for optional base types ============ */

/* Encodes an optional OCTETSTING value */
int
Set_octetstring_opt(unsigned char *buf, const OPTIONAL < OCTETSTRING > &val)
{
  if (!val.ispresent())
    return 0;
  return Set_octetstring(buf, val());
}

/* Encodes an optional LIN2_BO_LAST value */
int
Set_LIN2_BO_LAST_opt(unsigned char *buf,
                     const OPTIONAL < LIN2__BO__LAST > &val)
{
  if (!val.ispresent())
    return 0;
  return Set_LIN2_BO_LAST(buf, val());
}

/* Encodes an optional LIN4_BO_LAST value */
int
Set_LIN4_BO_LAST_opt(unsigned char *buf,
                     const OPTIONAL < LIN4__BO__LAST > &val)
{
  if (!val.ispresent())
    return 0;
  return Set_LIN4_BO_LAST(buf, val());
}

/* Encodes an optional LIN1 value */
int
Set_LIN1_opt(unsigned char *buf, const OPTIONAL < LIN1 > &val)
{
  if (!val.ispresent())
    return 0;
  return Set_LIN1(buf, val());
}

int
Set_SDVL_field_opt(unsigned char *buf, 
                   const OPTIONAL < INTEGER > &val, int length)
{
  int len = 0;

  Log_function_name_on_enter();

  if (!val.ispresent())
    return 0;
  len += Set_SDVL_field(buf, val(), length);
  Log_function_name_on_leave();
  return len;
}

/* ============ Encoding functions for common ROHC types ============ */

int
Set_CID(unsigned char *buf, const INTEGER & cid, BOOLEAN const &large_cid)
{
  int len = 0;

  Log_function_name_on_enter();

  if (large_cid == false)
  {
    if (cid > 0)
    {
      buf[len] = 0xE0 + (cid & 0x0F);
      len += 1;
    }
  }
  else
    len += Set_SDVL_field(&buf[len], cid, 0);

  Log_hexdump(buf, len);
  Log_function_name_on_leave();

  return len;
}

/* CSRC and IP extension header item list encoding */
int
Set_Item_list_opt(unsigned char *buf, const OPTIONAL < Item__list > &item_list)
{
  int len = 0;

  Log_function_name_on_enter();

  if (!item_list.ispresent())
  {
    Log_function_name_on_leave();
    return len;
  }

  /* Item list is an IP extension header list */
  if (item_list().get_selection() == Item__list::ALT_ip__item__list)
  {
    IP__Item__list ip_item_list = item_list().ip__item__list();

    for (int num = 0; num < ip_item_list.size_of(); num++)
    {
      int start_pos = len;

      switch (ip_item_list[num].get_selection())
      {
      case Item::ALT_ipv6__ext__item:
        {
          IPv6__ext__item & item = ip_item_list[num].ipv6__ext__item();

          len += Set_LIN1(&buf[len], item.nexthead());
          /* Length of the header in 8-octet units, not including 
             the first 8 octets. */
          if (item.hdr__ext__len() == 0)
          {
            if ((item.data().lengthof() + 2) % 8 != 0)
            {
              TTCN_error("Invalid length (%u) of the IPv6 extension header",
                         item.data().lengthof());
            }
            buf[len] = (item.data().lengthof() + 2) / 8 - 1;
            len += 1;
          }
          else
            len += Set_LIN1(&buf[len], item.hdr__ext__len());
          len += Set_octetstring(&buf[len], item.data());
          break;
        }
      case Item::ALT_mine__item:
        {
          MINE__item & item = ip_item_list[num].mine__item();

          Check_field_value("MINE protocol", item.protocol(), c__ip__proto__mine);
          Check_field_value("MINE reserved", *item.reserved(), 0);
          
          len += Set_LIN1(&buf[len], item.protocol());
          buf[len] = (*item.s__bit()) << 7;
          buf[len] += *item.reserved();
          len += 1;
          len += Set_LIN2_BO_LAST(&buf[len], item.cksum());
          len += Set_octetstring(&buf[len], item.dstaddr());
          len += Set_octetstring_opt(&buf[len], item.srcaddr());
          break;
        }
      case Item::ALT_ah__item:
        {
          AH__item & item = ip_item_list[num].ah__item();

          Check_field_value("AH nexthead", item.nexthead(), c__ip__proto__ah);
          len += Set_LIN1(&buf[len], item.nexthead());

          /* RFC 2402: This 8-bit field specifies the length of AH in 
             32-bit words (4-byte units), minus "2". */
          if (item.payload__len() == 0)
          {
            if (item.auth__data().ispresent())
              buf[len] = (item.auth__data()().lengthof() / 4) + 3 - 2;
            else
              buf[len] = 1;
            len += 1;
          }
          else
            len += Set_LIN1(&buf[len], item.payload__len());

          Check_field_value("AH reserved", item.reserved(), 0);
          len += Set_LIN2_BO_LAST(&buf[len], item.reserved());
          len += Set_LIN4_BO_LAST(&buf[len], item.spi());
          len += Set_LIN4_BO_LAST(&buf[len], item.sn());
          len += Set_octetstring_opt(&buf[len], item.auth__data());
          break;
        }
      case Item::ALT_esp__item:
        {
          ESP__item & item = ip_item_list[num].esp__item();

          Check_field_value("ESP nexthead", item.nexthead(), c__ip__proto__esp);
          len += Set_LIN1(&buf[len], item.nexthead());
          len += Set_LIN4_BO_LAST(&buf[len], item.spi());
          len += Set_LIN4_BO_LAST(&buf[len], item.sn());
          break;
        }
      case Item::ALT_gre__item:
        {
          GRE__item & item = ip_item_list[num].gre__item();

          Check_field_value("GRE nexthead", item.nexthead(), c__ip__proto__gre2);
          len += Set_LIN1(&buf[len], item.nexthead());
          buf[len] = (*item.C__bit()) << 7;
          Check_field_value("GRE reserved1", (int) *item.reserved__1(), 0);
          buf[len] += (*item.reserved__1()) << 6;
          buf[len] += (*item.K__bit()) << 5;
          buf[len] += (*item.S__bit()) << 4;
          Check_field_value("GRE reserved2", (int) *item.reserved__2(), 0);
          buf[len] += (*item.reserved__2()) << 3;
          buf[len] += *item.version();
          len += 1;
          Check_field_presence("checksum", *item.C__bit() == 1,
                               item.cksum().ispresent());
          Check_field_presence("key", *item.K__bit() == 1,
                               item.key().ispresent());
          Check_field_presence("sn", *item.S__bit() == 1,
                               item.sn().ispresent());
          len += Set_LIN2_BO_LAST_opt(&buf[len], item.cksum());
          len += Set_LIN4_BO_LAST_opt(&buf[len], item.key());
          len += Set_LIN4_BO_LAST_opt(&buf[len], item.sn());
          break;
        }
      default:
        {
          Log_not_implemented_union_field("Item",
                                          ip_item_list[num].get_selection());
          break;
        }
      }
      TTCN_logger.log(TTCN_DEBUG, "%uth item:", num);
      Log_hexdump(&(buf[start_pos]), len - start_pos);
    }
  }
  else if (item_list().get_selection() == Item__list::ALT_csrc__item__list)
  {
    /* Item list is CSRC item list */
    for (int num = 0; num < item_list().csrc__item__list().size_of(); num++)
    {
      int start_pos = len;

      len += Set_octetstring( &buf[len],
                             item_list().csrc__item__list()[num] );
      TTCN_logger.log(TTCN_DEBUG, "%uth item:", num);
      Log_hexdump(&(buf[start_pos]), len - start_pos);
    }
  }
  else if (item_list().get_selection() == Item__list::ALT_raw__data)
  {
    /* Item list is a raw octetstring */
    for (int num = 0; num < item_list().csrc__item__list().size_of(); num++)
    {
      int start_pos = len;

      len += Set_octetstring(&buf[len], item_list().raw__data()[num] );
      TTCN_logger.log(TTCN_DEBUG, "%uth item:", num);
      Log_hexdump(&(buf[start_pos]), len - start_pos);
    }
  }
  else
  {
    Log_not_implemented_union_field("Item_list", item_list().get_selection());
  }

  Log_hexdump(buf, len);
  Log_function_name_on_leave();

  return len;
}

int
Set_XI_list_opt(unsigned char *buf, const OPTIONAL < XI__list > &xilist,
                const OPTIONAL < BITSTRING > &padding, int length)
{
  int len = 0;
  int halfbyte = 0;

  Log_function_name_on_enter();

  if (!xilist.ispresent())
  {
    Log_function_name_on_leave();
    return len;
  }
  
  switch (xilist().get_selection())
  {
  case XI__list::ALT_xi__item4:
    {
      if (length != xilist().xi__item4().size_of())
        TTCN_logger.log(TTCN_WARNING,
                        "Number of indices != value of CC field");
      for (int num = 0; num < xilist().xi__item4().size_of(); num++)
      {
        if (halfbyte)
        {
          buf[len] += ((*xilist().xi__item4()[num].x__ind()) << 3) & 0x08;
          buf[len] += xilist().xi__item4()[num].index() & 0x07;
          len += 1;
        }
        else
        {
          buf[len] = ((*xilist().xi__item4()[num].x__ind()) << 7) & 0x80;
          buf[len] += (xilist().xi__item4()[num].index() << 4) & 0x70;
        }
        halfbyte = 1 - halfbyte;        // Invert the value (0 -> 1 or 1 -> 0)
      }
      break;
    }

  case XI__list::ALT_xi__item8:
    {
      if (length != xilist().xi__item8().size_of())
        TTCN_logger.log(TTCN_WARNING,
                        "Number of indices != value of CC field");
      for (int num = 0; num < xilist().xi__item8().size_of(); num++)
      {
        buf[len] = ((*xilist().xi__item8()[num].x__ind()) << 7) & 0x80;
        buf[len] += xilist().xi__item8()[num].index() & 0x7F;
        len += 1;
      }
      break;
    }

  default:
    Log_not_implemented_union_field("XI_list", xilist().get_selection());
    break;
  }

  if (padding.ispresent() && halfbyte == 0)
  {
    TTCN_logger.log(TTCN_WARNING, "Specified padding is ignored");
  }
  else if (!padding.ispresent() && halfbyte)
  {
    TTCN_logger.log(TTCN_WARNING,
                    "Padding not specified, padding bits left uninitialized");
  }
  else if (padding.ispresent())
  {
    Check_field_value("Index padding", *padding(), 0);
    buf[len] += (*padding()) & 0x0F;
    len += 1;
  }

  Log_hexdump(buf, len);
  Log_function_name_on_leave();

  return len;
}

int
Set_Encoding_Type_0(unsigned char *buf, const Enc__Type__0 & enct0)
{
  int len = 0;

  Log_function_name_on_enter();

  Check_field_presence("gen_id", *enct0.gp__bit() == 1,
                       enct0.gen__id().ispresent());

  Check_field_value("Encoding type", *enct0.et(), 0);
  buf[len] = ((*enct0.et()) << 6) & 0xC0;
  buf[len] += ShiftUpBit(*enct0.gp__bit(), 5);
  buf[len] += ShiftUpBit(*enct0.ps__bit(), 4);
  buf[len] += enct0.cc() & 0x0F;
  len += 1;
  len += Set_LIN1_opt(&buf[len], enct0.gen__id());
  Check_ps_bit__xi_format(enct0.ps__bit(), enct0.xi__list());
  len += Set_XI_list_opt(&buf[len], enct0.xi__list(), enct0.padding(),
                         enct0.cc());
  len += Set_Item_list_opt(&buf[len], enct0.item__list());

  Log_hexdump(buf, len);
  Log_function_name_on_leave();

  return len;
}


int
Set_Encoding_Type_1(unsigned char *buf, const Enc__Type__1 & enct1)
{
  int len = 0;
  int n = 0;

  Log_function_name_on_enter();

  Check_field_presence("gen_id", *enct1.gp__bit() == 1,
                       enct1.gen__id().ispresent());

  Check_field_value("Encoding type", *enct1.et(), 1);
  buf[len] = ((*enct1.et()) << 6) & 0xC0;
  buf[len] += ShiftUpBit(*enct1.gp__bit(), 5);
  buf[len] += ShiftUpBit(*enct1.ps__bit(), 4);
  buf[len] += enct1.xi1() & 0x0F;
  len += 1;
  len += Set_LIN1_opt(&buf[len], enct1.gen__id());
  len += Set_LIN1_opt(&buf[len], enct1.ref__id());
  len += Set_octetstring(&buf[len], enct1.insbitmask());

  // Count the number of 1's in the bitmask
  for (int i = 0; i < enct1.insbitmask().lengthof(); ++i)
  {
    if (enct1.insbitmask() & int2oct(i + 1, enct1.insbitmask().lengthof()))
      ++n;
  }

  Check_ps_bit__xi_format(enct1.ps__bit(), enct1.xi__list());
  len += Set_XI_list_opt(&buf[len], enct1.xi__list(), enct1.padding(), n);
  len += Set_Item_list_opt(&buf[len], enct1.item__list());

  Log_hexdump(buf, len);
  Log_function_name_on_leave();

  return len;
}


int
Set_Encoding_Type_2(unsigned char *buf, const Enc__Type__2 & enct2)
{
  int len = 0;

  Check_field_presence("gen_id", *enct2.gp__bit() == 1,
                       enct2.gen__id().ispresent());

  Check_field_value("Encoding type", *enct2.et(), 2);
  buf[len] = ((*enct2.et()) << 6) & 0xC0;
  buf[len] += ShiftUpBit(*enct2.gp__bit(), 5);
  buf[len] += ShiftUpBit(*enct2.res(), 4);
  buf[len] += enct2.count() & 0x0F;
  len += 1;
  len += Set_LIN1_opt(&buf[len], enct2.gen__id());
  len += Set_LIN1_opt(&buf[len], enct2.ref__id());
  len += Set_octetstring(&buf[len], enct2.rembitmask());

  Log_hexdump(buf, len);
  Log_function_name_on_leave();

  return len;
}


int
Set_Encoding_Type_3(unsigned char *buf, const Enc__Type__3 & enct3)
{
  int len = 0;
  int n = 0;

  Check_field_presence("gen_id", *enct3.gp__bit() == 1,
                       enct3.gen__id().ispresent());

  Check_field_value("Encoding type", *enct3.et(), 3);
  buf[len] = ((*enct3.et()) << 6) & 0xC0;
  buf[len] += ShiftUpBit(*enct3.gp__bit(), 5);
  buf[len] += ShiftUpBit(*enct3.ps__bit(), 4);
  buf[len] += enct3.xi1() & 0x0F;
  len += 1;
  len += Set_LIN1_opt(&buf[len], enct3.gen__id());
  len += Set_LIN1_opt(&buf[len], enct3.ref__id());
  len += Set_octetstring(&buf[len], enct3.rembitmask());
  len += Set_octetstring(&buf[len], enct3.insbitmask());

  // Count the number of 1's in the bitmask
  for (int i = 0; i < enct3.insbitmask().lengthof(); ++i)
  {
    if (enct3.insbitmask() & int2oct(i + 1, enct3.insbitmask().lengthof()))
      ++n;
  }
  Check_ps_bit__xi_format(enct3.ps__bit(), enct3.xi__list());
  len += Set_XI_list_opt(&buf[len], enct3.xi__list(), enct3.padding(), n);
  len += Set_Item_list_opt(&buf[len], enct3.item__list());

  Log_hexdump(buf, len);
  Log_function_name_on_leave();

  return len;
}

int
Set_Compr_head_list_opt(unsigned char *buf,
                        const OPTIONAL < Compr__head__list > &chl)
{
  int len = 0;

  Log_function_name_on_enter();

  if (!chl.ispresent())
  {
    Log_function_name_on_leave();
    return len;
  }
  if (chl().get_selection() == Compr__head__list::ALT_enctype0)
    len += Set_Encoding_Type_0(&buf[len], chl().enctype0());
  else if (chl().get_selection() == Compr__head__list::ALT_enctype1)
    len += Set_Encoding_Type_1(&buf[len], chl().enctype1());
  else if (chl().get_selection() == Compr__head__list::ALT_enctype2)
    len += Set_Encoding_Type_2(&buf[len], chl().enctype2());
  else if (chl().get_selection() == Compr__head__list::ALT_enctype3)
    len += Set_Encoding_Type_3(&buf[len], chl().enctype3());
  else
  {
    Log_not_implemented_union_field("Compr_head_list", chl().get_selection());
  }

  Log_hexdump(buf, len);
  Log_function_name_on_leave();

  return len;
}

int
Set_AEGSeqnum_opt(unsigned char *buf, const OPTIONAL < AEGSeqnum > &oseqn)
{
  int len = 0;

  Log_function_name_on_enter();

  if (!oseqn.ispresent())
  {
    Log_function_name_on_leave();
    return len;
  }
  if (oseqn().get_selection() == AEGSeqnum::ALT_short__form)
  {
    Check_field_value("form-bit", (int) *oseqn().short__form().ind(), 0);
    buf[len] = ShiftUpBit(*oseqn().short__form().ind(), 7);
    buf[len] += oseqn().short__form().lsb__of__seqnum() & 0x7F;
    len += 1;
  }
  else if (oseqn().get_selection() == AEGSeqnum::ALT_long__form)
  {
    Check_field_value("form-bit", (int) *oseqn().long__form().ind(), 1);
    buf[len] = ShiftUpBit(*oseqn().long__form().ind(), 7);
    buf[len] += (oseqn().long__form().lsb__of__seqnum() >> 24) & 0x7F;
    len += 1;

    buf[len] = (oseqn().long__form().lsb__of__seqnum() >> 16) & 0xFF;
    len += 1;

    buf[len] = (oseqn().long__form().lsb__of__seqnum() >> 8) & 0xFF;
    len += 1;

    buf[len] = oseqn().long__form().lsb__of__seqnum() & 0xFF;
    len += 1;
  }
  else
    Log_not_implemented_union_field("AEGSeqnum", oseqn().get_selection());

  Log_hexdump(buf, len);
  Log_function_name_on_leave();

  return len;
}

int
Set_IPv4_static_chain(unsigned char *buf, const IPv4__Static & chain)
{
  int len = 0;

  Log_function_name_on_enter();

  Check_field_value("IPv4 version", chain.version(), 4);
  buf[len] = (chain.version() << 4) & 0xF0;
  Check_field_value("IPv4 reserved (static chain)", chain.reserved(), 0);
  buf[len] += chain.reserved() & 0x0F;
  len += 1;
  len += Set_LIN1(&buf[len], chain.proto());
  len += Set_octetstring(&buf[len], chain.srcaddr());
  len += Set_octetstring(&buf[len], chain.dstaddr());

  Log_hexdump(buf, len);
  Log_function_name_on_leave();

  return len;
}

int
Set_IPv6_static_chain(unsigned char *buf, const IPv6__Static & chain)
{
  int len = 0;

  Log_function_name_on_enter();

  Check_field_value("IPv6 version", chain.version(), 6);
  buf[len] = (chain.version() << 4) & 0xF0;
  buf[len] += (chain.flowlabel() >> 16) & 0x0F;
  len += 1;

  buf[len] = (chain.flowlabel() >> 8) & 0xFF;
  len += 1;
  buf[len] = chain.flowlabel() & 0xFF;
  len += 1;

  len += Set_LIN1(&buf[len], chain.nexthead());
  len += Set_octetstring(&buf[len], chain.srcaddr());
  len += Set_octetstring(&buf[len], chain.dstaddr());

  Log_hexdump(buf, len);
  Log_function_name_on_leave();

  return len;
}

int
Set_UDP_static_chain(unsigned char *buf, const UDP__Static & chain)
{
  int len = 0;

  Log_function_name_on_enter();

  len += Set_LIN2_BO_LAST(&buf[len], chain.srcport());
  len += Set_LIN2_BO_LAST(&buf[len], chain.dstport());

  Log_hexdump(buf, len);
  Log_function_name_on_leave();

  return len;
}

int
Set_RTP_static_chain(unsigned char *buf, const RTP__Static & chain)
{
  int len = 0;

  Log_function_name_on_enter();

  len += Set_octetstring(&buf[len], chain.ssrc());

  Log_hexdump(buf, len);
  Log_function_name_on_leave();

  return len;
}

int
Set_RTP_dynamic_chain(unsigned char *buf, const RTP__Dynamic & chain)
{
  int len = 0;

  Log_function_name_on_enter();

  Check_field_value("RTP version", chain.vfield(), 2);
  buf[len] = (chain.vfield() << 6) & 0xC0;
  buf[len] += ShiftUpBit(*chain.pbit(), 5);
  buf[len] += ShiftUpBit(*chain.rxbit(), 4);
  buf[len] += chain.ccfield() & 0x0F;
  len += 1;
  buf[len] = ShiftUpBit(*chain.mbit(), 7);
  buf[len] += chain.ptfield() & 0x7F;
  len += 1;
  len += Set_LIN2_BO_LAST(&buf[len], chain.rtpseqnum());
  len += Set_LIN4_BO_LAST(&buf[len], chain.rtpts());
  len += Set_Encoding_Type_0(&buf[len], chain.gencsrclist());
  Check_field_presence("RTP rx", *chain.rxbit() == 1,
                       chain.rx__field().ispresent());
  if (chain.rx__field().ispresent())
  {
    Check_field_value("RX reserved", (int) *chain.rx__field()().reserved(), 0);
    buf[len] = ((*chain.rx__field()().reserved()) << 5) & 0xE0;
    buf[len] += ShiftUpBit(*chain.rx__field()().xbit(), 4);
    buf[len] += (chain.rx__field()().mode() << 2) & 0x0C;
    buf[len] += ShiftUpBit(*chain.rx__field()().tisbit(), 1);
    buf[len] += ShiftUpBit(*chain.rx__field()().tssbit(), 0);
    len += 1;
  }
  Check_field_presence("TS Stride",
                       chain.rx__field().ispresent() &&
                       (*chain.rx__field()().tssbit()) == 1,
                       chain.ts__stride().ispresent());
  Check_field_presence("TIME Stride",
                       chain.rx__field().ispresent() &&
                       (*chain.rx__field()().tisbit()) == 1,
                       chain.time__stride().ispresent());
  len += Set_SDVL_field_opt(&buf[len], chain.ts__stride(), 0);
  len += Set_SDVL_field_opt(&buf[len], chain.time__stride(), 0);

  Log_hexdump(buf, len);
  Log_function_name_on_leave();

  return len;
}

int
Set_RTP_flags_fields_opt(unsigned char *buf,
                         const OPTIONAL < RTP__flags__fields > &field)
{
  int len = 0;

  Log_function_name_on_enter();

  if (!field.ispresent())
  {
    Log_function_name_on_leave();
    return len;
  }
  buf[len] = (field().mode() << 6) & 0xC0;
  buf[len] += ShiftUpBit(*field().r__pt(), 5);
  buf[len] += ShiftUpBit(*field().m__bit(), 4);
  buf[len] += ShiftUpBit(*field().r__x(), 3);
  buf[len] += ShiftUpBit(*field().csrc(), 2);
  buf[len] += ShiftUpBit(*field().tss(), 1);
  buf[len] += ShiftUpBit(*field().tis(), 0);
  len += 1;

  Check_field_presence("RTP flags",
                       *field().r__pt() == 1, field().flags().ispresent());
  if (field().flags().ispresent())
  {
    buf[len] = (((*field().flags()().r__p()) << 7) & 0x80) +
      (field().flags()().rtp__pt() & 0x7F);
    len += 1;
  }
  Check_field_presence("Compressed CSRC list",
                       *field().csrc() == 1, field().csrc__list().ispresent());
  len += Set_Compr_head_list_opt(&buf[len], field().csrc__list());
  Check_field_presence("TS_STRIDE",
                       *field().tss() == 1, field().ts__stride().ispresent());
  len += Set_SDVL_field_opt(&buf[len], field().ts__stride(), 0);
  Check_field_presence("TIME_STRIDE",
                       *field().tis() == 1,
                       field().time__stride().ispresent());
  len += Set_SDVL_field_opt(&buf[len], field().time__stride(), 0);

  Log_hexdump(buf, len);
  Log_function_name_on_leave();

  return len;
}

int
Set_IPv4_dynamic_chain(unsigned char *buf, const IPv4__Dynamic & chain)
{
  int len = 0;

  Log_function_name_on_enter();

  len += Set_LIN1(&buf[len], chain.tos());
  len += Set_LIN1(&buf[len], chain.ttl());
  len += Set_LIN2_BO_LAST(&buf[len], chain.identification());

  buf[len] = ShiftUpBit(*chain.df__bit(), 7);
  buf[len] += ShiftUpBit(*chain.rnd__bit(), 6);
  buf[len] += ShiftUpBit(*chain.nbo__bit(), 5);
  Check_field_value("IPv4 reserved (dynamic chain)", *chain.reserved(), 0);
  buf[len] += (*chain.reserved()) & 0x1F;
  len += 1;
  len += Set_Encoding_Type_0(&buf[len], chain.genextheadlist());

  Log_hexdump(buf, len);
  Log_function_name_on_leave();

  return len;
}

int
Set_IPv6_dynamic_chain(unsigned char *buf, const IPv6__Dynamic & chain)
{
  int len = 0;

  Log_function_name_on_enter();

  len += Set_LIN1(&buf[len], chain.trafficclass());
  len += Set_LIN1(&buf[len], chain.hoplimit());
  len += Set_Encoding_Type_0(&buf[len], chain.genextheadlist());

  Log_hexdump(buf, len);
  Log_function_name_on_leave();

  return len;
}

/* ============ Encoding functions for ROHC types ============ */
int
Set_feedback(unsigned char *buf, const Feedback__u & feedback,
             BOOLEAN const &large_cid)
{
  int len = 0, crcpos = 0, crcstart = 0;

  Log_function_name_on_enter();

  Check_field_value("feedback type", (int) *feedback.feedback__type(), 30);
  Check_feedback_code__size(feedback.code(), feedback.size());
  buf[len] = ((*feedback.feedback__type()) << 3) & 0xF8;
  buf[len] += feedback.code() & 0x07;
  len += 1;

  len += Set_LIN1_opt(&buf[len], feedback.size());

  crcstart = len;

  len += Set_CID(&buf[len], feedback.feedback__data().cid(), large_cid);

  switch (feedback.feedback__data().feedback__type().get_selection())
  {
  case Feedback__type::ALT_feedback1:
    {
      Feedback1 fback1 =
        feedback.feedback__data().feedback__type().feedback1();

      len += Set_octetstring(&buf[len], fback1);
      break;
    }

  case Feedback__type::ALT_feedback2:
    {
      Feedback2 fback2 =
        feedback.feedback__data().feedback__type().feedback2();

      buf[len] = (fback2.acktype() << 6) & 0xC0;
      buf[len] += (fback2.mode() << 4) & 0x30;
      buf[len] += (fback2.sn() >> 8) & 0x0F;
      len += 1;

      buf[len] = fback2.sn() & 0xFF;
      len += 1;

      if (fback2.feedback__opts().ispresent())
      {
        Feedback__opts & fbopts = fback2.feedback__opts();

        for (int num = 0; num < fbopts.size_of(); num++)
        {
          buf[len] = (fbopts[num].opt__type() << 4) & 0xF0;
          Check_feedback_optlen__optsize(fbopts[num].opt__len(),
                                         fbopts[num].opt__data());
          buf[len] += fbopts[num].opt__len() & 0x0F;
          len += 1;

          len += Set_octetstring_opt(&buf[len], fbopts[num].opt__data());
          if (fbopts[num].opt__data().ispresent())
          {
            if ((fbopts[num].opt__len() == 1)
                && (fbopts[num].opt__type() == 1) && (buf[len - 1] == 0))
              crcpos = len - 1;
          }
        }
      }
      break;
    }

  default:
    break;
  }

  if (feedback.size().ispresent() && feedback.size() == 0)
  {
    buf[1] = (len - 2) & 0xFF;
  }
  if ((!feedback.size().ispresent()) && feedback.code() == 0)
  {
    buf[0] = (buf[0] & 0xF8) + ((len - 1) & 0x07);
  }

  if ((crcpos > crcstart) && (buf[crcpos] == 0))
    buf[crcpos] = ComputeCRC(&(buf[crcstart]), len - crcstart, 8);

  Log_hexdump(buf, len);
  Log_function_name_on_leave();

  return len;
}

int
Set_ROHC_Feedback_opt(unsigned char *buf,
                      const OPTIONAL < Feedback > &feedback,
                      BOOLEAN const &large_cid)
{
  int len = 0;

  Log_function_name_on_enter();

  if (!feedback.ispresent())
  {
    Log_function_name_on_leave();
    return len;
  }
  for (int num = 0; num < feedback().size_of(); num++)
  {
    len += Set_feedback(&buf[len], feedback()[num], large_cid);
  }

  Log_hexdump(buf, len);
  Log_function_name_on_leave();
  return len;
}

/* Inserts CID infomation into the buffer by moving memory areas */
int
Set_CID_for_packets(unsigned char *buf, ROHC__config const &config,
                    int cid, int buflen)
{
  int len = 0;

  Log_function_name_on_enter();

  if (config.large__cid() == false)
  {                             /* Small CID: first octet is the CID */
    if (cid > 0)
    {
      // Overlapping memory areas, use memmove
      memmove(&(buf[1]), &(buf[0]), buflen);
      buf[0] = 0xE0 + (cid & 0x0F);
      len += 1;
    }
  }
  else
  {                             /* Large CID: CID is placed after the first octet */
    int cidlen = SDVL_encoded_CID_length(cid);

    // Overlapping memory areas, use memmove
    memmove(&(buf[1 + cidlen]), &(buf[1]), buflen - 1);
    len += Set_SDVL_field(&(buf[1]), cid, 0);
  }

  Log_hexdump(buf, len);
  Log_function_name_on_leave();

  return len;
}

int
Set_ROHC_Header_Profile0(unsigned char *buf,
                         const Profile0__headers & hdr,
                         ROHC__config const &config)
{
  int len = 0, crcpos = 0, cidlen, cid;

  Log_function_name_on_enter();

  switch (hdr.get_selection())
  {
  case Profile0__headers::ALT_ir:
    {
      cid = hdr.ir().cid();
      Check_field_value("packet type", (int) *hdr.ir().type__ind(), 126);
      buf[len] = ((*hdr.ir().type__ind()) << 1) & 0xFE;
      buf[len] += (*hdr.ir().d()) & 0x01;
      len += 1;
      Check_field_value("profile", hdr.ir().profile(), 0);
      len += Set_LIN1(&buf[len], hdr.ir().profile());
      crcpos = len;
      len += Set_LIN1(&buf[len], hdr.ir().crc());
      len += Set_octetstring(&buf[len], hdr.ir().orig__packet());
      break;
    }

  case Profile0__headers::ALT_normal:
    {
      cid = hdr.normal().cid();
      len += Set_octetstring(&buf[len], hdr.normal().orig__packet());
      break;
    }

  default:
    Log_not_implemented_union_field("Profile0_headers", hdr.get_selection());
    break;
  }

  cidlen = Set_CID_for_packets(buf, config, cid, len);
  crcpos += cidlen;
  len += cidlen;

  if (hdr.get_selection() == Profile0__headers::ALT_ir && hdr.ir().crc() == 0)
  {
    buf[crcpos] = ComputeCRC(&(buf[0]), crcpos, 8);
  }

  Log_hexdump(buf, len);
  Log_function_name_on_leave();
  return len;
}

int
Set_Static_Chain_Profile1(unsigned char *buf, const Static__Chain & chain)
{
  int len = 0;

  Log_function_name_on_enter();

  for (int num = 0; num < chain.size_of(); num++)
  {
    switch (chain[num].get_selection())
    {
    case Static__Chain__u::ALT_ipv4__stat:
      len += Set_IPv4_static_chain(&buf[len], chain[num].ipv4__stat());
      break;
    case Static__Chain__u::ALT_ipv6__stat:
      len += Set_IPv6_static_chain(&buf[len], chain[num].ipv6__stat());
      break;
    case Static__Chain__u::ALT_udp__stat:
      len += Set_UDP_static_chain(&buf[len], chain[num].udp__stat());
      break;
    case Static__Chain__u::ALT_rtp__stat:
      len += Set_RTP_static_chain(&buf[len], chain[num].rtp__stat());
      break;
    default:
      Log_not_implemented_union_field
        ("Static_Chain_u", chain[num].get_selection());
      break;
    }
  }

  Log_hexdump(buf, len);
  Log_function_name_on_leave();

  return len;
}

int
Set_Dynamic_Chain_Profile1(unsigned char *buf, const Dynamic__Chain & dynch)
{
  int len = 0;

  Log_function_name_on_enter();

  for (int num = 0; num < dynch.size_of(); num++)
  {
    switch (dynch[num].get_selection())
    {
    case Dynamic__Chain__u::ALT_ipv4__dyn:
      len += Set_IPv4_dynamic_chain(&buf[len], dynch[num].ipv4__dyn());
      break;
    case Dynamic__Chain__u::ALT_ipv6__dyn:
      len += Set_IPv6_dynamic_chain(&buf[len], dynch[num].ipv6__dyn());
      break;
    case Dynamic__Chain__u::ALT_udp__dyn:
      len += Set_LIN2_BO_LAST(&buf[len], dynch[num].udp__dyn().cksum());
      break;
    case Dynamic__Chain__u::ALT_rtp__dyn:
      len += Set_RTP_dynamic_chain(&buf[len], dynch[num].rtp__dyn());
      break;
    default:
      Log_not_implemented_union_field
        ("Dynamic_Chain_u", dynch[num].get_selection());
      break;
    }
  }

  Log_hexdump(buf, len);
  Log_function_name_on_leave();

  return len;
}

int
Set_Profile1_ROHC_Header_ir(unsigned char *buf,
                            const Profile1__IR__header & hdr)
{
  int len = 0;

  Log_function_name_on_enter();

  Check_field_value("packet type", (int) *hdr.type__ind(), 126);
  buf[len] = ((*hdr.type__ind()) << 1) & 0xFE;
  Check_field_presence("dynamic chain", *hdr.d() == 1,
                       hdr.dyn__chain().ispresent());
  buf[len] += ShiftUpBit(*hdr.d(), 0);
  len += 1;
  Check_field_value("profile", hdr.profile(), 1);
  len += Set_LIN1(&buf[len], hdr.profile());
  len += Set_LIN1(&buf[len], hdr.crc());
  len += Set_Static_Chain_Profile1(&buf[len], hdr.stat__chain());
  if (hdr.dyn__chain().ispresent())
    len += Set_Dynamic_Chain_Profile1(&buf[len], hdr.dyn__chain());

  Log_hexdump(buf, len);
  Log_function_name_on_leave();

  return len;
}

int
Set_Profile1_ROHC_Header_irdyn(unsigned char *buf,
                               const Profile1__IR__DYN__header & hdr)
{
  int len = 0;

  Log_function_name_on_enter();

  Check_field_value("packet type", (int) *hdr.type__ind(), 248);
  buf[len] = *hdr.type__ind();
  len += 1;
  Check_field_value("profile", hdr.profile(), 1);
  len += Set_LIN1(&buf[len], hdr.profile());
  len += Set_LIN1(&buf[len], hdr.crc());
  len += Set_Dynamic_Chain_Profile1(&buf[len], hdr.dyn__chain());

  Log_hexdump(buf, len);
  Log_function_name_on_leave();

  return len;
}

int
Set_Profile1_ROHC_Header_r0(unsigned char *buf,
                            const Profile1__R__0__header & hdr)
{
  Log_function_name_on_enter();

  Check_field_value("packet type", (int) *hdr.type__ind(), 0);
  buf[0] = ((*hdr.type__ind()) << 6) & 0xC0;
  buf[0] += hdr.sn() & 0x3F;

  Log_hexdump(buf, 1);
  Log_function_name_on_leave();

  return 1;
}

int
Set_Profile1_ROHC_Header_r0crc(unsigned char *buf,
                               const Profile1__R__0__CRC__header & hdr)
{
  int len = 0;

  Log_function_name_on_enter();

  Check_field_value("packet type", (int) *hdr.type__ind(), 1);
  buf[len] = ((*hdr.type__ind()) << 6) & 0xC0;
  buf[len] += (hdr.sn() >> 1) & 0x3F;
  len += 1;
  buf[len] = (hdr.sn() << 7) & 0x80;
  buf[len] += hdr.crc() & 0x7F;
  len += 1;

  Log_hexdump(buf, len);
  Log_function_name_on_leave();

  return len;
}

int
Set_Profile1_ROHC_Header_uo0(unsigned char *buf,
                             const Profile1__UO__0__header & hdr)
{
  int len = 0;

  Log_function_name_on_enter();

  Check_field_value("packet type", (int) *hdr.type__ind(), 0);
  buf[len] = ((*hdr.type__ind()) << 7) & 0x80;
  buf[len] += (hdr.sn() << 3) & 0x78;
  buf[len] += hdr.crc() & 0x07;
  len += 1;

  Log_hexdump(buf, len);
  Log_function_name_on_leave();

  return len;
}

int
Set_Profile1_ROHC_Header_r1(unsigned char *buf,
                            const Profile1__R__1__header & hdr)
{
  int len = 0;

  Log_function_name_on_enter();

  Check_field_value("packet type", (int) *hdr.type__ind(), 2);
  buf[len] = ((*hdr.type__ind()) << 6) & 0xC0;
  buf[len] += hdr.sn() & 0x3F;
  len += 1;
  buf[len] = ShiftUpBit(*hdr.m__bit(), 7);
  buf[len] += ShiftUpBit(*hdr.x__bit(), 6);
  buf[len] += hdr.ts() & 0x3F;
  len += 1;

  Log_hexdump(buf, len);
  Log_function_name_on_leave();

  return len;
}

int
Set_Profile1_ROHC_Header_r1id(unsigned char *buf,
                              const Profile1__R__1__ID__header & hdr)
{
  int len = 0;

  Log_function_name_on_enter();

  Check_field_value("packet type", (int) *hdr.type__ind(), 2);
  buf[len] = ((*hdr.type__ind()) << 6) & 0xC0;
  buf[len] += hdr.sn() & 0x3F;
  len += 1;
  buf[len] = ShiftUpBit(*hdr.m__bit(), 7);
  buf[len] += ShiftUpBit(*hdr.x__bit(), 6);
  Check_field_value("T-bit", (int) *hdr.t__bit(), 0);
  buf[len] += ShiftUpBit(*hdr.t__bit(), 5);
  buf[len] += hdr.ip__id() & 0x1F;
  len += 1;

  Log_hexdump(buf, len);
  Log_function_name_on_leave();

  return len;
}

int
Set_Profile1_ROHC_Header_r1ts(unsigned char *buf,
                              const Profile1__R__1__TS__header & hdr)
{
  int len = 0;

  Log_function_name_on_enter();

  Check_field_value("packet type", (int) *hdr.type__ind(), 2);
  buf[len] = ((*hdr.type__ind()) << 6) & 0xC0;
  buf[len] += hdr.sn() & 0x3F;
  len += 1;
  buf[len] = ShiftUpBit(*hdr.m__bit(), 7);
  buf[len] += ShiftUpBit(*hdr.x__bit(), 6);
  Check_field_value("T-bit", (int) *hdr.t__bit(), 1);
  buf[len] += ShiftUpBit(*hdr.t__bit(), 5);
  buf[len] += hdr.ts() & 0x1F;
  len += 1;

  Log_hexdump(buf, len);
  Log_function_name_on_leave();

  return len;
}

int
Set_Profile1_ROHC_Header_uo1(unsigned char *buf,
                             const Profile1__UO__1__header & hdr)
{
  int len = 0;

  Log_function_name_on_enter();

  Check_field_value("packet type", (int) *hdr.type__ind(), 2);
  buf[len] = ((*hdr.type__ind()) << 6) & 0xC0;
  buf[len] += hdr.ts() & 0x3F;
  len += 1;
  buf[len] = ShiftUpBit(*hdr.m__bit(), 7);
  buf[len] += (hdr.sn() << 3) & 0x78;
  buf[len] += hdr.crc() & 0x07;
  len += 1;

  Log_hexdump(buf, len);
  Log_function_name_on_leave();

  return len;
}

int
Set_Profile1_ROHC_Header_uo1id(unsigned char *buf,
                               const Profile1__UO__1__ID__header & hdr)
{
  int len = 0;

  Log_function_name_on_enter();

  Check_field_value("packet type", (int) *hdr.type__ind(), 2);
  buf[len] = ((*hdr.type__ind()) << 6) & 0xC0;
  Check_field_value("T-bit", (int) *hdr.t__bit(), 0);
  buf[len] += ShiftUpBit(*hdr.t__bit(), 5);
  buf[len] += hdr.ip__id() & 0x1F;
  len += 1;
  buf[len] = ShiftUpBit(*hdr.x__bit(), 7);
  buf[len] += (hdr.sn() << 3) & 0x78;
  buf[len] += hdr.crc() & 0x07;
  len += 1;

  Log_hexdump(buf, len);
  Log_function_name_on_leave();

  return len;
}

int
Set_Profile1_ROHC_Header_uo1ts(unsigned char *buf,
                               const Profile1__UO__1__TS__header & hdr)
{
  int len = 0;

  Log_function_name_on_enter();

  Check_field_value("packet type", (int) *hdr.type__ind(), 2);
  buf[len] = ((*hdr.type__ind()) << 6) & 0xC0;
  Check_field_value("T-bit", (int) *hdr.t__bit(), 1);
  buf[len] += ShiftUpBit(*hdr.t__bit(), 5);
  buf[len] += hdr.ts() & 0x1F;
  len += 1;
  buf[len] = ShiftUpBit(*hdr.m__bit(), 7);
  buf[len] += (hdr.sn() << 3) & 0x78;
  buf[len] += hdr.crc() & 0x07;
  len += 1;

  Log_hexdump(buf, len);
  Log_function_name_on_leave();

  return len;
}

int
Set_Profile1_ROHC_Header_uor2(unsigned char *buf,
                              const Profile1__UOR__2__header & hdr)
{
  int len = 0;

  Log_function_name_on_enter();

  Check_field_value("packet type", (int) *hdr.type__ind(), 6);
  buf[len] = ((*hdr.type__ind()) << 5) & 0xE0;
  buf[len] += (hdr.ts() >> 1) & 0x1F;
  len += 1;
  buf[len] = (hdr.ts() << 7) & 0x80;
  buf[len] += ShiftUpBit(*hdr.m__bit(), 6);
  buf[len] += hdr.sn() & 0x3F;
  len += 1;
  buf[len] = ShiftUpBit(*hdr.x__bit(), 7);
  buf[len] += hdr.crc() & 0x7F;
  len += 1;

  Log_hexdump(buf, len);
  Log_function_name_on_leave();

  return len;
}

int
Set_Profile1_ROHC_Header_uor2id(unsigned char *buf,
                                const Profile1__UOR__2__ID__header & hdr)
{
  int len = 0;

  Log_function_name_on_enter();

  Check_field_value("packet type", (int) *hdr.type__ind(), 6);
  buf[len] = ((*hdr.type__ind()) << 5) & 0xE0;
  buf[len] += hdr.ip__id() & 0x1F;
  len += 1;
  Check_field_value("T-bit", (int) *hdr.t__bit(), 0);
  buf[len] = ShiftUpBit(*hdr.t__bit(), 7);
  buf[len] += ShiftUpBit(*hdr.m__bit(), 6);
  buf[len] += hdr.sn() & 0x3F;
  len += 1;
  buf[len] = ShiftUpBit(*hdr.x__bit(), 7);
  buf[len] += hdr.crc() & 0x7F;
  len += 1;

  Log_hexdump(buf, len);
  Log_function_name_on_leave();

  return len;
}

int
Set_Profile1_ROHC_Header_uor2ts(unsigned char *buf,
                                const Profile1__UOR__2__TS__header & hdr)
{
  int len = 0;

  Log_function_name_on_enter();

  Check_field_value("packet type", (int) *hdr.type__ind(), 6);
  Log_hexdump(buf, len);
  buf[len] = ((*hdr.type__ind()) << 5) & 0xE0;
  buf[len] += hdr.ts() & 0x1F;
  len += 1;
  Check_field_value("T-bit", (int) *hdr.t__bit(), 1);
  buf[len] = ShiftUpBit(*hdr.t__bit(), 7);
  buf[len] += ShiftUpBit(*hdr.m__bit(), 6);
  buf[len] += hdr.sn() & 0x3F;
  len += 1;
  buf[len] = ShiftUpBit(*hdr.x__bit(), 7);
  buf[len] += hdr.crc() & 0x7F;
  len += 1;

  Log_hexdump(buf, len);
  Log_function_name_on_leave();

  return len;
}

int
Set_Profile1_Inner_IP_flags_opt(unsigned char *buf,
                                const OPTIONAL < Inner__IP__flags > &field)
{
  int len = 0;

  Log_function_name_on_enter();

  if (!field.ispresent())
  {
    Log_function_name_on_leave();
    return len;
  }
  buf[len] = ShiftUpBit(*field().tos(), 7);
  buf[len] += ShiftUpBit(*field().ttl(), 6);
  buf[len] += ShiftUpBit(*field().df(), 5);
  buf[len] += ShiftUpBit(*field().pr(), 4);
  buf[len] += ShiftUpBit(*field().ipx(), 3);
  buf[len] += ShiftUpBit(*field().nbo(), 2);
  buf[len] += ShiftUpBit(*field().rnd__bit(), 1);
  buf[len] += ShiftUpBit(*field().ip2__bit(), 0);
  len += 1;

  Log_hexdump(buf, len);
  Log_function_name_on_leave();

  return len;
}

int
Set_Profile1_Outer_IP_flags_opt(unsigned char *buf,
                                const OPTIONAL < Outer__IP__flags > &field)
{
  int len = 0;

  Log_function_name_on_enter();

  if (!field.ispresent())
  {
    Log_function_name_on_leave();
    return len;
  }
  buf[len] = ShiftUpBit(*field().tos2(), 7);
  buf[len] += ShiftUpBit(*field().ttl2(), 6);
  buf[len] += ShiftUpBit(*field().df2(), 5);
  buf[len] += ShiftUpBit(*field().pr2(), 4);
  buf[len] += ShiftUpBit(*field().ipx2(), 3);
  buf[len] += ShiftUpBit(*field().nbo2(), 2);
  buf[len] += ShiftUpBit(*field().rnd2(), 1);
  buf[len] += ShiftUpBit(*field().i2__bit(), 0);
  len += 1;

  Log_hexdump(buf, len);
  Log_function_name_on_leave();

  return len;
}

int
Set_Profile1_ipx_headers_opt(unsigned char *buf,
                             const OPTIONAL < IP__Ext__heads > &field)
{
  int len = 0;

  Log_function_name_on_enter();

  if (!field.ispresent())
  {
    Log_function_name_on_leave();
    return len;
  }
  buf[len] = ShiftUpBit(*field().cl(), 7);
  buf[len] += ShiftUpBit(*field().aseq(), 6);
  buf[len] += ShiftUpBit(*field().eseq(), 5);
  buf[len] += ShiftUpBit(*field().gseq(), 4);
  buf[len] += (*field().res()) & 0x0F;
  len += 1;

  Check_field_presence("AH SN", *field().aseq() == 1,
                       field().ah__seq().ispresent());
  Check_field_presence("ESP SN", *field().eseq() == 1,
                       field().esp__seq().ispresent());
  Check_field_presence("GRE SN", *field().gseq() == 1,
                       field().gre__seq().ispresent());
  Check_field_presence("Compressed header list", *field().cl() == 1,
                       field().compr__head__list().ispresent());
  len += Set_AEGSeqnum_opt(&buf[len], field().ah__seq());
  len += Set_AEGSeqnum_opt(&buf[len], field().esp__seq());
  len += Set_AEGSeqnum_opt(&buf[len], field().gre__seq());
  len += Set_Compr_head_list_opt(&buf[len], field().compr__head__list());

  Log_hexdump(buf, len);
  Log_function_name_on_leave();

  return len;
}

int
Set_Profile1_Inner_IP_fields_opt(unsigned char *buf,
                                 const OPTIONAL < Inner__IP__flags > &flags,
                                 const OPTIONAL < Inner__IP__fields > &field)
{
  int len = 0;

  Log_function_name_on_enter();

  if (!field.ispresent())
  {
    Log_function_name_on_leave();
    return len;
  }
  Check_field_presence("TOS (inner IP)",
                       flags.ispresent() && (*flags().tos() == 1),
                       field().tos().ispresent());
  len += Set_LIN1_opt(&buf[len], field().tos());
  Check_field_presence("TTL (inner IP)",
                       flags.ispresent() && (*flags().ttl() == 1),
                       field().ttl().ispresent());
  len += Set_LIN1_opt(&buf[len], field().ttl());
  Check_field_presence("PROTO (inner IP)",
                       flags.ispresent() && (*flags().pr() == 1),
                       field().proto().ispresent());
  len += Set_LIN1_opt(&buf[len], field().proto());

  Check_field_presence("IP extension headers (inner IP)",
                       flags.ispresent() && (*flags().ipx() == 1),
                       field().ext__heads().ispresent());
  len += Set_Profile1_ipx_headers_opt(&buf[len], field().ext__heads());

  Log_hexdump(buf, len);
  Log_function_name_on_leave();

  return len;
}

int
Set_Profile1_Outer_IP_fields_opt(unsigned char *buf,
                                 const OPTIONAL < Outer__IP__flags > &flags,
                                 const OPTIONAL < Outer__IP__fields > &field)
{
  int len = 0;

  Log_function_name_on_enter();

  if (!field.ispresent())
  {
    Log_function_name_on_leave();
    return len;
  }
  Check_field_presence("TOS (outer IP)",
                       flags.ispresent() && (*flags().tos2() == 1),
                       field().tos().ispresent());
  len += Set_LIN1_opt(&buf[len], field().tos());
  Check_field_presence("TTL (outer IP)",
                       flags.ispresent() && (*flags().ttl2() == 1),
                       field().ttl().ispresent());
  len += Set_LIN1_opt(&buf[len], field().ttl());
  Check_field_presence("PROTO (outer IP)",
                       flags.ispresent() && (*flags().pr2() == 1),
                       field().proto().ispresent());
  len += Set_LIN1_opt(&buf[len], field().proto());

  Check_field_presence("IP extension headers (outer IP)",
                       flags.ispresent() && (*flags().ipx2() == 1),
                       field().ext__heads().ispresent());
  len += Set_Profile1_ipx_headers_opt(&buf[len], field().ext__heads());
  Check_field_presence("IP-ID (outer IP)",
                       flags.ispresent() && (*flags().i2__bit() == 1),
                       field().ip__id().ispresent());
  len += Set_LIN2_BO_LAST_opt(&buf[len], field().ip__id());

  Log_hexdump(buf, len);
  Log_function_name_on_leave();

  return len;
}

int
Set_Profile1_ROHC_Header_Extension0(unsigned char *buf, const Extension0 & ext)
{
  int len = 0;

  Log_function_name_on_enter();

  Check_field_value("extension type", (int) *ext.ext__type(), 0);
  buf[len] = ((*ext.ext__type()) << 6) & 0xC0;
  buf[len] += (ext.sn() << 3) & 0x38;
  buf[len] += ext.plust() & 0x07;
  len += 1;

  Log_hexdump(buf, len);
  Log_function_name_on_leave();

  return len;
}

int
Set_Profile1_ROHC_Header_Extension1(unsigned char *buf, const Extension1 & ext)
{
  int len = 0;

  Log_function_name_on_enter();

  Check_field_value("extension type", (int) *ext.ext__type(), 1);
  buf[len] = ((*ext.ext__type()) << 6) & 0xC0;
  buf[len] += (ext.sn() << 3) & 0x38;
  buf[len] += ext.plust() & 0x07;
  len += 1;
  len += Set_LIN1(&buf[len], ext.minust());

  Log_hexdump(buf, len);
  Log_function_name_on_leave();

  return len;
}

int
Set_Profile1_ROHC_Header_Extension2(unsigned char *buf, const Extension2 & ext)
{
  int len = 0;

  Log_function_name_on_enter();

  Check_field_value("extension type", (int) *ext.ext__type(), 2);
  buf[len] = ((*ext.ext__type()) << 6) & 0xC0;
  buf[len] += (ext.sn() << 3) & 0x38;
  buf[len] += (ext.plust() >> 8) & 0x07;
  len += 1;
  buf[len] = ext.plust() & 0xFF;
  len += 1;
  len += Set_LIN1(&buf[len], ext.minust());

  Log_hexdump(buf, len);
  Log_function_name_on_leave();

  return len;
}

int
Set_Profile1_ROHC_Header_Extension3(unsigned char *buf, const Extension3 & ext)
{
  int len = 0;
  int ts_length = 0;

  Log_function_name_on_enter();

  Check_field_value("extension type", (int) *ext.ext__type(), 3);
  buf[len] = ((*ext.ext__type()) << 6) & 0xC0;
  buf[len] += ShiftUpBit(*ext.s__bit(), 5);
  buf[len] += ShiftUpBit(*ext.r__ts__bit(), 4);
  buf[len] += ShiftUpBit(*ext.tsc__bit(), 3);
  buf[len] += ShiftUpBit(*ext.i__bit(), 2);
  buf[len] += ShiftUpBit(*ext.ip__bit(), 1);
  buf[len] += ShiftUpBit(*ext.rtp__bit(), 0);
  len += 1;

  Check_field_presence("Inner IP flags",
                       *ext.ip__bit() == 1,
                       ext.inner__ip__flags().ispresent());
  len += Set_Profile1_Inner_IP_flags_opt(&buf[len], ext.inner__ip__flags());
  Check_field_presence("Outer IP flags",
                       ext.inner__ip__flags().ispresent() &&
                       (*ext.inner__ip__flags()().ip2__bit() == 1),
                       ext.outer__ip__flags().ispresent());
  len += Set_Profile1_Outer_IP_flags_opt(&buf[len], ext.outer__ip__flags());
  Check_field_presence("SN", *ext.s__bit() == 1, ext.sn().ispresent());
  len += Set_LIN1_opt(&buf[len], ext.sn());
  Check_field_presence("RTP TS", *ext.r__ts__bit() == 1, ext.ts().ispresent());
  if (ext.ts__length().ispresent()) ts_length = ext.ts__length()();
  len += Set_SDVL_field_opt(&buf[len], ext.ts(), ts_length);
  Check_field_presence("Inner IP fields",
                       *ext.ip__bit() == 1, ext.inner__ip__hdr().ispresent());
  len += Set_Profile1_Inner_IP_fields_opt(&buf[len], ext.inner__ip__flags(),
                                          ext.inner__ip__hdr());
  Check_field_presence("Inner IP-ID",
                       *ext.i__bit() == 1, ext.ip__id().ispresent());
  len += Set_LIN2_BO_LAST_opt(&buf[len], ext.ip__id());
  Check_field_presence("Outer IP fields",
                       ext.inner__ip__flags().ispresent() &&
                       (*ext.inner__ip__flags()().ip2__bit() == 1),
                       ext.outer__ip__hdr().ispresent());
  len += Set_Profile1_Outer_IP_fields_opt(&buf[len], ext.outer__ip__flags(),
                                          ext.outer__ip__hdr());
  Check_field_presence("RTP flags and fields",
                       *ext.rtp__bit() == 1, ext.rtp__fl__fi().ispresent());
  len += Set_RTP_flags_fields_opt(&buf[len], ext.rtp__fl__fi());

  Log_hexdump(buf, len);
  Log_function_name_on_leave();

  return len;
}

int
Set_Profile1_ROHC_Header_Extension_opt(unsigned char *buf,
                                       const OPTIONAL < Profile1__Extension >
                                       &ext)
{
  int len = 0;

  Log_function_name_on_enter();

  if (ext.ispresent())
  {
    if (ext().get_selection() == Profile1__Extension::ALT_ext0)
    {
      len += Set_Profile1_ROHC_Header_Extension0(&buf[len], ext().ext0());
    }
    else if (ext().get_selection() == Profile1__Extension::ALT_ext1)
    {
      len += Set_Profile1_ROHC_Header_Extension1(&buf[len], ext().ext1());
    }
    else if (ext().get_selection() == Profile1__Extension::ALT_ext2)
    {
      len += Set_Profile1_ROHC_Header_Extension2(&buf[len], ext().ext2());
    }
    else if (ext().get_selection() == Profile1__Extension::ALT_ext3)
    {
      len += Set_Profile1_ROHC_Header_Extension3(&buf[len], ext().ext3());
    }
    else
    {
      Log_not_implemented_union_field("Profile1_Extension",
                                      ext().get_selection());
    }
  }

  Log_hexdump(buf, len);
  Log_function_name_on_leave();

  return len;
}

int
Set_Profile1_ROHC_Header_ipid_ah_gre(unsigned char *buf,
                                     const Profile1__headers & hdr)
{
  int len = 0;

  Log_function_name_on_enter();

  len += Set_LIN2_BO_LAST_opt(&buf[len], hdr.ip__id__outer());
  len += Set_octetstring_opt(&buf[len], hdr.ah__outer());
  len += Set_LIN2_BO_LAST_opt(&buf[len], hdr.gre__cksum1());
  len += Set_LIN2_BO_LAST_opt(&buf[len], hdr.ip__id__inner());
  len += Set_octetstring_opt(&buf[len], hdr.ah__inner());
  len += Set_LIN2_BO_LAST_opt(&buf[len], hdr.gre__cksum2());

  Log_hexdump(buf, len);
  Log_function_name_on_leave();

  return len;
}

int
Set_Profile1_ROHC_Header(unsigned char *buf,
                         const Profile1__headers & hdr,
                         ROHC__config const &config)
{
  int len = 0, crcpos = 0, cidlen = 0, cid;

  Log_function_name_on_enter();

  switch (hdr.base__header().get_selection())
  {
  case Profile1__base__header::ALT_ir:
    cid = hdr.base__header().ir().cid();
    crcpos = len + 2;
    len += Set_Profile1_ROHC_Header_ir(&buf[len], hdr.base__header().ir());
    break;
  case Profile1__base__header::ALT_ir__dyn:
    cid = hdr.base__header().ir__dyn().cid();
    crcpos = len + 2;
    len += Set_Profile1_ROHC_Header_irdyn(&buf[len],
                                          hdr.base__header().ir__dyn());
    break;
  case Profile1__base__header::ALT_r__0:
    cid = hdr.base__header().r__0().cid();
    len += Set_Profile1_ROHC_Header_r0(&buf[len], hdr.base__header().r__0());
    break;
  case Profile1__base__header::ALT_r__0__crc:
    cid = hdr.base__header().r__0__crc().cid();
    len += Set_Profile1_ROHC_Header_r0crc(&buf[len],
                                          hdr.base__header().r__0__crc());
    break;
  case Profile1__base__header::ALT_uo__0:
    cid = hdr.base__header().uo__0().cid();
    len += Set_Profile1_ROHC_Header_uo0(&buf[len], hdr.base__header().uo__0());
    break;
  case Profile1__base__header::ALT_r__1:
    cid = hdr.base__header().r__1().cid();
    len += Set_Profile1_ROHC_Header_r1(&buf[len], hdr.base__header().r__1());
    break;
  case Profile1__base__header::ALT_r__1__id:
    cid = hdr.base__header().r__1__id().cid();
    len +=
      Set_Profile1_ROHC_Header_r1id(&buf[len], hdr.base__header().r__1__id());
    break;
  case Profile1__base__header::ALT_r__1__ts:
    cid = hdr.base__header().r__1__ts().cid();
    len +=
      Set_Profile1_ROHC_Header_r1ts(&buf[len], hdr.base__header().r__1__ts());
    break;
  case Profile1__base__header::ALT_uo__1:
    cid = hdr.base__header().uo__1().cid();
    len += Set_Profile1_ROHC_Header_uo1(&buf[len], hdr.base__header().uo__1());
    break;
  case Profile1__base__header::ALT_uo__1__id:
    cid = hdr.base__header().uo__1__id().cid();
    len +=
      Set_Profile1_ROHC_Header_uo1id(&buf[len],
                                     hdr.base__header().uo__1__id());
    break;
  case Profile1__base__header::ALT_uo__1__ts:
    cid = hdr.base__header().uo__1__ts().cid();
    len +=
      Set_Profile1_ROHC_Header_uo1ts(&buf[len],
                                     hdr.base__header().uo__1__ts());
    break;
  case Profile1__base__header::ALT_uor__2:
    cid = hdr.base__header().uor__2().cid();
    len +=
      Set_Profile1_ROHC_Header_uor2(&buf[len], hdr.base__header().uor__2());
    break;
  case Profile1__base__header::ALT_uor__2__id:
    cid = hdr.base__header().uor__2__id().cid();
    len +=
      Set_Profile1_ROHC_Header_uor2id(&buf[len],
                                      hdr.base__header().uor__2__id());
    break;
  case Profile1__base__header::ALT_uor__2__ts:
    cid = hdr.base__header().uor__2__ts().cid();
    len +=
      Set_Profile1_ROHC_Header_uor2ts(&buf[len],
                                      hdr.base__header().uor__2__ts());
    break;
  default:
    Log_not_implemented_union_field
      ("Profile1_base_header", hdr.base__header().get_selection());
    break;
  }

  len += Set_Profile1_ROHC_Header_Extension_opt(&buf[len], hdr.ext());
  len += Set_Profile1_ROHC_Header_ipid_ah_gre(&buf[len], hdr);
  len += Set_LIN2_BO_LAST_opt(&buf[len], hdr.udp__cksum());

  cidlen = Set_CID_for_packets(buf, config, cid, len);
  crcpos += cidlen;
  len += cidlen;

  if (hdr.base__header().get_selection() == Profile1__base__header::ALT_ir
      && hdr.base__header().ir().crc() == 0)
    buf[crcpos] = ComputeCRC(&(buf[0]), len, 8);
  else if (hdr.base__header().get_selection() ==
           Profile1__base__header::ALT_ir__dyn
           && hdr.base__header().ir__dyn().crc() == 0)
    buf[crcpos] = ComputeCRC(&(buf[0]), len, 8);

  Log_hexdump(buf, len);
  Log_function_name_on_leave();

  return len;
}

int
Set_Profile2_Static_Chain(unsigned char
                          *buf, const Profile2__Static__Chain & chain)
{
  int len = 0;

  Log_function_name_on_enter();

  for (int num = 0; num < chain.size_of(); num++)
  {
    switch (chain[num].get_selection())
    {
    case Static__Chain__u::ALT_ipv4__stat:
      len += Set_IPv4_static_chain(&buf[len], chain[num].ipv4__stat());
      break;
    case Static__Chain__u::ALT_ipv6__stat:
      len += Set_IPv6_static_chain(&buf[len], chain[num].ipv6__stat());
      break;
    case Static__Chain__u::ALT_udp__stat:
      len += Set_UDP_static_chain(&buf[len], chain[num].udp__stat());
      break;
    default:
      Log_not_implemented_union_field
        ("Profile2_Static_Chain", chain[num].get_selection());
      break;
    }
  }

  Log_hexdump(buf, len);
  Log_function_name_on_leave();

  return len;
}

int
Set_Profile2_Dynamic_Chain(unsigned char
                           *buf, const Profile2__Dynamic__Chain & dynch)
{
  int len = 0;

  Log_function_name_on_enter();

  for (int num = 0; num < dynch.size_of(); num++)
  {
    switch (dynch[num].get_selection())
    {
    case Profile2__Dynamic__Chain__u::ALT_ipv4__dyn:
      len += Set_IPv4_dynamic_chain(&buf[len], dynch[num].ipv4__dyn());
      break;
    case Profile2__Dynamic__Chain__u::ALT_ipv6__dyn:
      len += Set_IPv6_dynamic_chain(&buf[len], dynch[num].ipv6__dyn());
      break;
    case Profile2__Dynamic__Chain__u::ALT_udp__dyn:
      len += Set_LIN2_BO_LAST(&buf[len], dynch[num].udp__dyn().cksum());
      len += Set_LIN2_BO_LAST(&buf[len], dynch[num].udp__dyn().udp__sn());
      break;
    default:
      Log_not_implemented_union_field
        ("Profile2_Dynamic_Chain", dynch[num].get_selection());
      break;
    }
  }

  Log_hexdump(buf, len);
  Log_function_name_on_leave();

  return len;
}

int
Set_Profile2_ROHC_Header_ir(unsigned char
                            *buf, const Profile2__IR__header & hdr)
{
  int len = 0;

  Log_function_name_on_enter();

  Check_field_value("packet type", (int) *hdr.type__ind(), 126);
  buf[len] = ((*hdr.type__ind()) << 1) & 0xFE;
  Check_field_presence("dynamic chain", *hdr.d() == 1,
                       hdr.dyn__chain().ispresent());
  buf[len] += ShiftUpBit(*hdr.d(), 0);
  len += 1;
  Check_field_value("profile", hdr.profile(), 2);
  len += Set_LIN1(&buf[len], hdr.profile());
  len += Set_LIN1(&buf[len], hdr.crc());
  len += Set_Profile2_Static_Chain(&buf[len], hdr.stat__chain());
  if (hdr.dyn__chain().ispresent())
    len += Set_Profile2_Dynamic_Chain(&buf[len], hdr.dyn__chain());

  Log_hexdump(buf, len);
  Log_function_name_on_leave();

  return len;
}

int
Set_Profile2_ROHC_Header_irdyn(unsigned
                               char *buf,
                               const Profile2__IR__DYN__header & hdr)
{
  int len = 0;

  Log_function_name_on_enter();

  Check_field_value("packet type", (int) *hdr.type__ind(), 248);
  buf[len] = *hdr.type__ind();
  len += 1;
  Check_field_value("profile", hdr.profile(), 2);
  len += Set_LIN1(&buf[len], hdr.profile());
  len += Set_LIN1(&buf[len], hdr.crc());
  len += Set_Profile2_Dynamic_Chain(&buf[len], hdr.dyn__chain());

  Log_hexdump(buf, len);
  Log_function_name_on_leave();

  return len;
}

int
Set_Profile2_ROHC_Header_r1(unsigned char
                            *buf, const Profile2__R__1__header & hdr)
{
  int len = 0;

  Log_function_name_on_enter();

  Check_field_value("packet type", (int) *hdr.type__ind(), 2);
  buf[len] = ((*hdr.type__ind()) << 6) & 0xC0;
  buf[len] += hdr.sn() & 0x3F;
  len += 1;
  buf[len] = ShiftUpBit(*hdr.x__bit(), 7);
  buf[len] += hdr.ip__id() & 0x7F;
  len += 1;

  Log_hexdump(buf, len);
  Log_function_name_on_leave();

  return len;
}

int
Set_Profile2_ROHC_Header_uo1(unsigned char
                             *buf, const Profile2__UO__1__header & hdr)
{
  int len = 0;

  Log_function_name_on_enter();

  Check_field_value("packet type", (int) *hdr.type__ind(), 2);
  buf[len] = ((*hdr.type__ind()) << 6) & 0xC0;
  buf[len] += hdr.ip__id() & 0x3F;
  len += 1;
  buf[len] = (hdr.sn() << 3) & 0xF8;
  buf[len] += hdr.crc() & 0x07;
  len += 1;

  Log_hexdump(buf, len);
  Log_function_name_on_leave();

  return len;
}

int
Set_Profile2_ROHC_Header_uor2(unsigned char *buf,
                              const Profile2__UOR__2__header & hdr)
{
  int len = 0;

  Log_function_name_on_enter();

  Check_field_value("packet type", (int) *hdr.type__ind(), 6);
  buf[len] = ((*hdr.type__ind()) << 5) & 0xE0;
  buf[len] += hdr.sn() & 0x1F;
  len += 1;
  buf[len] = ShiftUpBit(*hdr.x__bit(), 7);
  buf[len] += hdr.crc() & 0x7F;
  len += 1;

  Log_hexdump(buf, len);
  Log_function_name_on_leave();

  return len;
}

int
Set_Profile2_Inner_IP_flags_opt(unsigned char *buf,
                                const OPTIONAL < Profile2__Inner__IP__flags >
                                &field)
{
  int len = 0;

  Log_function_name_on_enter();

  if (!field.ispresent())
  {
    Log_function_name_on_leave();
    return len;
  }
  buf[len] = ShiftUpBit(*field().tos(), 7);
  buf[len] += ShiftUpBit(*field().ttl(), 6);
  buf[len] += ShiftUpBit(*field().df(), 5);
  buf[len] += ShiftUpBit(*field().pr(), 4);
  buf[len] += ShiftUpBit(*field().ipx(), 3);
  buf[len] += ShiftUpBit(*field().nbo(), 2);
  buf[len] += ShiftUpBit(*field().rnd__bit(), 1);
  Check_field_value("inner IP flags, reserved", (int) *field().reserved(), 0);
  buf[len] += ShiftUpBit(*field().reserved(), 0);
  len += 1;

  Log_hexdump(buf, len);
  Log_function_name_on_leave();

  return len;
}

int
Set_Profile2_Inner_IP_fields_opt(unsigned char *buf,
                                 const OPTIONAL < Profile2__Inner__IP__flags >
                                 &flags,
                                 const OPTIONAL < Inner__IP__fields > &field)
{
  int len = 0;

  Log_function_name_on_enter();

  if (!field.ispresent())
  {
    Log_function_name_on_leave();
    return len;
  }
  Check_field_presence("TOS (inner IP)",
                       flags.ispresent() && (*flags().tos() == 1),
                       field().tos().ispresent());
  len += Set_LIN1_opt(&buf[len], field().tos());
  Check_field_presence("TTL (inner IP)",
                       flags.ispresent() && (*flags().ttl() == 1),
                       field().ttl().ispresent());
  len += Set_LIN1_opt(&buf[len], field().ttl());
  Check_field_presence("PROTO (inner IP)",
                       flags.ispresent() && (*flags().pr() == 1),
                       field().proto().ispresent());
  len += Set_LIN1_opt(&buf[len], field().proto());

  len += Set_Profile1_ipx_headers_opt(&buf[len], field().ext__heads());

  Log_hexdump(buf, len);
  Log_function_name_on_leave();

  return len;
}


int
Set_Profile2_ROHC_Header_Extension0(unsigned char *buf,
                                    const Profile2__Extension0 & ext)
{
  int len = 0;

  Log_function_name_on_enter();

  Check_field_value("extension type", (int) *ext.ext__type(), 0);
  buf[len] = ((*ext.ext__type()) << 6) & 0xC0;
  buf[len] += (ext.sn() << 3) & 0x38;
  buf[len] += ext.ip__id() & 0x07;
  len += 1;

  Log_hexdump(buf, len);
  Log_function_name_on_leave();

  return len;
}

int
Set_Profile2_ROHC_Header_Extension1(unsigned char *buf,
                                    const Profile2__Extension1 & ext)
{
  int len = 0;

  Log_function_name_on_enter();

  Check_field_value("extension type", (int) *ext.ext__type(), 1);
  buf[len] = ((*ext.ext__type()) << 6) & 0xC0;
  buf[len] += (ext.sn() << 3) & 0x38;
  buf[len] += (ext.ip__id() >> 8) & 0x07;
  len += 1;
  buf[len] = ext.ip__id() & 0xFF;
  len += 1;

  Log_hexdump(buf, len);
  Log_function_name_on_leave();

  return len;
}

int
Set_Profile2_ROHC_Header_Extension2(unsigned char *buf,
                                    const Profile2__Extension2 & ext)
{
  int len = 0;

  Log_function_name_on_enter();

  Check_field_value("extension type", (int) *ext.ext__type(), 2);
  buf[len] = ((*ext.ext__type()) << 6) & 0xC0;
  buf[len] += (ext.sn() << 3) & 0x38;
  buf[len] += (ext.ip__id2() >> 8) & 0x07;
  len += 1;
  buf[len] = ext.ip__id2() & 0xFF;
  len += 1;
  buf[len] = ext.ip__id() & 0xFF;
  len += 1;

  Log_hexdump(buf, len);
  Log_function_name_on_leave();

  return len;
}

int
Set_Profile2_ROHC_Header_Extension3(unsigned char *buf,
                                    const Profile2__Extension3 & ext)
{
  int len = 0;

  Log_function_name_on_enter();

  Check_field_value("extension type", (int) *ext.ext__type(), 3);
  buf[len] = ((*ext.ext__type()) << 6) & 0xC0;
  buf[len] += ShiftUpBit(*ext.s__bit(), 5);
  buf[len] += ((ext.mode()) << 3) & 0x18;
  buf[len] += ShiftUpBit(*ext.i__bit(), 2);
  buf[len] += ShiftUpBit(*ext.ip__bit(), 1);
  buf[len] += ShiftUpBit(*ext.ip2__bit(), 0);
  len += 1;

  Check_field_presence("Inner IP flags",
                       *ext.ip__bit() == 1,
                       ext.inner__ip__flags().ispresent());
  len += Set_Profile2_Inner_IP_flags_opt(&buf[len], ext.inner__ip__flags());
  Check_field_presence("Outer IP flags",
                       *ext.ip2__bit() == 1,
                       ext.outer__ip__flags().ispresent());
  len += Set_Profile1_Outer_IP_flags_opt(&buf[len], ext.outer__ip__flags());
  Check_field_presence("SN", *ext.s__bit() == 1, ext.sn().ispresent());
  len += Set_LIN1_opt(&buf[len], ext.sn());
  Check_field_presence("Inner IP fields",
                       *ext.ip__bit() == 1, ext.inner__ip__hdr().ispresent());
  len += Set_Profile2_Inner_IP_fields_opt(&buf[len], ext.inner__ip__flags(),
                                          ext.inner__ip__hdr());
  Check_field_presence("Inner IP-ID",
                       *ext.i__bit() == 1, ext.ip__id().ispresent());
  len += Set_LIN2_BO_LAST_opt(&buf[len], ext.ip__id());
  Check_field_presence("Outer IP fields",
                       *ext.ip2__bit() == 1, ext.outer__ip__hdr().ispresent());
  len += Set_Profile1_Outer_IP_fields_opt(&buf[len], ext.outer__ip__flags(),
                                          ext.outer__ip__hdr());

  Log_hexdump(buf, len);
  Log_function_name_on_leave();

  return len;
}

int
Set_Profile2_ROHC_Header_Extension_opt(unsigned char *buf,
                                       const OPTIONAL < Profile2__Extension >
                                       &ext)
{
  int len = 0;

  Log_function_name_on_enter();

  if (ext.ispresent())
  {
    if (ext().get_selection() == Profile2__Extension::ALT_ext0)
    {
      len += Set_Profile2_ROHC_Header_Extension0(&buf[len], ext().ext0());
    }
    else if (ext().get_selection() == Profile2__Extension::ALT_ext1)
    {
      len += Set_Profile2_ROHC_Header_Extension1(&buf[len], ext().ext1());
    }
    else if (ext().get_selection() == Profile2__Extension::ALT_ext2)
    {
      len += Set_Profile2_ROHC_Header_Extension2(&buf[len], ext().ext2());
    }
    else if (ext().get_selection() == Profile2__Extension::ALT_ext3)
    {
      len += Set_Profile2_ROHC_Header_Extension3(&buf[len], ext().ext3());
    }
    else
    {
      Log_not_implemented_union_field("Profile2_Extension",
                                      ext().get_selection());
    }
  }

  Log_hexdump(buf, len);
  Log_function_name_on_leave();

  return len;
}

int
Set_Profile2_ROHC_Header_ipid_ah_gre(unsigned char *buf,
                                     const Profile2__headers & hdr)
{
  int len = 0;

  Log_function_name_on_enter();

  len += Set_LIN2_BO_LAST_opt(&buf[len], hdr.ip__id__outer());
  len += Set_octetstring_opt(&buf[len], hdr.ah__outer());
  len += Set_LIN2_BO_LAST_opt(&buf[len], hdr.gre__cksum1());
  len += Set_LIN2_BO_LAST_opt(&buf[len], hdr.ip__id__inner());
  len += Set_octetstring_opt(&buf[len], hdr.ah__inner());
  len += Set_LIN2_BO_LAST_opt(&buf[len], hdr.gre__cksum2());

  Log_hexdump(buf, len);
  Log_function_name_on_leave();

  return len;
}

int
Set_Profile2_ROHC_Header(unsigned char *buf,
                         const Profile2__headers & hdr,
                         ROHC__config const &config)
{
  int len = 0, crcpos = 0, cidlen = 0, cid;

  Log_function_name_on_enter();

  switch (hdr.base__header().get_selection())
  {
  case Profile2__base__header::ALT_ir:
    cid = hdr.base__header().ir().cid();
    crcpos = len + 2;
    len += Set_Profile2_ROHC_Header_ir(&buf[len], hdr.base__header().ir());
    break;
  case Profile2__base__header::ALT_ir__dyn:
    cid = hdr.base__header().ir__dyn().cid();
    crcpos = len + 2;
    len +=
      Set_Profile2_ROHC_Header_irdyn(&buf[len], hdr.base__header().ir__dyn());
    break;
  case Profile1__base__header::ALT_r__0:
    cid = hdr.base__header().r__0().cid();
    len += Set_Profile1_ROHC_Header_r0(&buf[len], hdr.base__header().r__0());
    break;
  case Profile1__base__header::ALT_r__0__crc:
    cid = hdr.base__header().r__0__crc().cid();
    len +=
      Set_Profile1_ROHC_Header_r0crc(&buf[len],
                                     hdr.base__header().r__0__crc());
    break;
  case Profile1__base__header::ALT_uo__0:
    cid = hdr.base__header().uo__0().cid();
    len += Set_Profile1_ROHC_Header_uo0(&buf[len], hdr.base__header().uo__0());
    break;
  case Profile2__base__header::ALT_r__1:
    cid = hdr.base__header().r__1().cid();
    len += Set_Profile2_ROHC_Header_r1(&buf[len], hdr.base__header().r__1());
    break;
  case Profile2__base__header::ALT_uo__1:
    cid = hdr.base__header().uo__1().cid();
    len += Set_Profile2_ROHC_Header_uo1(&buf[len], hdr.base__header().uo__1());
    break;
  case Profile2__base__header::ALT_uor__2:
    cid = hdr.base__header().uor__2().cid();
    len +=
      Set_Profile2_ROHC_Header_uor2(&buf[len], hdr.base__header().uor__2());
    break;
  default:
    Log_not_implemented_union_field
      ("Profile2_base_header", hdr.base__header().get_selection());
    break;
  }

  len += Set_Profile2_ROHC_Header_Extension_opt(&buf[len], hdr.ext());
  len += Set_Profile2_ROHC_Header_ipid_ah_gre(&buf[len], hdr);
  len += Set_LIN2_BO_LAST_opt(&buf[len], hdr.udp__cksum());

  cidlen = Set_CID_for_packets(buf, config, cid, len);
  crcpos += cidlen;
  len += cidlen;

  if (hdr.base__header().get_selection() == Profile2__base__header::ALT_ir)
  {
    if (hdr.base__header().ir().crc() == 0)
      buf[crcpos] = ComputeCRC(&(buf[0]), len, 8);
  }
  else
    if (hdr.base__header().
        get_selection() == Profile2__base__header::ALT_ir__dyn)
  {
    if (hdr.base__header().ir__dyn().crc() == 0)
      buf[crcpos] = ComputeCRC(&(buf[0]), len, 8);
  }

  Log_hexdump(buf, len);
  Log_function_name_on_leave();

  return len;
}                               /* Set_Profile2_ROHC_Header */

int
Set_Profile4_Static_Chain(unsigned char *buf,
                          const Profile4__Static__Chain & chain)
{
  int len = 0;

  Log_function_name_on_enter();

  for (int num = 0; num < chain.size_of(); num++)
  {
    switch (chain[num].get_selection())
    {
    case Static__Chain__u::ALT_ipv4__stat:
      len += Set_IPv4_static_chain(&buf[len], chain[num].ipv4__stat());
      break;
    case Static__Chain__u::ALT_ipv6__stat:
      len += Set_IPv6_static_chain(&buf[len], chain[num].ipv6__stat());
      break;
    default:
      Log_not_implemented_union_field
        ("Profile4_Static_Chain", chain[num].get_selection());
      break;
    }
  }

  Log_hexdump(buf, len);
  Log_function_name_on_leave();

  return len;
}

int
Set_Profile4_IPv4_dynamic(unsigned char *buf,
                          const Profile4__IPv4__Dynamic & chain)
{
  int len = 0;

  Log_function_name_on_enter();

  len += Set_LIN1(&buf[len], chain.tos());
  len += Set_LIN1(&buf[len], chain.ttl());
  len += Set_LIN2_BO_LAST(&buf[len], chain.identification());

  buf[len] = ShiftUpBit(*chain.df__bit(), 7);
  buf[len] += ShiftUpBit(*chain.rnd__bit(), 6);
  buf[len] += ShiftUpBit(*chain.nbo__bit(), 5);
  buf[len] += ShiftUpBit(*chain.sid__bit(), 4);
  Check_field_value("IPv4 reserved (dynamic chain)", *chain.reserved(), 0);
  buf[len] += (*chain.reserved()) & 0x0F;
  len += 1;
  len += Set_Encoding_Type_0(&buf[len], chain.genextheadlist());

  Log_hexdump(buf, len);
  Log_function_name_on_leave();

  return len;
}

int
Set_Profile4_Dynamic_Chain(unsigned char *buf,
                           const Profile4__Dynamic__Chain & dynch)
{
  int len = 0;

  Log_function_name_on_enter();

  for (int num = 0; num < dynch.size_of(); num++)
  {
    switch (dynch[num].get_selection())
    {
    case Profile4__Dynamic__Chain__u::ALT_ipv4__dyn:
      len += Set_Profile4_IPv4_dynamic(&buf[len], dynch[num].ipv4__dyn());
      break;
    case Profile4__Dynamic__Chain__u::ALT_ipv6__dyn:
      len += Set_IPv6_dynamic_chain(&buf[len], dynch[num].ipv6__dyn());
      break;
    case Profile4__Dynamic__Chain__u::ALT_sn:
      len += Set_LIN2_BO_LAST(&buf[len], dynch[num].sn());
      break;
    default:
      Log_not_implemented_union_field
        ("Profile4_Dynamic_Chain", dynch[num].get_selection());
      break;
    }
  }

  Log_hexdump(buf, len);
  Log_function_name_on_leave();

  return len;
}

int
Set_Profile4_ROHC_Header_ir(unsigned char *buf,
                            const Profile4__IR__header & hdr)
{
  int len = 0;

  Log_function_name_on_enter();

  Check_field_value("packet type", (int) *hdr.type__ind(), 126);
  buf[len] = ((*hdr.type__ind()) << 1) & 0xFE;
  Check_field_presence("dynamic chain", *hdr.d() == 1,
                       hdr.dyn__chain().ispresent());
  buf[len] += ShiftUpBit(*hdr.d(), 0);
  len += 1;
  Check_field_value("profile", hdr.profile(), 4);
  len += Set_LIN1(&buf[len], hdr.profile());
  len += Set_LIN1(&buf[len], hdr.crc());
  len += Set_Profile4_Static_Chain(&buf[len], hdr.stat__chain());
  if (hdr.dyn__chain().ispresent())
    len += Set_Profile4_Dynamic_Chain(&buf[len], hdr.dyn__chain());

  Log_hexdump(buf, len);
  Log_function_name_on_leave();

  return len;
}

int
Set_Profile4_ROHC_Header_irdyn(unsigned char *buf,
                               const Profile4__IR__DYN__header & hdr)
{
  int len = 0;

  Log_function_name_on_enter();

  Check_field_value("packet type", (int) *hdr.type__ind(), 248);
  buf[len] = *hdr.type__ind();
  len += 1;
  Check_field_value("profile", hdr.profile(), 4);
  len += Set_LIN1(&buf[len], hdr.profile());
  len += Set_LIN1(&buf[len], hdr.crc());
  len += Set_Profile4_Dynamic_Chain(&buf[len], hdr.dyn__chain());

  Log_hexdump(buf, len);
  Log_function_name_on_leave();

  return len;
}

int
Set_Profile4_ROHC_Header_ipid_ah_gre(unsigned char *buf,
                                     const Profile4__headers & other_headers)
{
  int len = 0;

  Log_function_name_on_enter();

  len += Set_LIN2_BO_LAST_opt(&buf[len], other_headers.ip__id__outer());
  len += Set_octetstring_opt(&buf[len], other_headers.ah__outer());
  len += Set_LIN2_BO_LAST_opt(&buf[len], other_headers.gre__cksum1());
  len += Set_LIN2_BO_LAST_opt(&buf[len], other_headers.ip__id__inner());
  len += Set_octetstring_opt(&buf[len], other_headers.ah__inner());
  len += Set_LIN2_BO_LAST_opt(&buf[len], other_headers.gre__cksum2());

  Log_hexdump(buf, len);
  Log_function_name_on_leave();

  return len;
}

int
Set_Profile4_ROHC_Header(unsigned char *buf,
                         const Profile4__headers & hdr,
                         ROHC__config const &config)
{
  int len = 0, crcpos = 0, cidlen = 0, cid;

  Log_function_name_on_enter();

  switch (hdr.base__header().get_selection())
  {
  case Profile4__base__header::ALT_ir:
    cid = hdr.base__header().ir().cid();
    crcpos = len + 2;
    len += Set_Profile4_ROHC_Header_ir(&buf[len], hdr.base__header().ir());
    break;
  case Profile4__base__header::ALT_ir__dyn:
    cid = hdr.base__header().ir__dyn().cid();
    crcpos = len + 2;
    len +=
      Set_Profile4_ROHC_Header_irdyn(&buf[len], hdr.base__header().ir__dyn());
    break;
  case Profile1__base__header::ALT_r__0:
    cid = hdr.base__header().r__0().cid();
    len += Set_Profile1_ROHC_Header_r0(&buf[len], hdr.base__header().r__0());
    break;
  case Profile1__base__header::ALT_r__0__crc:
    cid = hdr.base__header().r__0__crc().cid();
    len +=
      Set_Profile1_ROHC_Header_r0crc(&buf[len],
                                     hdr.base__header().r__0__crc());
    break;
  case Profile1__base__header::ALT_uo__0:
    cid = hdr.base__header().uo__0().cid();
    len += Set_Profile1_ROHC_Header_uo0(&buf[len], hdr.base__header().uo__0());
    break;
  case Profile2__base__header::ALT_r__1:
    cid = hdr.base__header().r__1().cid();
    len += Set_Profile2_ROHC_Header_r1(&buf[len], hdr.base__header().r__1());
    break;
  case Profile2__base__header::ALT_uo__1:
    cid = hdr.base__header().uo__1().cid();
    len += Set_Profile2_ROHC_Header_uo1(&buf[len], hdr.base__header().uo__1());
    break;
  case Profile2__base__header::ALT_uor__2:
    cid = hdr.base__header().uor__2().cid();
    len +=
      Set_Profile2_ROHC_Header_uor2(&buf[len], hdr.base__header().uor__2());
    break;
  default:
    Log_not_implemented_union_field
      ("Profile4_base_header", hdr.base__header().get_selection());
    break;
  }

  len += Set_Profile2_ROHC_Header_Extension_opt(&buf[len], hdr.ext());
  len += Set_Profile4_ROHC_Header_ipid_ah_gre(&buf[len], hdr);
  if (hdr.additional__IP().ispresent())
  {
    len += Set_Profile4_Dynamic_Chain(&buf[len], hdr.additional__IP()());
  }

  cidlen = Set_CID_for_packets(buf, config, cid, len);
  crcpos += cidlen;
  len += cidlen;

  if (hdr.base__header().get_selection() == Profile4__base__header::ALT_ir)
  {
    if (hdr.base__header().ir().crc() == 0)
      buf[crcpos] = ComputeCRC(&(buf[0]), len, 8);
  }
  else if (hdr.base__header().get_selection() ==
           Profile4__base__header::ALT_ir__dyn)
  {
    if (hdr.base__header().ir__dyn().crc() == 0)
      buf[crcpos] = ComputeCRC(&(buf[0]), len, 8);
  }

  Log_hexdump(buf, len);
  Log_function_name_on_leave();

  return len;
}

OCTETSTRING
f__ROHC__enc(ROHC__packet__u const &pdu, ROHC__config const &config)
{
  unsigned char buf[MAX_PACKET_SIZE];
  int len = 0;

  Log_function_name_on_enter();

  TTCN_logger.begin_event(TTCN_DEBUG);
  config.log();
  pdu.log();
  TTCN_logger.end_event();

  if (pdu.get_selection() == ROHC__packet__u::ALT_segment)
  {
    // Processing padding
    len += Set_octetstring_opt(&buf[len], pdu.segment().padding());
    // Processing feedback
    len +=
      Set_ROHC_Feedback_opt(&buf[len],
                            pdu.segment().feedback(), config.large__cid());
    // Processing segment header
    Check_field_value("segment header id",
                      pdu.segment().segment__header__id(), 127);
    buf[len] = (pdu.segment().segment__header__id() << 1) & 0xFE;
    if (pdu.segment().final())
      buf[len] += 1;
    len += 1;
    // Processing payload
    len += Set_octetstring_opt(&buf[len], pdu.segment().payload());
    // Processing CRC
    if (pdu.segment().crc().ispresent())
    {
      buf[len] = (pdu.segment().crc()() >> 24) & 0xFF;
      len += 1;
      buf[len] = (pdu.segment().crc()() >> 16) & 0xFF;
      len += 1;
      buf[len] = (pdu.segment().crc()() >> 8) & 0xFF;
      len += 1;
      buf[len] = pdu.segment().crc()() & 0xFF;
      len += 1;
    }
  }
  else if (pdu.get_selection() == ROHC__packet__u::ALT_rohc)
  {
    // Processing padding
    len += Set_octetstring_opt(&buf[len], pdu.rohc().padding());
    // Processing feedback
    len += Set_ROHC_Feedback_opt(&buf[len],
                                 pdu.rohc().feedback(), config.large__cid());
    // Processing ROHC header
    switch (pdu.rohc().header().get_selection())
    {
    case Header::ALT_prof0:
      len += Set_ROHC_Header_Profile0(&buf[len], pdu.rohc().header().prof0(),
                                      config);
      break;
    case Header::ALT_prof1:
      len += Set_Profile1_ROHC_Header(&buf[len], pdu.rohc().header().prof1(),
                                      config);
      break;
    case Header::ALT_prof2:
      len += Set_Profile2_ROHC_Header(&buf[len], pdu.rohc().header().prof2(),
                                      config);
      break;
    case Header::ALT_prof4:
      len += Set_Profile4_ROHC_Header(&buf[len], pdu.rohc().header().prof4(),
                                      config);
      break;
    default:
      Log_not_implemented_union_field("Header",
                                      pdu.rohc().header().get_selection());
      break;
    }

    // Processing payload
    len += Set_octetstring_opt(&buf[len], pdu.rohc().payload());
  }

  Log_hexdump(buf, len);
  Log_function_name_on_leave();

  return OCTETSTRING(len, &(buf[0]));
}





/* ============ Decoding functions for base types ============ */

/* Mirrors the bits of an octet and returns the same pointer. */
unsigned char *
Convert_bitstring(unsigned char *buf, int len)
{
  unsigned char dst_buf = 0;

  if (len > 8)
  {
    TTCN_error("Bitstring is too long, at most 8 bits can be converted");
  }
  for (int i = 0; i < 8; i++)
  {
    dst_buf += ShiftUpBit(ShiftDownBit(buf[0], i), 7 - i);
  }
  buf[0] = dst_buf;

  return buf;
}

/* Decodes an OCTETSTING value */
int
Get_octetstring(const unsigned char *buf, OCTETSTRING & octstr, int length)
{
  octstr = OCTETSTRING(length, buf);
  return length;
}

/* Decodes a LIN1 value */
int
Get_LIN1(const unsigned char *buf, LIN1 & var)
{
  var = LIN1(buf[0]);
  return 1;
}

/* Decodes a LIN2_BO_LAST value */
int
Get_LIN2_BO_LAST(const unsigned char *buf, LIN2__BO__LAST & var)
{
  var = (buf[0] << 8) + buf[1];
  return 2;
}

/* Decodes a LIN4_BO_LAST value */
int
Get_LIN4_BO_LAST(const unsigned char *buf, LIN4__BO__LAST & var)
{
  var = (buf[0] << 24) + (buf[1] << 16) + (buf[2] << 8) + buf[3];
  return 4;
}

int
Get_SDVL_field(const unsigned char *buf, INTEGER & dest)
{
  int len = 0;
  int decoded_value;

  Log_function_name_on_enter();

  if ((buf[len] & 0x80) == 0)
  {
    dest = buf[len] & 0x7F;
    len += 1;
  }
  else if ((buf[len] & 0xC0) == 0x80)
  {
    dest = ((buf[len] << 8) & 0x3F00) + (buf[len + 1] & 0xFF);
    len += 2;
  }
  else if ((buf[len] & 0xE0) == 0xC0)
  {
    dest =
      ((buf[len] << 16) & 0x1F0000) +
      ((buf[len + 1] << 8) & 0xFF00) + (buf[len + 2] & 0xFF);
    len += 3;
  }
  else                          // if ((buf[len] & 0xE0) == 0xE0)
  {
    dest =
      ((buf[len] << 24) & 0x1F000000) +
      ((buf[len + 1] << 16) & 0xFF0000) +
      ((buf[len + 2] << 8) & 0xFF00) + (buf[len + 3] & 0xFF);
    len += 4;
  }

  decoded_value = dest;
  TTCN_logger.log(TTCN_DEBUG, "Decoded value is %u (encoded on %u octets)",
                  decoded_value, len);

  Log_hexdump(buf, len);
  Log_function_name_on_leave();

  return len;
}

/* ============ Decoding wrappers for optional base types ============ */

/* Decodes a optional octetstring value */
int
Get_octetstring_opt(const unsigned char *buf, OPTIONAL < OCTETSTRING > &var,
                    bool hasvalue, int length)
{
  if (hasvalue)
  {
    return Get_octetstring(buf, var(), length);
  }
  var = OMIT_VALUE;
  return 0;
}

/* Decodes a optional LIN1 value */
int
Get_LIN1_opt(const unsigned char *buf, OPTIONAL < LIN1 > &var, bool hasvalue)
{
  if (hasvalue)
  {
    return Get_LIN1(buf, var());
  }
  var = OMIT_VALUE;
  return 0;
}

/* Decodes a optional LIN2_BO_LAST value */
int
Get_LIN2_BO_LAST_opt(const unsigned char *buf,
                     OPTIONAL < LIN2__BO__LAST > &var, bool hasvalue)
{
  if (hasvalue)
  {
    return Get_LIN2_BO_LAST(buf, var());
  }
  var = OMIT_VALUE;
  return 0;
}

/* Decodes a optional LIN4_BO_LAST value */
int
Get_LIN4_BO_LAST_opt(const unsigned char *buf,
                     OPTIONAL < LIN4__BO__LAST > &var, bool hasvalue)
{
  if (hasvalue)
  {
    return Get_LIN4_BO_LAST(buf, var());
  }
  var = OMIT_VALUE;
  return 0;
}

/* Decodes a optional SDVL value */
int
Get_SDVL_field_opt(const unsigned char *buf,
                   OPTIONAL < INTEGER > &var, bool hasvalue)
{
  if (hasvalue)
  {
    return Get_SDVL_field(buf, var());
  }
  var = OMIT_VALUE;
  return 0;
}

/* ============ Decoding functions for ROHC types ============ */

/* Returns the number of matching octets, that are the same as the passed
  'octet'. */
int
Strip_leading_octet(const unsigned char *buf, int length, unsigned char octet)
{
  int len = 0;

  while ((buf[len] == octet) && (len < length))
    len += 1;
  return len;
}

int
Get_ROHC_Padding(const unsigned char *buf, int length,
                 OPTIONAL < Padding > &p_padding, ROHC__config & config)
{
  int len = Strip_leading_octet(buf, length, 0xE0);

  Log_function_name_on_enter();

  if (len > 0)
  {
    p_padding = OCTETSTRING(len, &(buf[0]));
    Log_hexdump(buf, len);
  }
  else
    p_padding = OMIT_VALUE;

  Log_function_name_on_leave();

  return len;
}

int
Get_CID(const unsigned char *buf, INT14b__BO__LAST & cid,
        ROHC__config & config)
{
  int len = 0;
  int dec_cid;

  Log_function_name_on_enter();

  if (config.large__cid() == false)
  {
    if ((buf[len] & 0xF0) == 0xE0)
    {
      cid = buf[len] & 0x0F;
      len += 1;
    }
    else
      cid = 0;
  }
  else
  {
    len += Get_SDVL_field(&buf[len], cid);
  }

  dec_cid = cid;
  TTCN_logger.log(TTCN_DEBUG, "Decoded CID = %u", dec_cid);

  Log_hexdump(buf, len);
  Log_function_name_on_leave();

  return len;
}

int
Get_feedback(const unsigned char *buf, Feedback__u & feedback,
             ROHC__config & config)
{
  int len = 0, fbsize = 0, cidlen;
  unsigned char ctemp;

  Log_function_name_on_enter();

  ctemp = (buf[len] >> 3) & 0x1F;
  feedback.feedback__type() = BITSTRING(5, &ctemp);
  feedback.code() = buf[len] & 0x07;
  fbsize = feedback.code();
  len += 1;
  len += Get_LIN1_opt(&buf[len], feedback.size(), feedback.code() == 0);
  if (feedback.size().ispresent())
  {
    fbsize = feedback.size()();
  }

  cidlen = Get_CID(&buf[len], feedback.feedback__data().cid(), config);
  fbsize -= cidlen;
  len += cidlen;

  if (fbsize == 1)              /* Feedback type 1 */
  {
    len += Get_octetstring(&buf[len],
                           feedback.feedback__data().feedback__type().
                           feedback1(), fbsize);
  }
  else                          /* Feedback type 2 */
  {
    int optnum = 0;
    Feedback__opts fbopts;
    Feedback2 feedback2;

    feedback2.acktype() = (buf[len] >> 6) & 0x03;
    feedback2.mode() = (buf[len] >> 4) & 0x03;
    ctemp = buf[len] & 0x0F;
    len += 1;
    fbsize -= 1;
    feedback2.sn() = ((ctemp << 8) & 0x0F00) + buf[len];
    len += 1;
    fbsize -= 1;
    feedback2.feedback__opts() = OMIT_VALUE;
    while (fbsize > 0)
    {
      fbopts[optnum].opt__type() = (buf[len] >> 4) & 0x0F;
      fbopts[optnum].opt__len() = buf[len] & 0x0F;
      len += 1;
      fbsize -= 1;
      if (fbopts[optnum].opt__len())
      {
        len += Get_octetstring(&buf[len], fbopts[optnum].opt__data(),
                               fbopts[optnum].opt__len());
        fbsize -= fbopts[optnum].opt__len();
      }
      else
        fbopts[optnum].opt__data() = OMIT_VALUE;
      feedback2.feedback__opts() = fbopts;
      optnum += 1;
    }

    feedback.feedback__data().feedback__type().feedback2() = feedback2;
  }

  Log_hexdump(buf, len);
  Log_function_name_on_leave();

  return len;
}

int
Get_ROHC_Feedback(const unsigned char *buf,
                  int length,
                  OPTIONAL < Feedback > &feedback, ROHC__config & config)
{
  int len = 0, fbnum = 0;

  Log_function_name_on_enter();

  feedback = OMIT_VALUE;
  while (((buf[len] & 0xF8) == 0xF0) && (len < length))
  {
    len += Get_feedback(&buf[len], feedback()[fbnum], config);
    fbnum++;
  }

  Log_hexdump(buf, len);
  Log_function_name_on_leave();

  return len;
}

int
Get_XI_list(const unsigned char *buf,
            OPTIONAL < XI__list > &xilist, int m,
            const BITSTRING & ps_bit, int *itemnum,
            OPTIONAL < BITSTRING > &padding)
{
  int len = 0;
  unsigned char ctemp;

  Log_function_name_on_enter();

  if (!m)
  {
    xilist = OMIT_VALUE;
    padding = OMIT_VALUE;
    return len;
  }

  if ((*ps_bit) == 1)
  {
    for (int num = 0; num < m; num++)
    {
      ctemp = ShiftDownBit(buf[len], 7);
      xilist().xi__item8()[num].x__ind() = BITSTRING(1, &ctemp);
      if (ctemp)
        *itemnum += 1;
      xilist().xi__item8()[num].index() = buf[len] & 0x7F;
      len += 1;
    }
    padding = OMIT_VALUE;
  }
  else
  {
    int halfbyte = 0;

    for (int num = 0; num < m; num++)
    {
      if (halfbyte)
      {
        ctemp = ShiftDownBit(buf[len], 3);
        xilist().xi__item4()[num].x__ind() = BITSTRING(1, &ctemp);
        if (ctemp)
          *itemnum += 1;
        xilist().xi__item4()[num].index() = buf[len] & 0x07;
        len += 1;
      }
      else
      {
        ctemp = ShiftDownBit(buf[len], 7);
        xilist().xi__item4()[num].x__ind() = BITSTRING(1, &ctemp);
        if (ctemp)
          *itemnum += 1;
        xilist().xi__item4()[num].index() = (buf[len] >> 4) & 0x07;
      }
      halfbyte = 1 - halfbyte;  /* Invert the value */
    }
    /* Padding */
    if (halfbyte == 1)
    {
      ctemp = buf[len] & 0x0F;
      padding() = BITSTRING(4, &ctemp);
      len += 1;
    }
    else
      padding = OMIT_VALUE;
  }

  Log_hexdump(buf, len);
  Log_function_name_on_leave();

  return len;
}

int
Get_Item_list(const unsigned char *buf,
              OPTIONAL < Item__list > &item_list, int n,
              Item__list::union_selection_type selection, 
              t_dat *dat, int ip_idx)
{
  int len = 0;

  Log_function_name_on_enter();

  if (n == 0)
  {
    item_list = OMIT_VALUE;
    Log_function_name_on_leave();
    return len;
  }

  TTCN_logger.log(TTCN_DEBUG, "Expecting %u items", n);

  /* Item list is an IP extension header list */
  if (selection == Item__list::ALT_ip__item__list)
  {
    IP__Item__list & itlist = item_list().ip__item__list();
    for (int num = 0; num < n; num++)
    {
      int headid = buf[len];
      len += 1;

      TTCN_logger.log(TTCN_DEBUG, "%uth item", num);

      if (headid == c__ip__proto__ah)
      {
        AH__item & item = itlist[num].ah__item();
        int payload_length, ah_data_len;

        TTCN_logger.log(TTCN_DEBUG, "AH header found");
        item.nexthead() = headid;
        len += Get_LIN1(&buf[len], item.payload__len());
        payload_length = item.payload__len();
        ah_data_len = (item.payload__len() - 1) * 4;
        len += Get_LIN2_BO_LAST(&buf[len], item.reserved());
        len += Get_LIN4_BO_LAST(&buf[len], item.spi());
        len += Get_LIN4_BO_LAST(&buf[len], item.sn());
        len += Get_octetstring_opt(&buf[len], item.auth__data(),
                                   item.payload__len() != 1, ah_data_len);
        dat->context.ip__ctx()[ip_idx].ah__data__len() = ah_data_len;
        dat->context.ip__ctx()[ip_idx].ah__present() = true;
        // There is no compressed AH data after the base header
        dat->compressed_ah_data_len[ip_idx] = 0;
        TTCN_logger.log(TTCN_DEBUG, 
                    "AH payload length is %d meaning %d octets",
                    payload_length, ah_data_len);
        Log_hexdump(buf, len);
      }
      else if (headid == c__ip__proto__mine)
      {
        unsigned char ctemp;
        MINE__item & item = itlist[num].mine__item();

        TTCN_logger.log(TTCN_DEBUG, "MINE header found");
        item.protocol() = headid;
        ctemp = ShiftDownBit(buf[len], 7);
        item.s__bit() = BITSTRING(1, &ctemp);
        item.reserved() = BITSTRING(7, &buf[len]);
        len += 1;
        len += Get_LIN2_BO_LAST(&buf[len], item.cksum());
        len += Get_octetstring(&buf[len], item.dstaddr(), 4);
        len += Get_octetstring_opt(&buf[len], item.srcaddr(),
                                   *item.s__bit() == 1, 4);
        Log_hexdump(buf, len);
      }
      else if (headid == c__ip__proto__esp)
      {
        ESP__item & item = itlist[num].esp__item();

        TTCN_logger.log(TTCN_DEBUG, "ESP header found");
        item.nexthead() = headid;
        len += Get_LIN4_BO_LAST(&buf[len], item.spi());
        len += Get_LIN4_BO_LAST(&buf[len], item.sn());
        
        dat->context.ip__ctx()[ip_idx].esp__present() = true;
        Log_hexdump(buf, len);
      }
      else if (headid == c__ip__proto__gre2)
      {
        GRE__item & item = itlist[num].gre__item();
        unsigned char ctemp;

        TTCN_logger.log(TTCN_DEBUG, "GRE header found");
        item.nexthead() = headid;
        ctemp = ShiftDownBit(buf[len], 7);
        item.C__bit() = BITSTRING(1, &ctemp);
        ctemp = ShiftDownBit(buf[len], 6);
        item.reserved__1() = BITSTRING(1, &ctemp);
        ctemp = ShiftDownBit(buf[len], 5);
        item.K__bit() = BITSTRING(1, &ctemp);
        ctemp = ShiftDownBit(buf[len], 4);
        item.S__bit() = BITSTRING(1, &ctemp);
        ctemp = ShiftDownBit(buf[len], 3);
        item.reserved__2() = BITSTRING(1, &ctemp);
        ctemp = buf[len] & 0x07;
        item.version() = BITSTRING(3, &ctemp);
        len += 1;
        len += Get_LIN2_BO_LAST_opt(&buf[len], item.cksum(),
                                    *item.C__bit() == 1);
        len += Get_LIN4_BO_LAST_opt(&buf[len], item.key(),
                                    *item.K__bit() == 1);
        len += Get_LIN4_BO_LAST_opt(&buf[len], item.sn(), *item.S__bit() == 1);
        
        dat->context.ip__ctx()[ip_idx].gre__cksum__present() = (*item.C__bit() == 1);
        dat->context.ip__ctx()[ip_idx].gre__present() = true;
        // There is no compressed GRE data after the base header
        dat->ipx_gre_cksum_present[ip_idx] = false;
        Log_hexdump(buf, len);
      }
      else                      /* Assuming IPv6 extension header */
        /* TODO check: why only these were supported:
           if (headid == 0 || headid == 43 || headid == 60) */
      {
        IPv6__ext__item & item = itlist[num].ipv6__ext__item();
        int ext_len;
        int ext_len_oct;

        TTCN_logger.log(TTCN_DEBUG, "IPv6 header found (type %d)", headid);
        item.nexthead() = headid;
        len += Get_LIN1(&buf[len], item.hdr__ext__len());
        ext_len_oct = item.hdr__ext__len();
        ext_len = (item.hdr__ext__len() + 1) * 8;
        TTCN_logger.log(TTCN_DEBUG, 
                    "IPv6 extension length is %d meaning %d octets",
                    ext_len_oct, ext_len);
        // 2 octets already been read
        len += Get_octetstring(&buf[len], item.data(), ext_len - 2);
        Log_hexdump(buf, len);
      }
    }                           /* for (num) */
  }
  else if (selection == Item__list::ALT_csrc__item__list)
  {
    for (int num = 0; num < n; num++)
    {
      TTCN_logger.log(TTCN_DEBUG, "%uth item", num);

      len +=
        Get_octetstring(&buf[len], item_list().csrc__item__list()[num], 4);
    }
  }
  else if (selection == Item__list::ALT_raw__data)
  {
    /* TODO: RAW_data field is just a temporary hack for Profile4 tests */
    RAW__data & itlist = item_list().raw__data();

    for (int num = 0; num < n; num++)
    {
      int headid = char2int(buf[len]);

      if (headid == 0 || headid == 43 || headid == 60)
      {
        int itemlimit = char2int(buf[len + 1]) + 1 * 8;
        for (int itemlen = 0; itemlen < itemlimit; itemlen++)
        {
          if (itemlen == 0)
            itlist[num] = OCTETSTRING(1, &buf[len]);
          else
            itlist[num] = itlist[num] + OCTETSTRING(1, &buf[len]);
          len += 1;
        }
      }
      else if (headid == 51)
      {
        int itemlimit = char2int(buf[len + 1]) + 1 * 8;
        for (int itemlen = 0; itemlen < itemlimit; itemlen++)
        {
          if (itemlen == 0)
            itlist[num] = OCTETSTRING(1, &buf[len]);
          else
            itlist[num] = itlist[num] + OCTETSTRING(1, &buf[len]);
          len += 1;
        }
      }
      else if (headid == 50)
      {
        for (int itemlen = 0; itemlen < 9; itemlen++)
        {
          if (itemlen == 0)
            itlist[num] = OCTETSTRING(1, &buf[len]);
          else
            itlist[num] = itlist[num] + OCTETSTRING(1, &buf[len]);
          len += 1;
        }
      }
      else if (headid == 55 || headid == 47)
      {
        for (int itemlen = 0; itemlen < 13; itemlen++)
        {
          if (itemlen == 0)
            itlist[num] = OCTETSTRING(1, &buf[len]);
          else
            itlist[num] = itlist[num] + OCTETSTRING(1, &buf[len]);
          len += 1;
        }
      }
      else
        TTCN_logger.log(TTCN_WARNING,
                        "Unrecognized element in encoding type 0");
    }
  }
  else
  {
    Log_not_implemented_union_field("Item_list", selection);
  }

  Log_hexdump(buf, len);
  Log_function_name_on_leave();

  return len;
}

int
Get_Encoding_Type_0(Enc__Type__0 & enct0, const unsigned char *buf,
                    Item__list::union_selection_type selection,
                    t_dat *dat, int ip_idx)
{
  unsigned char ctemp;
  int itemnum = 0;
  int len = 0;

  Log_function_name_on_enter();

  ctemp = (buf[len] >> 6) & 0x03;
  enct0.et() = BITSTRING(2, &ctemp);
  ctemp = ShiftDownBit(buf[len], 5);
  enct0.gp__bit() = BITSTRING(1, &ctemp);
  ctemp = ShiftDownBit(buf[len], 4);
  enct0.ps__bit() = BITSTRING(1, &ctemp);
  enct0.cc() = buf[len] & 0x0F;
  len += 1;
  len += Get_LIN1_opt(&buf[len], enct0.gen__id(), (*enct0.gp__bit()) == 1);
  len += Get_XI_list(&buf[len], enct0.xi__list(),
                     enct0.cc(), enct0.ps__bit(), &itemnum, enct0.padding());
  len += Get_Item_list(&buf[len], enct0.item__list(), itemnum, selection,
                       dat, ip_idx);

  Log_hexdump(buf, len);
  Log_function_name_on_leave();

  return len;
}

int
Get_Encoding_Type_1(Enc__Type__1 & enct1, const unsigned char *buf,
                    Item__list::union_selection_type selection,
                    t_dat *dat, int ip_idx)
{
  unsigned char ctemp;
  int knum = 0, itemnum = 0, len = 0;

  Log_function_name_on_enter();

  ctemp = (buf[len] >> 6) & 0x03;
  enct1.et() = BITSTRING(2, &ctemp);
  ctemp = ShiftDownBit(buf[len], 5);
  enct1.gp__bit() = BITSTRING(1, &ctemp);
  ctemp = ShiftDownBit(buf[len], 4);
  enct1.ps__bit() = BITSTRING(1, &ctemp);
  enct1.xi1() = buf[len] & 0x0F;
  len += 1;
  len += Get_LIN1_opt(&buf[len], enct1.gen__id(), (*enct1.gp__bit()) == 1);
  len += Get_LIN1(&buf[len], enct1.ref__id());
  if ((buf[len] & 0x80) == 0)   /* 1 octet insertion mask */
  {
    for (int i = 0; i < 8; i++)
    {
      if (ShiftDownBit(buf[len], i) == 1)
        knum++;
    }
    len += Get_octetstring(&buf[len], enct1.insbitmask(), 1);
  }
  else                          /* 2 octets insertion mask */
  {
    for (int i = 0; i < 8; i++)
      if (ShiftDownBit(buf[len], i) == 1)
        knum++;
    for (int i = 0; i < 8; i++)
      if (ShiftDownBit(buf[len + 1], i) == 1)
        knum++;
    len += Get_octetstring(&buf[len], enct1.insbitmask(), 2);
  }

  /* Determine whether xi1 already contains the first index */
  if ((*enct1.ps__bit()) == 0)
  {
    knum--;
  }
  len +=
    Get_XI_list(&buf[len], enct1.xi__list(),
                knum, enct1.ps__bit(), &itemnum, enct1.padding());
  len += Get_Item_list(&buf[len], enct1.item__list(), itemnum, selection,
                       dat, ip_idx);

  Log_hexdump(buf, len);
  Log_function_name_on_leave();

  return len;
}


int
Get_Encoding_Type_2(Enc__Type__2 & enct2, const unsigned char *buf,
                    Item__list::union_selection_type selection)
{
  unsigned char ctemp;
  int len = 0;

  Log_function_name_on_enter();

  ctemp = (buf[len] >> 6) & 0x03;
  enct2.et() = BITSTRING(2, &ctemp);
  ctemp = ShiftDownBit(buf[len], 5);
  enct2.gp__bit() = BITSTRING(1, &ctemp);
  ctemp = ShiftDownBit(buf[len], 4);
  enct2.res() = BITSTRING(1, &ctemp);
  enct2.count() = buf[len] & 0x0F;
  len += 1;
  len += Get_LIN1_opt(&buf[len], enct2.gen__id(), (*enct2.gp__bit()) == 1);
  len += Get_LIN1(&buf[len], enct2.ref__id());
  if ((buf[len] & 0x80) == 0)
    len += Get_octetstring(&buf[len], enct2.rembitmask(), 1);
  else
    len += Get_octetstring(&buf[len], enct2.rembitmask(), 2);

  Log_hexdump(buf, len);
  Log_function_name_on_leave();

  return len;
}


int
Get_Encoding_Type_3(Enc__Type__3 & enct3, const unsigned char *buf,
                    Item__list::union_selection_type selection,
                    t_dat *dat, int ip_idx)
{
  unsigned char ctemp;
  int itemnum = 0;
  int knum = 0;
  int len = 0;

  Log_function_name_on_enter();

  ctemp = (buf[len] >> 6) & 0x03;
  enct3.et() = BITSTRING(2, &ctemp);
  ctemp = ShiftDownBit(buf[len], 5);
  enct3.gp__bit() = BITSTRING(1, &ctemp);
  ctemp = ShiftDownBit(buf[len], 4);
  enct3.ps__bit() = BITSTRING(1, &ctemp);
  enct3.xi1() = buf[len] & 0x0F;
  len += 1;
  len += Get_LIN1_opt(&buf[len], enct3.gen__id(), (*enct3.gp__bit()) == 1);
  len += Get_LIN1(&buf[len], enct3.ref__id());
  if ((buf[len] & 0x80) == 0)
    len += Get_octetstring(&buf[len], enct3.rembitmask(), 1);
  else
    len += Get_octetstring(&buf[len], enct3.rembitmask(), 2);

  if ((buf[len] & 0x80) == 0)   /* 1 octet insertion mask */
  {
    for (int i = 0; i < 8; i++)
    {
      if (ShiftDownBit(buf[len], i) == 1)
        knum++;
    }
    len += Get_octetstring(&buf[len], enct3.insbitmask(), 1);
  }
  else                          /* 2 octets insertion mask */
  {
    for (int i = 0; i < 8; i++)
      if (ShiftDownBit(buf[len], i) == 1)
        knum++;
    for (int i = 0; i < 8; i++)
      if (ShiftDownBit(buf[len + 1], i) == 1)
        knum++;
    len += Get_octetstring(&buf[len], enct3.insbitmask(), 2);
  }

  /* Determine whether xi1 already contains the first index */
  if ((*enct3.ps__bit()) == 0)
  {
    knum--;
  }
  len += Get_XI_list(&buf[len], enct3.xi__list(),
                     knum, enct3.ps__bit(), &itemnum, enct3.padding());
  len += Get_Item_list(&buf[len], enct3.item__list(), itemnum, selection,
                       dat, ip_idx);

  Log_hexdump(buf, len);
  Log_function_name_on_leave();

  return len;
}

int
Get_Compr_head_list(Compr__head__list & chl, const unsigned char *buf,
                    Item__list::union_selection_type selection,
                    t_dat *dat, int ip_idx)
{
  int len = 0;

  Log_function_name_on_enter();

  if ((buf[len] & 0xC0) == 0)
    len += Get_Encoding_Type_0(chl.enctype0(), &buf[len], selection,
                               dat, ip_idx);
  else if ((buf[len] & 0xC0) == 0x40)
    len += Get_Encoding_Type_1(chl.enctype1(), &buf[len], selection,
                               dat, ip_idx);
  else if ((buf[len] & 0xC0) == 0x80)
    len += Get_Encoding_Type_2(chl.enctype2(), &buf[len], selection);
  else
    len += Get_Encoding_Type_3(chl.enctype3(), &buf[len], selection,
                               dat, ip_idx);

  Log_hexdump(buf, len);
  Log_function_name_on_leave();

  return len;
}

int
Get_ipv4_dynamic(const unsigned char *buf, IPv4__Dynamic & chain,
                 t_dat *dat, int ip_level)
{
  int len = 0;
  unsigned char ctemp;

  Log_function_name_on_enter();

  len += Get_LIN1(&buf[len], chain.tos());
  len += Get_LIN1(&buf[len], chain.ttl());
  len += Get_LIN2_BO_LAST(&buf[len], chain.identification());
  ctemp = ShiftDownBit(buf[len], 7);
  chain.df__bit() = BITSTRING(1, &ctemp);
  ctemp = ShiftDownBit(buf[len], 6);
  chain.rnd__bit() = BITSTRING(1, &ctemp);
  dat->context.ip__ctx()[ip_level].rnd__bit() = BOOLEAN(ctemp);
  ctemp = ShiftDownBit(buf[len], 5);
  chain.nbo__bit() = BITSTRING(1, &ctemp);
  ctemp = buf[len] & 0x1F;
  chain.reserved() = BITSTRING(5, &ctemp);
  len += 1;
  len += Get_Encoding_Type_0(chain.genextheadlist(),
                             &buf[len], Item__list::ALT_ip__item__list,
                             dat, ip_level);

  Log_hexdump(buf, len);
  Log_function_name_on_leave();

  return len;
}

int
Get_ipv4_static(const unsigned char *buf, IPv4__Static & chain)
{
  int len = 0;

  Log_function_name_on_enter();

  chain.version() = (buf[len] >> 4) & 0x0F;
  chain.reserved() = buf[len] & 0x0F;
  len += 1;
  len += Get_LIN1(&buf[len], chain.proto());
  len += Get_octetstring(&buf[len], chain.srcaddr(), 4);
  len += Get_octetstring(&buf[len], chain.dstaddr(), 4);

  Log_hexdump(buf, len);
  Log_function_name_on_leave();

  return len;
}

int
Get_ipv6_static(const unsigned char *buf, IPv6__Static & chain)
{
  int len = 0;

  Log_function_name_on_enter();

  chain.version() = (buf[len] >> 4) & 0x0F;
  chain.flowlabel() =
    ((buf[len] & 0x0F) << 16) + (buf[len + 1] << 8) + buf[len + 2];
  len += 3;
  len += Get_LIN1(&buf[len], chain.nexthead());
  len += Get_octetstring(&buf[len], chain.srcaddr(), 16);
  len += Get_octetstring(&buf[len], chain.dstaddr(), 16);

  Log_hexdump(buf, len);
  Log_function_name_on_leave();

  return len;
}

int
Get_ipv6_dynamic(const unsigned char *buf, IPv6__Dynamic & chain,
                 t_dat *dat, int ip_level)
{
  int len = 0;

  Log_function_name_on_enter();

  len += Get_LIN1(&buf[len], chain.trafficclass());
  len += Get_LIN1(&buf[len], chain.hoplimit());
  len += Get_Encoding_Type_0(chain.genextheadlist(), &buf[len],
                             Item__list::ALT_ip__item__list, dat, ip_level);

  Log_hexdump(buf, len);
  Log_function_name_on_leave();

  return len;
}

int
Get_udp_static(const unsigned char *buf, UDP__Static & chain)
{
  int len = 0;

  Log_function_name_on_enter();

  len += Get_LIN2_BO_LAST(&buf[len], chain.srcport());
  len += Get_LIN2_BO_LAST(&buf[len], chain.dstport());

  Log_hexdump(buf, len);
  Log_function_name_on_leave();

  return len;
}

int
Get_udp_dynamic_wo_sn(const unsigned char *buf, 
                      UDP__Dynamic & chain, t_dat *dat)
{
  int len = 0;

  Log_function_name_on_enter();

  len += Get_LIN2_BO_LAST(&buf[len], chain.cksum());
  if (chain.cksum() == 0)
  {
    /* Checksum value 0 means that no UDP checksum is present in the
      forthcoming compressed packets. */
    dat->context.udp__ctx().udp__cksum() = false;
  }

  Log_hexdump(buf, len);
  Log_function_name_on_leave();

  return len;
}

int
Get_udp_dynamic_w_sn(const unsigned char *buf, 
                     Profile2__UDP__Dynamic & chain, t_dat *dat)
{
  int len = 0;

  Log_function_name_on_enter();

  len += Get_LIN2_BO_LAST(&buf[len], chain.cksum());
  len += Get_LIN2_BO_LAST(&buf[len], chain.udp__sn());
  if (chain.cksum() == 0)
  {
    /* Checksum value 0 means that no UDP checksum is present in the
      forthcoming compressed packets. */
    dat->context.udp__ctx().udp__cksum() = false;
  }

  Log_hexdump(buf, len);
  Log_function_name_on_leave();

  return len;
}

int
Get_rtp_static(const unsigned char *buf, RTP__Static & chain)
{
  int len = 0;

  Log_function_name_on_enter();

  len += Get_octetstring(&buf[len], chain.ssrc(), 4);

  Log_hexdump(buf, len);
  Log_function_name_on_leave();

  return len;
}

int
Get_rtp_dynamic(const unsigned char *buf, RTP__Dynamic & chain,
                t_dat *dat)
{
  int len = 0;
  unsigned char ctemp;

  Log_function_name_on_enter();

  chain.vfield() = (buf[len] >> 6) & 0x03;
  ctemp = ShiftDownBit(buf[len], 5);
  chain.pbit() = BITSTRING(1, &ctemp);
  ctemp = ShiftDownBit(buf[len], 4);
  chain.rxbit() = BITSTRING(1, &ctemp);
  chain.ccfield() = buf[len] & 0x0F;
  len += 1;
  ctemp = ShiftDownBit(buf[len], 7);
  chain.mbit() = BITSTRING(1, &ctemp);
  chain.ptfield() = buf[len] & 0x7F;
  len += 1;
  len += Get_LIN2_BO_LAST(&buf[len], chain.rtpseqnum());
  len += Get_LIN4_BO_LAST(&buf[len], chain.rtpts());
  len += Get_Encoding_Type_0(chain.gencsrclist(),
                             &buf[len], Item__list::ALT_csrc__item__list,
                             dat, 0);
  if ((*chain.rxbit()) == 1)
  {
    Rx__Field & rxfield = chain.rx__field();

    ctemp = (buf[len] >> 5) & 0x07;
    rxfield.reserved() = BITSTRING(3, &ctemp);
    ctemp = ShiftDownBit(buf[len], 4);
    rxfield.xbit() = BITSTRING(1, &ctemp);
    rxfield.mode() = (buf[len] >> 2) & 0x03;
    ctemp = ShiftDownBit(buf[len], 1);
    rxfield.tisbit() = BITSTRING(1, &ctemp);
    ctemp = ShiftDownBit(buf[len], 0);
    rxfield.tssbit() = BITSTRING(1, &ctemp);
    len += 1;
    len += Get_SDVL_field_opt(&buf[len], chain.ts__stride(),
                              *rxfield.tssbit() == 1);
    len += Get_SDVL_field_opt(&buf[len], chain.time__stride(),
                              *rxfield.tisbit() == 1);
  }
  else
  {
    chain.rx__field() = OMIT_VALUE;
    chain.ts__stride() = OMIT_VALUE;
    chain.time__stride() = OMIT_VALUE;
  }

  Log_hexdump(buf, len);
  Log_function_name_on_leave();

  return len;
}

/* Reads the compressed IP-ID field after the base header */
int
Get_IP_ID_compressed_opt(unsigned char *buf,
                         OPTIONAL < LIN2__BO__LAST > &field,
                         t_dat *dat, int ip_level)
{
  int len = 0;

  Log_function_name_on_enter();

  if (dat->context.pkt() != Packet__type::IR && dat->context.pkt() != Packet__type::IR__DYN)
  {
    int ip_idx = getIPidx(dat, ip_level);
    TTCN_logger.log(TTCN_DEBUG, "iplevel = %d, ip_idx = %d", ip_level, ip_idx);
    
    if (ip_idx >= 0)
    {
      if (dat->context.ip__ctx()[ip_idx].rnd__bit())
        len += Get_LIN2_BO_LAST(&buf[len], field());
      else 
        field = OMIT_VALUE;
    }
    else
      field = OMIT_VALUE;
  }
  else 
    field = OMIT_VALUE;

  Log_hexdump(buf, len);
  Log_function_name_on_leave();

  return len;
}

/* Reads the AH data field after the base header */
int
Get_AH_data_opt(unsigned char *buf,
                OPTIONAL < OCTETSTRING > &field, t_dat *dat,
                int ip_level)
{
  int len = 0;
  int ip_idx = getIPidx(dat, ip_level);

  Log_function_name_on_enter();
  
  TTCN_logger.log(TTCN_DEBUG, "iplevel = %d, ip_idx = %d", ip_level, ip_idx);

  /* AH present */
  if (ip_idx >= 0)
  {
    int ah_len = dat->compressed_ah_data_len[ip_idx];

    if (ah_len > 0)    
      len += Get_octetstring(&buf[len], field(), ah_len);
    else
      field = OMIT_VALUE;
  }
  else
    field = OMIT_VALUE;

  Log_hexdump(buf, len);
  Log_function_name_on_leave();

  return len;
}

/* Reads the GRE checksum field after the base header */
int
Get_GRE_cksum_opt(unsigned char *buf,
                  OPTIONAL < LIN2__BO__LAST > &field, t_dat *dat,
                  int ip_level)
{
  int len = 0;
  int ip_idx = getIPidx(dat, ip_level);

  Log_function_name_on_enter();
  
  TTCN_logger.log(TTCN_DEBUG, "iplevel = %d, ip_idx = %d", ip_level, ip_idx);

  /* Compressed packet and GRE present */
  if (ip_idx >= 0)
  {
    if (dat->ipx_gre_cksum_present[ip_idx])
      len += Get_LIN2_BO_LAST(&buf[len], field());
    else
      field = OMIT_VALUE;
  }
  else
    field = OMIT_VALUE;

  Log_hexdump(buf, len);
  Log_function_name_on_leave();

  return len;
}

/* Reads the UDP checksum field after the base header */
int
Get_UDP_cksum_opt(unsigned char *buf,
                  OPTIONAL < LIN2__BO__LAST > &field, t_dat *dat)
{
  int len = 0;

  Log_function_name_on_enter();

  if (dat->context.pkt() != Packet__type::IR && dat->context.pkt() != Packet__type::IR__DYN &&
      dat->context.udp__ctx().udp__cksum())
  {
    len += Get_LIN2_BO_LAST(&buf[len], field());
  }
  else
    field = OMIT_VALUE;

  Log_hexdump(buf, len);
  Log_function_name_on_leave();

  return len;
}

int
Get_AEGSeqnum_opt(OPTIONAL < AEGSeqnum > &seqn, const unsigned char *buf,
                  bool hasvalue)
{
  unsigned char ctemp;
  int len = 0;

  Log_function_name_on_enter();

  if (hasvalue)
  {
    if ((buf[len] & 0x80) == 0)
    {
      ctemp = ShiftDownBit(buf[len], 7);
      seqn().short__form().ind() = BITSTRING(1, &ctemp);
      seqn().short__form().lsb__of__seqnum() = buf[len] & 0x7F;
      len += 1;
    }
    else
    {
      ctemp = ShiftDownBit(buf[len], 7);
      seqn().long__form().ind() = BITSTRING(1, &ctemp);
      seqn().long__form().lsb__of__seqnum() =
        ((buf[len] & 0x7F) << 24) +
        (buf[len + 1] << 16) + (buf[len + 2] << 8) + buf[len + 3];
      len += 4;
    }
  }
  else
    seqn = OMIT_VALUE;

  Log_hexdump(buf, len);
  Log_function_name_on_leave();

  return len;
}

int
Get_Inner_IP_flags_opt(unsigned char *buf,
                       OPTIONAL < Inner__IP__flags > &field,
                       t_dat *dat, bool hasvalue)
{
  int len = 0;
  int ip_idx = getIPidx(dat, Inner_IP);
  unsigned char ctemp;

  Log_function_name_on_enter();
  
  TTCN_logger.log(TTCN_DEBUG, "iplevel = %d, ip_idx = %d", Inner_IP, ip_idx);

  if (hasvalue)
  {
    ctemp = ShiftDownBit(buf[len], 7);
    field().tos() = BITSTRING(1, &ctemp);
    ctemp = ShiftDownBit(buf[len], 6);
    field().ttl() = BITSTRING(1, &ctemp);
    ctemp = ShiftDownBit(buf[len], 5);
    field().df() = BITSTRING(1, &ctemp);
    ctemp = ShiftDownBit(buf[len], 4);
    field().pr() = BITSTRING(1, &ctemp);
    ctemp = ShiftDownBit(buf[len], 3);
    field().ipx() = BITSTRING(1, &ctemp);
    ctemp = ShiftDownBit(buf[len], 2);
    field().nbo() = BITSTRING(1, &ctemp);
    ctemp = ShiftDownBit(buf[len], 1);
    field().rnd__bit() = BITSTRING(1, &ctemp);
    dat->context.ip__ctx()[ip_idx].rnd__bit() = BOOLEAN(ctemp);
    ctemp = ShiftDownBit(buf[len], 0);
    field().ip2__bit() = BITSTRING(1, &ctemp);
    len += 1;
  }
  else
    field = OMIT_VALUE;

  Log_hexdump(buf, len);
  Log_function_name_on_leave();

  return len;
}

int
Get_Outer_IP_flags_opt(unsigned char *buf,
                       OPTIONAL < Outer__IP__flags > &field,
                       t_dat *dat, bool hasvalue)
{
  int len = 0;
  int ip_idx = getIPidx(dat, Outer_IP);
  unsigned char ctemp;

  Log_function_name_on_enter();
  
  TTCN_logger.log(TTCN_DEBUG, "iplevel = %d, ip_idx = %d", Outer_IP, ip_idx);

  if (hasvalue)
  {
    ctemp = ShiftDownBit(buf[len], 7);
    field().tos2() = BITSTRING(1, &ctemp);
    ctemp = ShiftDownBit(buf[len], 6);
    field().ttl2() = BITSTRING(1, &ctemp);
    ctemp = ShiftDownBit(buf[len], 5);
    field().df2() = BITSTRING(1, &ctemp);
    ctemp = ShiftDownBit(buf[len], 4);
    field().pr2() = BITSTRING(1, &ctemp);
    ctemp = ShiftDownBit(buf[len], 3);
    field().ipx2() = BITSTRING(1, &ctemp);
    ctemp = ShiftDownBit(buf[len], 2);
    field().nbo2() = BITSTRING(1, &ctemp);
    ctemp = ShiftDownBit(buf[len], 1);
    field().rnd2() = BITSTRING(1, &ctemp);
    dat->context.ip__ctx()[ip_idx].rnd__bit() = BOOLEAN(ctemp);
    ctemp = ShiftDownBit(buf[len], 0);
    field().i2__bit() = BITSTRING(1, &ctemp);
    len += 1;
  }
  else
    field = OMIT_VALUE;

  Log_hexdump(buf, len);
  Log_function_name_on_leave();

  return len;
}

int
Get_ipx_headers_opt(unsigned char *buf, OPTIONAL < IP__Ext__heads > &field,
                    t_dat *dat, bool hasvalue, int ip_idx)
{
  int len = 0;
  unsigned char ctemp;

  Log_function_name_on_enter();
  
  TTCN_logger.log(TTCN_DEBUG, "present = %d, ip_idx = %d", hasvalue, ip_idx);

  if (hasvalue)
  {
    ctemp = ShiftDownBit(buf[len], 7);
    field().cl() = BITSTRING(1, &ctemp);
    ctemp = ShiftDownBit(buf[len], 6);
    field().aseq() = BITSTRING(1, &ctemp);
    ctemp = ShiftDownBit(buf[len], 5);
    field().eseq() = BITSTRING(1, &ctemp);
    ctemp = ShiftDownBit(buf[len], 4);
    field().gseq() = BITSTRING(1, &ctemp);
    ctemp = buf[len] & 0x0F;
    field().res() = BITSTRING(4, &ctemp);
    len += 1;

    len +=
      Get_AEGSeqnum_opt(field().ah__seq(), &buf[len], *field().aseq() == 1);
    len +=
      Get_AEGSeqnum_opt(field().esp__seq(), &buf[len], *field().eseq() == 1);
    len +=
      Get_AEGSeqnum_opt(field().gre__seq(), &buf[len], *field().gseq() == 1);

    if (*field().cl() == 1)
    {
      len += Get_Compr_head_list(field().compr__head__list(), &buf[len],
                                 Item__list::ALT_ip__item__list,
                                 dat, ip_idx);
    }
    else
      field().compr__head__list() = OMIT_VALUE;
  }
  else
    field = OMIT_VALUE;

  Log_hexdump(buf, len);
  Log_function_name_on_leave();

  return len;
}

int
Get_Inner_IP_fields_opt(unsigned char *buf,
                        OPTIONAL < Inner__IP__fields > &field,
                        OPTIONAL < Inner__IP__flags > &flags,
                        t_dat *dat, bool hasvalue)
{
  int len = 0;

  Log_function_name_on_enter();

  if (hasvalue)
  {
    len += Get_LIN1_opt(&buf[len], field().tos(), (*flags().tos()) == 1);
    len += Get_LIN1_opt(&buf[len], field().ttl(), (*flags().ttl()) == 1);
    len += Get_LIN1_opt(&buf[len], field().proto(), (*flags().pr()) == 1);
    len += Get_ipx_headers_opt(&buf[len], field().ext__heads(), dat,
                               (*flags().ipx()) == 1, getIPidx(dat, Inner_IP));
  }
  else
    field = OMIT_VALUE;

  Log_hexdump(buf, len);
  Log_function_name_on_leave();

  return len;
}

int
Get_Outer_IP_fields_opt(unsigned char *buf,
                        OPTIONAL < Outer__IP__fields > &field,
                        OPTIONAL < Outer__IP__flags > &flags,
                        t_dat *dat, bool hasvalue)
{
  int len = 0;

  Log_function_name_on_enter();

  if (hasvalue)
  {
    len += Get_LIN1_opt(&buf[len], field().tos(), (*flags().tos2()) == 1);
    len += Get_LIN1_opt(&buf[len], field().ttl(), (*flags().ttl2()) == 1);
    len += Get_LIN1_opt(&buf[len], field().proto(), (*flags().pr2()) == 1);
    len += Get_ipx_headers_opt(&buf[len], field().ext__heads(), dat,
                               (*flags().ipx2()) == 1, getIPidx(dat, Outer_IP));
    len += Get_LIN2_BO_LAST_opt(&buf[len], field().ip__id(),
                                (*flags().i2__bit()) == 1);
  }
  else
    field = OMIT_VALUE;

  Log_hexdump(buf, len);
  Log_function_name_on_leave();

  return len;
}

int
Get_RTP_flags_fields_opt(unsigned char *buf,
                         OPTIONAL < RTP__flags__fields > &field,
                         t_dat *dat, bool hasvalue)
{
  int len = 0;
  unsigned char ctemp;

  Log_function_name_on_enter();

  if (hasvalue)
  {
    field().mode() = (buf[len] >> 6) & 0x03;
    ctemp = ShiftDownBit(buf[len], 5);
    field().r__pt() = BITSTRING(1, &ctemp);
    ctemp = ShiftDownBit(buf[len], 4);
    field().m__bit() = BITSTRING(1, &ctemp);
    ctemp = ShiftDownBit(buf[len], 3);
    field().r__x() = BITSTRING(1, &ctemp);
    ctemp = ShiftDownBit(buf[len], 2);
    field().csrc() = BITSTRING(1, &ctemp);
    ctemp = ShiftDownBit(buf[len], 1);
    field().tss() = BITSTRING(1, &ctemp);
    ctemp = ShiftDownBit(buf[len], 0);
    field().tis() = BITSTRING(1, &ctemp);
    len += 1;

    if (*field().r__pt() == 1)
    {
      ctemp = ShiftDownBit(buf[len], 7);
      field().flags()().r__p() = BITSTRING(1, &ctemp);
      field().flags()().rtp__pt() = buf[len] & 0x7F;
      len += 1;
    }
    else
      field().flags() = OMIT_VALUE;

    if (*field().csrc() == 1)
    {
      len += Get_Compr_head_list(field().csrc__list(),
                                 &buf[len], Item__list::ALT_csrc__item__list,
                                 dat, 0);
    }
    else
      field().csrc__list() = OMIT_VALUE;

    len += Get_SDVL_field_opt(&buf[len], field().ts__stride(),
                              *field().tss() == 1);
    len += Get_SDVL_field_opt(&buf[len], field().time__stride(),
                              *field().tis() == 1);
  }
  else
    field = OMIT_VALUE;

  Log_hexdump(buf, len);
  Log_function_name_on_leave();

  return len;
}

int
Get_Profile0_ROHC_Header(unsigned char *buf, t_dat *dat, int length,
                         Profile0__headers & p_header, int cid)
{
  unsigned char ctemp;
  int len = 0;

  Log_function_name_on_enter();

  if (isIR(buf[len]))
  {
    Profile0__IR__packet & bhead = p_header.ir();

    bhead.cid() = cid;
    ctemp = (buf[len] >> 1) & 0x7F;
    bhead.type__ind() = BITSTRING(7, &ctemp);
    ctemp = ShiftDownBit(buf[len], 0);
    bhead.d() = BITSTRING(1, &ctemp);
    len += 1;
    len += Get_LIN1(&buf[len], bhead.profile());
    dat->context.profile() = bhead.profile();
    len += Get_LIN1(&buf[len], bhead.crc());
    Get_octetstring(&buf[len], bhead.orig__packet(), length - len);
    len = length;
  }
  else
  {
    Profile0__normal__packet & bhead = p_header.normal();
    bhead.cid() = cid;
    Get_octetstring(&buf[len], bhead.orig__packet(), length - len);
    len = length;
  }

  Log_hexdump(buf, len);
  Log_function_name_on_leave();

  return len;
}

int
Get_Profile1_Dynamic_Chain(Dynamic__Chain & dynch,
                           const unsigned char *buf, t_dat *dat)
{
  int len = 0, num;

  Log_function_name_on_enter();

  for (num = 0; num < dat->context.ip__ctx().size_of(); num++)
  {
    if (dat->context.ip__ctx()[num].version() == 4)
    {
      len += Get_ipv4_dynamic(&buf[len], dynch[num].ipv4__dyn(), dat, num);
    }
    else if (dat->context.ip__ctx()[num].version() == 6)
    {
      len += Get_ipv6_dynamic(&buf[len], dynch[num].ipv6__dyn(), dat, num);
    }
    else
    {
      int ipver = dat->context.ip__ctx()[num].version();
      TTCN_error("Invalid IP version %u on level %d", ipver, num);
    }
  }

  num = dynch.size_of();
  len += Get_udp_dynamic_wo_sn(&buf[len], dynch[num].udp__dyn(), dat);

  num = dynch.size_of();
  len += Get_rtp_dynamic(&buf[len], dynch[num].rtp__dyn(), dat);

  Log_hexdump(buf, len);
  Log_function_name_on_leave();

  return len;
}

int
Get_Profile1_Static_Chain(unsigned char *buf, Static__Chain & chain,
                          t_dat *dat)
{
  int len = 0, num = 0, nextheader = 0;

  Log_function_name_on_enter();
  
  while (nextheader != c__ip__proto__udp)
  {
    if (((buf[len] >> 4) & 0x0F) == 4)
    {
      initIPcontext(dat, num, 4);
      len += Get_ipv4_static(&buf[len], chain[num].ipv4__stat());
      nextheader = chain[num].ipv4__stat().proto();
    }
    else if (((buf[len] >> 4) & 0x0F) == 6)
    {
      initIPcontext(dat, num, 6);
      len += Get_ipv6_static(&buf[len], chain[num].ipv6__stat());
      nextheader = chain[num].ipv6__stat().nexthead();
    }
    else
    {
      TTCN_error("Invalid IP version %u on level %u (nextheader = %d)", 
                  buf[len], num, nextheader);
    }
    num += 1;
  }

  len += Get_udp_static(&buf[len], chain[num].udp__stat());
  num += 1;

  len += Get_rtp_static(&buf[len], chain[num].rtp__stat());

  Log_hexdump(buf, len);
  Log_function_name_on_leave();

  return len;
}

int
Get_Profile1_extension0(unsigned char *buf, Extension0 & ext,
                        t_dat *dat)
{
  int len = 0;
  unsigned char ctemp;

  Log_function_name_on_enter();

  ctemp = (buf[len] >> 6) & 0x03;
  ext.ext__type() = BITSTRING(2, &ctemp);
  ext.sn() = (buf[len] >> 3) & 0x07;
  ext.plust() = buf[len] & 0x07;
  len += 1;

  Log_hexdump(buf, len);
  Log_function_name_on_leave();

  return len;
}

int
Get_Profile1_extension1(unsigned char *buf, Extension1 & ext,
                        t_dat *dat)
{
  int len = 0;
  unsigned char ctemp;

  Log_function_name_on_enter();

  ctemp = (buf[len] >> 6) & 0x03;
  ext.ext__type() = BITSTRING(2, &ctemp);
  ext.sn() = (buf[len] >> 3) & 0x07;
  ext.plust() = buf[len] & 0x07;
  len += 1;
  ext.minust() = buf[len];
  len += 1;

  Log_hexdump(buf, len);
  Log_function_name_on_leave();

  return len;
}

int
Get_Profile1_extension2(unsigned char *buf, Extension2 & ext,
                        t_dat *dat)
{
  int len = 0;
  unsigned char ctemp;

  Log_function_name_on_enter();

  ctemp = (buf[len] >> 6) & 0x03;
  ext.ext__type() = BITSTRING(2, &ctemp);
  ext.sn() = (buf[len] >> 3) & 0x07;
  ext.plust() = ((buf[len] & 0x07) << 8) + (buf[len + 1] & 0xFF);
  len += 2;
  ext.minust() = buf[len];
  len += 1;

  Log_hexdump(buf, len);
  Log_function_name_on_leave();

  return len;
}

int
Get_Profile1_extension3(unsigned char *buf, Extension3 & ext,
                        t_dat *dat)
{
  int len = 0;
  int ts_length = 0;
  unsigned char ctemp;

  Log_function_name_on_enter();

  ctemp = (buf[len] >> 6) & 0x03;
  ext.ext__type() = BITSTRING(2, &ctemp);
  ctemp = ShiftDownBit(buf[len], 5);
  ext.s__bit() = BITSTRING(1, &ctemp);
  ctemp = ShiftDownBit(buf[len], 4);
  ext.r__ts__bit() = BITSTRING(1, &ctemp);
  ctemp = ShiftDownBit(buf[len], 3);
  ext.tsc__bit() = BITSTRING(1, &ctemp);
  ctemp = ShiftDownBit(buf[len], 2);
  ext.i__bit() = BITSTRING(1, &ctemp);
  ctemp = ShiftDownBit(buf[len], 1);
  ext.ip__bit() = BITSTRING(1, &ctemp);
  ctemp = ShiftDownBit(buf[len], 0);
  ext.rtp__bit() = BITSTRING(1, &ctemp);
  len += 1;

  len += Get_Inner_IP_flags_opt(&buf[len], ext.inner__ip__flags(),
                                dat, *ext.ip__bit() == 1);

  len += Get_Outer_IP_flags_opt(&buf[len], ext.outer__ip__flags(), dat,
                                (ext.inner__ip__flags().ispresent() &&
                                 (*ext.inner__ip__flags()().ip2__bit()) == 1));

  len += Get_LIN1_opt(&buf[len], ext.sn(), *ext.s__bit() == 1);
  ts_length = Get_SDVL_field_opt(&buf[len], ext.ts(), *ext.r__ts__bit() == 1);
  if (ts_length > 0) 
  {
    ext.ts__length()() = ts_length;
  } 
  else
  {
    ext.ts__length() = OMIT_VALUE;
  }
  len += ts_length;
  len += Get_Inner_IP_fields_opt(&buf[len], ext.inner__ip__hdr(),
                                 ext.inner__ip__flags(), dat,
                                 *ext.ip__bit() == 1);

  len += Get_LIN2_BO_LAST_opt(&buf[len], ext.ip__id(), *ext.i__bit() == 1);
  len += Get_Outer_IP_fields_opt(&buf[len], ext.outer__ip__hdr(),
                                 ext.outer__ip__flags(), dat,
                                 (ext.inner__ip__flags().ispresent() &&
                                 (*ext.inner__ip__flags()().ip2__bit()) == 1));

  len += Get_RTP_flags_fields_opt(&buf[len], ext.rtp__fl__fi(), dat,
                                  *ext.rtp__bit() == 1);

  Log_hexdump(buf, len);
  Log_function_name_on_leave();

  return len;
}

int
Get_Profile1_extensions_opt(unsigned char *buf,
                            OPTIONAL < Profile1__Extension > &ext,
                            t_dat *dat, bool hasvalue)
{
  int len = 0;

  Log_function_name_on_enter();

  if (hasvalue)
  {
    if ((buf[len] & 0xC0) == 0)
    {
      len += Get_Profile1_extension0(&buf[len], ext().ext0(), dat);
    }
    else if ((buf[len] & 0xC0) == 0x40)
    {
      len += Get_Profile1_extension1(&buf[len], ext().ext1(), dat);
    }
    else if ((buf[len] & 0xC0) == 0x80)
    {
      len += Get_Profile1_extension2(&buf[len], ext().ext2(), dat);
    }
    else                        /* if ((buf[len] & 0xC0) == 0xC0) */
    {
      len += Get_Profile1_extension3(&buf[len], ext().ext3(), dat);
    }
  }
  else
  {
    ext = OMIT_VALUE;
  }

  Log_hexdump(buf, len);
  Log_function_name_on_leave();

  return len;
}

int
Get_Profile1_ROHC_Header_ir(unsigned char *buf, Profile1__IR__header & bhead,
                            t_dat *dat, int cid)
{
  int len = 0;
  unsigned char ctemp;

  Log_function_name_on_enter();

  dat->context.pkt() = Packet__type(Packet__type::IR);
  bhead.cid() = cid;
  ctemp = (buf[len] >> 1) & 0x7F;
  bhead.type__ind() = BITSTRING(7, &ctemp);
  ctemp = ShiftDownBit(buf[len], 0);
  bhead.d() = BITSTRING(1, &ctemp);
  len += 1;

  len += Get_LIN1(&buf[len], bhead.profile());
  dat->context.profile() = bhead.profile();
  len += Get_LIN1(&buf[len], bhead.crc());

  len += Get_Profile1_Static_Chain(&buf[len], bhead.stat__chain(), dat);

  if (*bhead.d() == 1)
    len += Get_Profile1_Dynamic_Chain(bhead.dyn__chain(), &buf[len], dat);
  else
    bhead.dyn__chain() = OMIT_VALUE;

  Log_hexdump(buf, len);
  Log_function_name_on_leave();

  return len;
}

int
Get_Profile1_ROHC_Header_irdyn(unsigned char *buf,
                               Profile1__IR__DYN__header & bhead,
                               t_dat *dat, int cid)
{
  int len = 0;

  Log_function_name_on_enter();

  dat->context.pkt() = Packet__type(Packet__type::IR__DYN);
  bhead.cid() = cid;
  bhead.type__ind() = BITSTRING(8, &buf[len]);
  len += 1;
  len += Get_LIN1(&buf[len], bhead.profile());
  dat->context.profile() = bhead.profile();
  len += Get_LIN1(&buf[len], bhead.crc());

  len += Get_Profile1_Dynamic_Chain(bhead.dyn__chain(), &buf[len], dat);

  Log_hexdump(buf, len);
  Log_function_name_on_leave();

  return len;
}

int
Get_Profile1_ROHC_Header_r0(unsigned char *buf,
                            Profile1__R__0__header & bhead,
                            t_dat *dat, int cid)
{
  int len = 0;
  unsigned char ctemp;

  Log_function_name_on_enter();

  dat->context.pkt() = Packet__type(Packet__type::R__0);
  
  bhead.cid() = cid;
  ctemp = (buf[len] >> 6) & 0x03;
  bhead.type__ind() = BITSTRING(2, &ctemp);
  bhead.sn() = buf[len] & 0x3F;
  len += 1;

  Log_hexdump(buf, len);
  Log_function_name_on_leave();

  return len;
}

int
Get_Profile1_ROHC_Header_r0crc(unsigned char *buf,
                               Profile1__R__0__CRC__header & bhead,
                               t_dat *dat, int cid)
{
  int len = 0;
  unsigned char ctemp;

  Log_function_name_on_enter();

  dat->context.pkt() = Packet__type(Packet__type::R__0__CRC);
  
  bhead.cid() = cid;
  ctemp = (buf[len] >> 6) & 0x03;
  bhead.type__ind() = BITSTRING(2, &ctemp);
  ctemp = (buf[len] << 1) & 0x7E;
  len += 1;
  ctemp += ShiftDownBit(buf[len], 7);
  bhead.sn() = ctemp;
  bhead.crc() = buf[len] & 0x7F;
  len += 1;

  Log_hexdump(buf, len);
  Log_function_name_on_leave();

  return len;
}

int
Get_Profile1_ROHC_Header_uo0(unsigned char *buf,
                             Profile1__UO__0__header & bhead,
                             t_dat *dat, int cid)
{
  int len = 0;
  unsigned char ctemp;

  Log_function_name_on_enter();

  dat->context.pkt() = Packet__type(Packet__type::UO__0);
  bhead.cid() = cid;
  ctemp = ShiftDownBit(buf[len], 7);
  bhead.type__ind() = BITSTRING(1, &ctemp);
  bhead.sn() = (buf[len] >> 3) & 0x0F;
  bhead.crc() = buf[len] & 0x07;
  len += 1;

  Log_hexdump(buf, len);
  Log_function_name_on_leave();

  return len;
}

int
Get_Profile1_ROHC_Header_r1(unsigned char *buf,
                            Profile1__R__1__header & bhead,
                            t_dat *dat, int cid,
                            OPTIONAL < Profile1__Extension > &ext)
{
  int len = 0;
  unsigned char ctemp;

  Log_function_name_on_enter();

  dat->context.pkt() = Packet__type(Packet__type::R__1);
  bhead.cid() = cid;
  ctemp = (buf[len] >> 6) & 0x03;
  bhead.type__ind() = BITSTRING(2, &ctemp);
  bhead.sn() = buf[len] & 0x3F;
  len += 1;
  ctemp = ShiftDownBit(buf[len], 7);
  bhead.m__bit() = BITSTRING(1, &ctemp);
  ctemp = ShiftDownBit(buf[len], 6);
  bhead.x__bit() = BITSTRING(1, &ctemp);
  bhead.ts() = buf[len] & 0x3F;
  len += 1;

  if (*bhead.x__bit())
  {
    len += Get_Profile1_extensions_opt(&buf[len], ext, dat,
                                       *bhead.x__bit());
  }

  Log_hexdump(buf, len);
  Log_function_name_on_leave();

  return len;
}

int
Get_Profile1_ROHC_Header_r1_id(unsigned char *buf,
                               Profile1__R__1__ID__header & bhead,
                               t_dat *dat, int cid,
                               OPTIONAL < Profile1__Extension > &ext)
{
  int len = 0;
  unsigned char ctemp;

  Log_function_name_on_enter();

  dat->context.pkt() =  Packet__type(Packet__type::R__1__ID);
  bhead.cid() = cid;
  ctemp = (buf[len] >> 6) & 0x03;
  bhead.type__ind() = BITSTRING(2, &ctemp);
  bhead.sn() = buf[len] & 0x3F;
  len += 1;
  ctemp = ShiftDownBit(buf[len], 7);
  bhead.m__bit() = BITSTRING(1, &ctemp);
  ctemp = ShiftDownBit(buf[len], 6);
  bhead.x__bit() = BITSTRING(1, &ctemp);
  ctemp = ShiftDownBit(buf[len], 5);
  bhead.t__bit() = BITSTRING(1, &ctemp);
  bhead.ip__id() = buf[len] & 0x1F;
  len += 1;

  if (*bhead.x__bit())
  {
    len += Get_Profile1_extensions_opt(&buf[len], ext, dat,
                                       *bhead.x__bit());
  }

  Log_hexdump(buf, len);
  Log_function_name_on_leave();

  return len;
}

int
Get_Profile1_ROHC_Header_r1_ts(unsigned char *buf,
                               Profile1__R__1__TS__header & bhead,
                               t_dat *dat, int cid,
                               OPTIONAL < Profile1__Extension > &ext)
{
  int len = 0;
  unsigned char ctemp;

  Log_function_name_on_enter();

  dat->context.pkt() =  Packet__type(Packet__type::R__1__TS);
  bhead.cid() = cid;
  ctemp = (buf[len] >> 6) & 0x03;
  bhead.type__ind() = BITSTRING(2, &ctemp);
  bhead.sn() = buf[len] & 0x3F;
  len += 1;
  ctemp = ShiftDownBit(buf[len], 7);
  bhead.m__bit() = BITSTRING(1, &ctemp);
  ctemp = ShiftDownBit(buf[len], 6);
  bhead.x__bit() = BITSTRING(1, &ctemp);
  ctemp = ShiftDownBit(buf[len], 5);
  bhead.t__bit() = BITSTRING(1, &ctemp);
  bhead.ts() = buf[len] & 0x1F;
  len += 1;

  if (*bhead.x__bit())
  {
    len += Get_Profile1_extensions_opt(&buf[len], ext, dat,
                                       *bhead.x__bit());
  }

  Log_hexdump(buf, len);
  Log_function_name_on_leave();

  return len;
}

int
Get_Profile1_ROHC_Header_uo1(unsigned char *buf,
                             Profile1__UO__1__header & bhead,
                             t_dat *dat, int cid)
{
  int len = 0;
  unsigned char ctemp;

  Log_function_name_on_enter();

  dat->context.pkt() =  Packet__type(Packet__type::UO__1);
  bhead.cid() = cid;
  ctemp = (buf[len] >> 6) & 0x03;
  bhead.type__ind() = BITSTRING(2, &ctemp);
  bhead.ts() = buf[len] & 0x3F;
  len += 1;
  ctemp = ShiftDownBit(buf[len], 7);
  bhead.m__bit() = BITSTRING(1, &ctemp);
  bhead.sn() = (buf[len] >> 3) & 0x0F;
  bhead.crc() = buf[len] & 0x07;
  len += 1;

  Log_hexdump(buf, len);
  Log_function_name_on_leave();

  return len;
}

int
Get_Profile1_ROHC_Header_uo1_id(unsigned char *buf,
                                Profile1__UO__1__ID__header & bhead,
                                t_dat *dat, int cid,
                                OPTIONAL < Profile1__Extension > &ext)
{
  int len = 0;
  unsigned char ctemp;

  Log_function_name_on_enter();

  dat->context.pkt() =  Packet__type(Packet__type::UO__1__ID);
  bhead.cid() = cid;
  ctemp = (buf[len] >> 6) & 0x03;
  bhead.type__ind() = BITSTRING(2, &ctemp);
  ctemp = ShiftDownBit(buf[len], 5);
  bhead.t__bit() = BITSTRING(1, &ctemp);
  bhead.ip__id() = buf[len] & 0x1F;
  len += 1;
  ctemp = ShiftDownBit(buf[len], 7);
  bhead.x__bit() = BITSTRING(1, &ctemp);
  bhead.sn() = (buf[len] >> 3) & 0x0F;
  bhead.crc() = buf[len] & 0x07;
  len += 1;

  if (*bhead.x__bit())
  {
    len += Get_Profile1_extensions_opt(&buf[len], ext,
                                       dat, *bhead.x__bit());
  }

  Log_hexdump(buf, len);
  Log_function_name_on_leave();

  return len;
}

int
Get_Profile1_ROHC_Header_uo1_ts(unsigned char *buf,
                                Profile1__UO__1__TS__header & bhead,
                                t_dat *dat, int cid)
{
  int len = 0;
  unsigned char ctemp;

  Log_function_name_on_enter();

  dat->context.pkt() =  Packet__type(Packet__type::UO__1__TS);
  bhead.cid() = cid;
  ctemp = (buf[len] >> 6) & 0x03;
  bhead.type__ind() = BITSTRING(2, &ctemp);
  ctemp = ShiftDownBit(buf[len], 5);
  bhead.t__bit() = BITSTRING(1, &ctemp);
  bhead.ts() = buf[len] & 0x1F;
  len += 1;
  ctemp = ShiftDownBit(buf[len], 7);
  bhead.m__bit() = BITSTRING(1, &ctemp);
  bhead.sn() = (buf[len] >> 3) & 0x0F;
  bhead.crc() = buf[len] & 0x07;
  len += 1;

  Log_hexdump(buf, len);
  Log_function_name_on_leave();

  return len;
}

int
Get_Profile1_ROHC_Header_uor2(unsigned char *buf,
                              Profile1__UOR__2__header & bhead,
                              t_dat *dat, int cid,
                              OPTIONAL < Profile1__Extension > &ext)
{
  int len = 0;
  unsigned char ctemp;

  Log_function_name_on_enter();

  dat->context.pkt() =  Packet__type(Packet__type::UOR__2);
  bhead.cid() = cid;
  ctemp = (buf[len] >> 5) & 0x07;
  bhead.type__ind() = BITSTRING(3, &ctemp);
  ctemp = (buf[len] & 0x1F) << 1;
  len += 1;
  ctemp += ShiftDownBit(buf[len], 7);
  bhead.ts() = ctemp;
  ctemp = ShiftDownBit(buf[len], 6);
  bhead.m__bit() = BITSTRING(1, &ctemp);
  bhead.sn() = buf[len] & 0x3F;
  len += 1;
  ctemp = ShiftDownBit(buf[len], 7);
  bhead.x__bit() = BITSTRING(1, &ctemp);
  bhead.crc() = buf[len] & 0x7F;
  len += 1;

  if (*bhead.x__bit())
  {
    len += Get_Profile1_extensions_opt(&buf[len], ext,
                                       dat, *bhead.x__bit());
  }

  Log_hexdump(buf, len);
  Log_function_name_on_leave();

  return len;
}

int
Get_Profile1_ROHC_Header_uor2_id(unsigned char *buf,
                                 Profile1__UOR__2__ID__header & bhead,
                                 t_dat *dat, int cid,
                                 OPTIONAL < Profile1__Extension > & ext)
{
  int len = 0;
  unsigned char ctemp;

  Log_function_name_on_enter();

  dat->context.pkt() =  Packet__type(Packet__type::UOR__2__ID);
  bhead.cid() = cid;
  ctemp = (buf[len] >> 5) & 0x07;
  bhead.type__ind() = BITSTRING(3, &ctemp);
  bhead.ip__id() = buf[len] & 0x1F;
  len += 1;
  ctemp = ShiftDownBit(buf[len], 7);
  bhead.t__bit() = BITSTRING(1, &ctemp);
  ctemp = ShiftDownBit(buf[len], 6);
  bhead.m__bit() = BITSTRING(1, &ctemp);
  bhead.sn() = buf[len] & 0x3F;
  len += 1;
  ctemp = ShiftDownBit(buf[len], 7);
  bhead.x__bit() = BITSTRING(1, &ctemp);
  bhead.crc() = buf[len] & 0x7F;
  len += 1;

  if (*bhead.x__bit())
  {
    len += Get_Profile1_extensions_opt(&buf[len], ext,
                                       dat, *bhead.x__bit());
  }

  Log_hexdump(buf, len);
  Log_function_name_on_leave();

  return len;
}

int
Get_Profile1_ROHC_Header_uor2_ts(unsigned char *buf,
                                 Profile1__UOR__2__TS__header & bhead,
                                 t_dat *dat, int cid,
                                 OPTIONAL < Profile1__Extension > & ext)
{
  int len = 0;
  unsigned char ctemp;

  Log_function_name_on_enter();

  dat->context.pkt() =  Packet__type(Packet__type::UOR__2__TS);
  bhead.cid() = cid;
  ctemp = (buf[len] >> 5) & 0x07;
  bhead.type__ind() = BITSTRING(3, &ctemp);
  bhead.ts() = buf[len] & 0x1F;
  len += 1;
  ctemp = ShiftDownBit(buf[len], 7);
  bhead.t__bit() = BITSTRING(1, &ctemp);
  ctemp = ShiftDownBit(buf[len], 6);
  bhead.m__bit() = BITSTRING(1, &ctemp);
  bhead.sn() = buf[len] & 0x3F;
  len += 1;
  ctemp = ShiftDownBit(buf[len], 7);
  bhead.x__bit() = BITSTRING(1, &ctemp);
  bhead.crc() = buf[len] & 0x7F;
  len += 1;

  if (*bhead.x__bit())
  {
    len += Get_Profile1_extensions_opt(&buf[len], ext,
                                       dat, *bhead.x__bit());
  }

  Log_hexdump(buf, len);
  Log_function_name_on_leave();

  return len;
}

int
Get_Profile1_ROHC_Header(unsigned char *buf, t_dat *dat, int length,
                         Profile1__headers & hdr, int cid)
{
  int len = 0;

  Log_function_name_on_enter();

  if (isIR(buf[len]))
  {
    len += Get_Profile1_ROHC_Header_ir(&buf[len],
                                       hdr.base__header().ir(), dat, cid);
  }
  else if (isIRDYN(buf[len]))
  {
    len += Get_Profile1_ROHC_Header_irdyn(&buf[len],
                                          hdr.base__header().ir__dyn(),
                                          dat, cid);
  }
  else if (isUOR2(buf[len]))
  {
    if (isAnyIPv4RNDunset(dat))
    {
      if (buf[len + 1] & 0x80) // T = 1
      {
        len += Get_Profile1_ROHC_Header_uor2_ts(&buf[len],
                                                hdr.base__header().
                                                uor__2__ts(), dat, cid,
                                                hdr.ext());
      }
      else // T = 0
      {
        len += Get_Profile1_ROHC_Header_uor2_id(&buf[len],
                                                hdr.base__header().
                                                uor__2__id(), dat, cid,
                                                hdr.ext());
      }
    }
    else // at least one RND is unset
    {
      len += Get_Profile1_ROHC_Header_uor2(&buf[len],
                                           hdr.base__header().uor__2(),
                                           dat, cid, hdr.ext());
    }
  }
  else if (dat->context.mode() == ROHC_mode_R)
  {
    if ((buf[len] & 0xC0) == 0)
    {
      len += Get_Profile1_ROHC_Header_r0(&buf[len],
                                         hdr.base__header().r__0(),
                                         dat, cid);
    }
    else if ((buf[len] & 0xC0) == 0x40)
    {
      len += Get_Profile1_ROHC_Header_r0crc(&buf[len],
                                            hdr.base__header().r__0__crc(),
                                            dat, cid);
    }
    else if ((buf[len] & 0xC0) == 0x80) // PT-1
    {
      if (isAnyIPv4RNDunset(dat))
      {
        if (buf[len + 1] & 0x20) // T = 1
        {
          len += Get_Profile1_ROHC_Header_r1_ts(&buf[len],
                                                hdr.base__header().r__1__ts(),
                                                dat, cid, hdr.ext());
        }
        else // T = 0
        {
          len += Get_Profile1_ROHC_Header_r1_id(&buf[len],
                                                hdr.base__header().r__1__id(),
                                                dat, cid, hdr.ext());
        }
      }
      else // at least one RND is unset
      {
        len += Get_Profile1_ROHC_Header_r1(&buf[len],
                                           hdr.base__header().r__1(),
                                           dat, cid, hdr.ext());
      }
    }
  }
  else if (dat->context.mode() != ROHC_mode_R) // U-O modes
  {
    if ((buf[len] & 0x80) == 0)
    {
      len += Get_Profile1_ROHC_Header_uo0(&buf[len],
                                          hdr.base__header().uo__0(),
                                          dat, cid);
    }
    else if ((buf[len] & 0xC0) == 0x80) // PT-1
    {
      if (isAnyIPv4RNDunset(dat))
      {
        if (buf[len] & 0x20) // T = 1
        {
          len += Get_Profile1_ROHC_Header_uo1_ts(&buf[len],
                                             hdr.base__header().uo__1__ts(),
                                             dat, cid);
        }
        else // T = 0
        {
          len += Get_Profile1_ROHC_Header_uo1_id(&buf[len],
                                             hdr.base__header().uo__1__id(),
                                             dat, cid, hdr.ext());
        }
      }
      else // at least one RND is unset
      {
        len += Get_Profile1_ROHC_Header_uo1(&buf[len],
                                          hdr.base__header().uo__1(),
                                          dat, cid);
      }
    }
  }  

  if (!hdr.ext().is_bound())
    hdr.ext() = OMIT_VALUE;
  len += Get_IP_ID_compressed_opt(&buf[len], hdr.ip__id__outer(),
                                  dat, Outer_IP);
  len += Get_AH_data_opt(&buf[len], hdr.ah__outer(), dat, Outer_IP);
  len += Get_GRE_cksum_opt(&buf[len], hdr.gre__cksum1(), dat, Outer_IP);
  len += Get_IP_ID_compressed_opt(&buf[len], hdr.ip__id__inner(),
                                  dat, Inner_IP);
  len += Get_AH_data_opt(&buf[len], hdr.ah__inner(), dat, Inner_IP);
  len += Get_GRE_cksum_opt(&buf[len], hdr.gre__cksum2(), dat, Inner_IP);
  len += Get_UDP_cksum_opt(&buf[len], hdr.udp__cksum(), dat);

  Log_hexdump(buf, len);
  Log_function_name_on_leave();

  return len;
}                               /* Get_Profile1_ROHC_Header */

int
Get_Profile2_Dynamic_Chain(Profile2__Dynamic__Chain & dynch,
                           const unsigned char *buf, t_dat *dat)
{
  int len = 0, num;

  Log_function_name_on_enter();

  for (num = 0; num < dat->context.ip__ctx().size_of(); num++)
  {
    if (dat->context.ip__ctx()[num].version() == 4)
    {
      len += Get_ipv4_dynamic(&buf[len], dynch[num].ipv4__dyn(), dat, num);
    }
    else if (dat->context.ip__ctx()[num].version() == 6)
    {
      len += Get_ipv6_dynamic(&buf[len], dynch[num].ipv6__dyn(), dat, num);
    }
    else
    {
      int ipver = dat->context.ip__ctx()[num].version();
      TTCN_error("Invalid IP version %u on level %d", ipver, num);
    }
  }

  num = dynch.size_of();
  len += Get_udp_dynamic_w_sn(&buf[len], dynch[num].udp__dyn(), dat);

  Log_hexdump(buf, len);
  Log_function_name_on_leave();

  return len;
}

int
Get_Profile2_extension0(unsigned char *buf, Profile2__Extension0 & ext,
                        t_dat *dat)
{
  int len = 0;
  unsigned char ctemp;

  Log_function_name_on_enter();

  ctemp = (buf[len] >> 6) & 0x03;
  ext.ext__type() = BITSTRING(2, &ctemp);
  ext.sn() = (buf[len] >> 3) & 0x07;
  ext.ip__id() = buf[len] & 0x07;
  len += 1;

  Log_hexdump(buf, len);
  Log_function_name_on_leave();

  return len;
}

int
Get_Profile2_extension1(unsigned char *buf, Profile2__Extension1 & ext,
                        t_dat *dat)
{
  int len = 0;
  unsigned char ctemp;

  Log_function_name_on_enter();

  ctemp = (buf[len] >> 6) & 0x03;
  ext.ext__type() = BITSTRING(2, &ctemp);
  ext.sn() = (buf[len] >> 3) & 0x07;
  ext.ip__id() = ((buf[len] & 0x07) << 8) + (buf[len + 1] & 0xFF);
  len += 2;

  Log_hexdump(buf, len);
  Log_function_name_on_leave();

  return len;
}

int
Get_Profile2_extension2(unsigned char *buf, Profile2__Extension2 & ext,
                        t_dat *dat)
{
  int len = 0;
  unsigned char ctemp;

  Log_function_name_on_enter();

  ctemp = (buf[len] >> 6) & 0x03;
  ext.ext__type() = BITSTRING(2, &ctemp);
  ext.sn() = (buf[len] >> 3) & 0x07;
  ext.ip__id2() = ((buf[len] & 0x07) << 8) + (buf[len + 1] & 0xFF);
  len += 2;
  ext.ip__id() = buf[len];
  len += 1;

  Log_hexdump(buf, len);
  Log_function_name_on_leave();

  return len;
}

int
Get_Profile2_Inner_IP_flags_opt(unsigned char *buf,
                                OPTIONAL < Profile2__Inner__IP__flags > &field,
                                t_dat *dat, bool hasvalue)
{
  int len = 0;
  unsigned char ctemp;

  Log_function_name_on_enter();

  if (hasvalue)
  {
    ctemp = ShiftDownBit(buf[len], 7);
    field().tos() = BITSTRING(1, &ctemp);
    ctemp = ShiftDownBit(buf[len], 6);
    field().ttl() = BITSTRING(1, &ctemp);
    ctemp = ShiftDownBit(buf[len], 5);
    field().df() = BITSTRING(1, &ctemp);
    ctemp = ShiftDownBit(buf[len], 4);
    field().pr() = BITSTRING(1, &ctemp);
    ctemp = ShiftDownBit(buf[len], 3);
    field().ipx() = BITSTRING(1, &ctemp);
    ctemp = ShiftDownBit(buf[len], 2);
    field().nbo() = BITSTRING(1, &ctemp);
    ctemp = ShiftDownBit(buf[len], 1);
    field().rnd__bit() = BITSTRING(1, &ctemp);
    dat->context.ip__ctx()[getIPidx(dat, Inner_IP)].rnd__bit() = BOOLEAN(ctemp);
    ctemp = ShiftDownBit(buf[len], 0);
    field().reserved() = BITSTRING(1, &ctemp);
    len += 1;
  }
  else
    field = OMIT_VALUE;

  Log_hexdump(buf, len);
  Log_function_name_on_leave();

  return len;
}

int
Get_Profile2_Inner_IP_fields_opt(unsigned char *buf,
                                 OPTIONAL < Inner__IP__fields > &field,
                                 OPTIONAL < Profile2__Inner__IP__flags >
                                 &flags, t_dat *dat,
                                 bool hasvalue)
{
  int len = 0;

  Log_function_name_on_enter();

  if (hasvalue)
  {
    len += Get_LIN1_opt(&buf[len], field().tos(), (*flags().tos()) == 1);
    len += Get_LIN1_opt(&buf[len], field().ttl(), (*flags().ttl()) == 1);
    len += Get_LIN1_opt(&buf[len], field().proto(), (*flags().pr()) == 1);
    len += Get_ipx_headers_opt(&buf[len], field().ext__heads(), dat,
                               (*flags().ipx()) == 1, getIPidx(dat, Inner_IP));
  }
  else
    field = OMIT_VALUE;

  Log_hexdump(buf, len);
  Log_function_name_on_leave();

  return len;
}

int
Get_Profile2_extension3(unsigned char *buf, Profile2__Extension3 & ext,
                        t_dat *dat)
{
  int len = 0;
  unsigned char ctemp;

  Profile2__Inner__IP__flags inipflags;

  Log_function_name_on_enter();

  ctemp = (buf[len] >> 6) & 0x03;
  ext.ext__type() = BITSTRING(2, &ctemp);
  ctemp = ShiftDownBit(buf[len], 5);
  ext.s__bit() = BITSTRING(1, &ctemp);
  ext.mode() = (buf[len] >> 3) & 0x03;
  ctemp = ShiftDownBit(buf[len], 2);
  ext.i__bit() = BITSTRING(1, &ctemp);
  ctemp = ShiftDownBit(buf[len], 1);
  ext.ip__bit() = BITSTRING(1, &ctemp);
  ctemp = ShiftDownBit(buf[len], 0);
  ext.ip2__bit() = BITSTRING(1, &ctemp);
  len += 1;

  len +=
    Get_Profile2_Inner_IP_flags_opt(&buf[len], ext.inner__ip__flags(),
                                    dat, *ext.ip__bit() == 1);

  len += Get_Outer_IP_flags_opt(&buf[len], ext.outer__ip__flags(), dat,
                                *ext.ip2__bit() == 1);

  len += Get_LIN1_opt(&buf[len], ext.sn(), *ext.s__bit() == 1);

  len += Get_Profile2_Inner_IP_fields_opt(&buf[len], ext.inner__ip__hdr(),
                                          ext.inner__ip__flags(), dat,
                                          *ext.ip__bit() == 1);

  len += Get_LIN2_BO_LAST_opt(&buf[len], ext.ip__id(), *ext.i__bit() == 1);

  len += Get_Outer_IP_fields_opt(&buf[len], ext.outer__ip__hdr(),
                                 ext.outer__ip__flags(), dat,
                                 *ext.ip2__bit() == 1);

  Log_hexdump(buf, len);
  Log_function_name_on_leave();

  return len;
}

int
Get_Profile2_extensions_opt(unsigned char *buf,
                            OPTIONAL < Profile2__Extension > &ext,
                            t_dat *dat, bool hasvalue)
{
  int len = 0;

  Log_function_name_on_enter();

  if (hasvalue)
  {
    if ((buf[len] & 0xC0) == 0)
    {
      len += Get_Profile2_extension0(&buf[len], ext().ext0(), dat);
    }
    else if ((buf[len] & 0xC0) == 0x40)
    {
      len += Get_Profile2_extension1(&buf[len], ext().ext1(), dat);
    }
    else if ((buf[len] & 0xC0) == 0x80)
    {
      len += Get_Profile2_extension2(&buf[len], ext().ext2(), dat);
    }
    else                        /* if ((buf[len] & 0xC0) == 0xC0) */
    {
      len += Get_Profile2_extension3(&buf[len], ext().ext3(), dat);
    }
  }
  else
  {
    ext = OMIT_VALUE;
  }

  Log_hexdump(buf, len);
  Log_function_name_on_leave();

  return len;
}

int
Get_Profile2_Static_Chain(unsigned char *buf, Profile2__Static__Chain & chain,
                          t_dat *dat)
{
  int len = 0, num = 0, nextheader = 0;

  Log_function_name_on_enter();

  while (nextheader != c__ip__proto__udp)
  {
    if (((buf[len] >> 4) & 0x0F) == 4)
    {
      initIPcontext(dat, num, 4);
      len += Get_ipv4_static(&buf[len], chain[num].ipv4__stat());
      nextheader = chain[num].ipv4__stat().proto();
    }
    else if (((buf[len] >> 4) & 0x0F) == 6)
    {
      initIPcontext(dat, num, 6);
      len += Get_ipv6_static(&buf[len], chain[num].ipv6__stat());
      nextheader = chain[num].ipv6__stat().nexthead();
    }
    else
    {
      TTCN_error("Invalid IP version %u on level %u", buf[len], num);
    }
    num += 1;
  }

  len += Get_udp_static(&buf[len], chain[num].udp__stat());

  Log_hexdump(buf, len);
  Log_function_name_on_leave();

  return len;
}

int
Get_Profile2_ROHC_Header_ir(unsigned char *buf, Profile2__IR__header & bhead,
                            t_dat *dat, int cid)
{
  int len = 0;
  unsigned char ctemp;

  Log_function_name_on_enter();

  dat->context.pkt() =  Packet__type(Packet__type::IR);
  bhead.cid() = cid;
  ctemp = (buf[len] >> 1) & 0x7F;
  bhead.type__ind() = BITSTRING(7, &ctemp);
  ctemp = ShiftDownBit(buf[len], 0);
  bhead.d() = BITSTRING(1, &ctemp);
  len += 1;
  len += Get_LIN1(&buf[len], bhead.profile());
  dat->context.profile() = bhead.profile();
  len += Get_LIN1(&buf[len], bhead.crc());

  len += Get_Profile2_Static_Chain(&buf[len], bhead.stat__chain(), dat);

  if (*bhead.d() == 1)
    len += Get_Profile2_Dynamic_Chain(bhead.dyn__chain(), &buf[len], dat);
  else
    bhead.dyn__chain() = OMIT_VALUE;

  Log_hexdump(buf, len);
  Log_function_name_on_leave();

  return len;
}

int
Get_Profile2_ROHC_Header_irdyn(unsigned char *buf,
                               Profile2__IR__DYN__header & bhead,
                               t_dat *dat, int cid)
{
  int len = 0;

  Log_function_name_on_enter();

  dat->context.pkt() =  Packet__type(Packet__type::IR__DYN);
  bhead.cid() = cid;
  bhead.type__ind() = BITSTRING(8, &buf[len]);
  len += 1;
  len += Get_LIN1(&buf[len], bhead.profile());
  dat->context.profile() = bhead.profile();
  len += Get_LIN1(&buf[len], bhead.crc());

  len += Get_Profile2_Dynamic_Chain(bhead.dyn__chain(), &buf[len], dat);

  Log_hexdump(buf, len);
  Log_function_name_on_leave();

  return len;
}

int
Get_Profile2_ROHC_Header_r1(unsigned char *buf,
                            Profile2__R__1__header & bhead,
                            t_dat *dat, int cid,
                            OPTIONAL < Profile2__Extension > &ext)
{
  int len = 0;
  unsigned char ctemp;

  Log_function_name_on_enter();

  dat->context.pkt() =  Packet__type(Packet__type::R__1);
  bhead.cid() = cid;
  ctemp = (buf[len] >> 6) & 0x03;
  bhead.type__ind() = BITSTRING(2, &ctemp);
  bhead.sn() = buf[len] & 0x3F;
  len += 1;
  ctemp = ShiftDownBit(buf[len], 7);
  bhead.x__bit() = BITSTRING(1, &ctemp);
  bhead.ip__id() = buf[len] & 0x7F;
  len += 1;

  if (*bhead.x__bit())
  {
    len += Get_Profile2_extensions_opt(&buf[len], ext,
                                       dat, *bhead.x__bit());
  }

  Log_hexdump(buf, len);
  Log_function_name_on_leave();

  return len;
}

int
Get_Profile2_ROHC_Header_uo1(unsigned char *buf,
                             Profile2__UO__1__header & bhead,
                             t_dat *dat, int cid)
{
  int len = 0;
  unsigned char ctemp;

  Log_function_name_on_enter();

  dat->context.pkt() =  Packet__type(Packet__type::UO__1);
  bhead.cid() = cid;
  ctemp = (buf[len] >> 6) & 0x03;
  bhead.type__ind() = BITSTRING(2, &ctemp);
  bhead.ip__id() = buf[len] & 0x3F;
  len += 1;
  bhead.sn() = (buf[len] >> 3) & 0x1F;
  bhead.crc() = buf[len] & 0x07;
  len += 1;

  Log_hexdump(buf, len);
  Log_function_name_on_leave();

  return len;
}

int
Get_Profile2_ROHC_Header_uor2(unsigned char *buf,
                              Profile2__UOR__2__header & bhead,
                              t_dat *dat, int cid,
                              OPTIONAL < Profile2__Extension > &ext)
{
  int len = 0;
  unsigned char ctemp;

  Log_function_name_on_enter();

  dat->context.pkt() =  Packet__type(Packet__type::UOR__2);
  bhead.cid() = cid;
  ctemp = (buf[len] >> 5) & 0x07;
  bhead.type__ind() = BITSTRING(3, &ctemp);
  bhead.sn() = buf[len] & 0x1F;
  len += 1;
  ctemp = ShiftDownBit(buf[len], 7);
  bhead.x__bit() = BITSTRING(1, &ctemp);
  bhead.crc() = buf[len] & 0x7F;
  len += 1;

  if (*bhead.x__bit())
  {
    len += Get_Profile2_extensions_opt(&buf[len], ext,
                                       dat, *bhead.x__bit());
  }

  Log_hexdump(buf, len);
  Log_function_name_on_leave();

  return len;
}

int
Get_Profile2_ROHC_Header(unsigned char *buf, int length,
                         Profile2__headers & hdr,
                         t_dat *dat, int cid)
{
  int len = 0;

  Log_function_name_on_enter();

  if (isIR(buf[len]))
  {
    len += Get_Profile2_ROHC_Header_ir(&buf[len],
                                       hdr.base__header().ir(), dat, cid);
  }
  else if (isIRDYN(buf[len]))
  {
    len += Get_Profile2_ROHC_Header_irdyn(&buf[len],
                                          hdr.base__header().ir__dyn(),
                                          dat, cid);
  }
  else if (isUOR2(buf[len]))
  {
    len += Get_Profile2_ROHC_Header_uor2(&buf[len],
                                         hdr.base__header().uor__2(),
                                         dat, cid, hdr.ext());
  }
  else if (dat->context.mode() == ROHC_mode_R)
  {
    if ((buf[len] & 0xC0) == 0)
    {
      len += Get_Profile1_ROHC_Header_r0(&buf[len],
                                         hdr.base__header().r__0(),
                                         dat, cid);
    }
    else if ((buf[len] & 0xC0) == 0x40)
    {
      len += Get_Profile1_ROHC_Header_r0crc(&buf[len],
                                            hdr.base__header().r__0__crc(),
                                            dat, cid);
    }
    else if ((buf[len] & 0xC0) == 0x80)
    {
      len += Get_Profile2_ROHC_Header_r1(&buf[len],
                                         hdr.base__header().r__1(),
                                         dat, cid, hdr.ext());
    }
    else
    {
      TTCN_error("Unknown compressed packet (R mode) (0x%02x)", buf[len]);
    }    
  }
  else if (dat->context.mode() != ROHC_mode_R) // U-O modes
  {
    if ((buf[len] & 0x80) == 0)
    {
      len += Get_Profile1_ROHC_Header_uo0(&buf[len],
                                          hdr.base__header().uo__0(),
                                          dat, cid);
    }
    else if ((buf[len] & 0xC0) == 0x80)
    {
      len += Get_Profile2_ROHC_Header_uo1(&buf[len],
                                          hdr.base__header().uo__1(),
                                          dat, cid);
    }
    else
    {
      TTCN_error("Unknown compressed packet (U-O mode) (0x%02x)", buf[len]);
    } 
  }

  if (!hdr.ext().is_bound())
    hdr.ext() = OMIT_VALUE;
  len += Get_IP_ID_compressed_opt(&buf[len], hdr.ip__id__outer(),
                                  dat, Outer_IP);
  len += Get_AH_data_opt(&buf[len], hdr.ah__outer(), dat, Outer_IP);
  len += Get_GRE_cksum_opt(&buf[len], hdr.gre__cksum1(), dat, Outer_IP);
  len += Get_IP_ID_compressed_opt(&buf[len], hdr.ip__id__inner(),
                                  dat, Inner_IP);
  len += Get_AH_data_opt(&buf[len], hdr.ah__inner(), dat, Inner_IP);
  len += Get_GRE_cksum_opt(&buf[len], hdr.gre__cksum2(), dat, Inner_IP);
  len += Get_UDP_cksum_opt(&buf[len], hdr.udp__cksum(), dat);

  Log_hexdump(buf, len);
  Log_function_name_on_leave();

  return len;
}                               /* Get_Profile2_ROHC_Header */

int
Get_Profile4_ipv4_dynamic(const unsigned char *buf,
                          Profile4__IPv4__Dynamic & chain,
                          t_dat *dat, int ip_level)
{
  int len = 0;
  unsigned char ctemp;

  Log_function_name_on_enter();

  len += Get_LIN1(&buf[len], chain.tos());
  len += Get_LIN1(&buf[len], chain.ttl());
  len += Get_LIN2_BO_LAST(&buf[len], chain.identification());
  ctemp = ShiftDownBit(buf[len], 7);
  chain.df__bit() = BITSTRING(1, &ctemp);
  ctemp = ShiftDownBit(buf[len], 6);
  chain.rnd__bit() = BITSTRING(1, &ctemp);
  dat->context.ip__ctx()[ip_level].rnd__bit() = BOOLEAN(ctemp);
  ctemp = ShiftDownBit(buf[len], 5);
  chain.nbo__bit() = BITSTRING(1, &ctemp);
  ctemp = ShiftDownBit(buf[len], 4);
  chain.sid__bit() = BITSTRING(1, &ctemp);
  ctemp = buf[len] & 0x0F;
  chain.reserved() = BITSTRING(4, &ctemp);
  len += 1;
  len += Get_Encoding_Type_0(chain.genextheadlist(),
                             &buf[len], Item__list::ALT_ip__item__list,
                             dat, ip_level);

  Log_hexdump(buf, len);
  Log_function_name_on_leave();

  return len;
}

int
Get_Profile4_Dynamic_Chain(Profile4__Dynamic__Chain & dynch,
                           const unsigned char *buf, t_dat *dat)
{
  int len = 0, num;

  Log_function_name_on_enter();

  for (num = 0; num < dat->context.ip__ctx().size_of() && num < 2; num++)
  {
    if (dat->context.ip__ctx()[num].version() == 4)
    {
      len += Get_Profile4_ipv4_dynamic(&buf[len], dynch[num].ipv4__dyn(),
                                       dat, num);
    }
    else if (dat->context.ip__ctx()[num].version() == 6)
    {
      len += Get_ipv6_dynamic(&buf[len], dynch[num].ipv6__dyn(), dat, num);
    }
    else
    {
      int ipver = dat->context.ip__ctx()[num].version();
      TTCN_error("Invalid IP version %u on level %d", ipver, num);
    }
  }

  num = dynch.size_of();
  len += Get_LIN2_BO_LAST(&buf[len], dynch[num].sn());

  Log_hexdump(buf, len);
  Log_function_name_on_leave();

  return len;
}

int
Get_Profile4_Additional_Dynamic_Chain_opt(OPTIONAL < Profile4__Dynamic__Chain >
                                          &dynch,
                                          const unsigned char *buf,
                                          t_dat *dat)
{
  int len = 0;

  Log_function_name_on_enter();

  if (dat->context.ip__ctx().size_of() > 2)
  {
    for (int num = 2; num < dat->context.ip__ctx().size_of(); num++)
    {
      if (dat->context.ip__ctx()[num].version() == 4)
      {
        len += Get_Profile4_ipv4_dynamic(&buf[len], 
                                          dynch()[num - 2].ipv4__dyn(),
                                         dat, num);
      }
      else if (dat->context.ip__ctx()[num].version() == 6)
      {
        len += Get_ipv6_dynamic(&buf[len], dynch()[num - 2].ipv6__dyn(),
                                dat, num);
      }
      else
      {
        int ipver = dat->context.ip__ctx()[num].version();
        TTCN_error("Invalid IP version %u on level %d", ipver, num);
      }
    }
  }
  else
    dynch = OMIT_VALUE;

  Log_hexdump(buf, len);
  Log_function_name_on_leave();

  return len;
}

int
Get_Profile4_Static_Chain(unsigned char *buf, Profile4__Static__Chain & chain,
                          t_dat *dat)
{
  int len = 0, num = 0, nextheader = 0;
  bool terminated = false;

  Log_function_name_on_enter();

  do
  {
    /* Version does not include the MSB bit of the version field */
    int ver = (buf[len] >> 4) & 0x07;

    terminated = (buf[len] & 0x80 > 0) ? true : false;
    if (ver == c__ip__version__ipv4)
    {
      // version is stored with the MSB bit
      initIPcontext(dat, num, (buf[len] >> 4) & 0x0F);
      len += Get_ipv4_static(&buf[len], chain[num].ipv4__stat());
      nextheader = chain[num].ipv4__stat().proto();
    }
    else if (ver == c__ip__version__ipv6)
    {
      // version is stored with the MSB bit
      initIPcontext(dat, num, (buf[len] >> 4) & 0x0F);
      len += Get_ipv6_static(&buf[len], chain[num].ipv6__stat());
      nextheader = chain[num].ipv6__stat().nexthead();
    }
    else
    {
      TTCN_error("Invalid IP version %u on level %u", ver, num);
    }
    /* Explicit MSB termination */
    if (terminated)
    {
      TTCN_logger.log(TTCN_DEBUG, "MSB static chain termination on level %u",
                      num);
    }
    num += 1;
  }
  while ((nextheader == c__ip__proto__ipv4 || nextheader == c__ip__proto__ipv6) && 
          !terminated);

  Log_hexdump(buf, len);
  Log_function_name_on_leave();

  return len;
}

int
Get_Profile4_ROHC_Header_ir(unsigned char *buf, Profile4__IR__header & bhead,
                            t_dat *dat, int cid)
{
  int len = 0;
  unsigned char ctemp;

  Log_function_name_on_enter();

  dat->context.pkt() =  Packet__type(Packet__type::IR);
  bhead.cid() = cid;
  ctemp = (buf[len] >> 1) & 0x7F;
  bhead.type__ind() = BITSTRING(7, &ctemp);
  ctemp = ShiftDownBit(buf[len], 0);
  bhead.d() = BITSTRING(1, &ctemp);
  len += 1;
  len += Get_LIN1(&buf[len], bhead.profile());
  dat->context.profile() = bhead.profile();
  len += Get_LIN1(&buf[len], bhead.crc());

  len += Get_Profile4_Static_Chain(&buf[len], bhead.stat__chain(), dat);

  if (*bhead.d() == 1)
    len += Get_Profile4_Dynamic_Chain(bhead.dyn__chain(), &buf[len], dat);
  else
    bhead.dyn__chain() = OMIT_VALUE;

  Log_hexdump(buf, len);
  Log_function_name_on_leave();

  return len;
}

int
Get_Profile4_ROHC_Header_irdyn(unsigned char *buf,
                               Profile4__IR__DYN__header & bhead,
                               t_dat *dat, int cid)
{
  int len = 0;

  Log_function_name_on_enter();

  dat->context.pkt() =  Packet__type(Packet__type::IR__DYN);
  bhead.cid() = cid;
  bhead.type__ind() = BITSTRING(8, &buf[len]);
  len += 1;
  len += Get_LIN1(&buf[len], bhead.profile());
  dat->context.profile() = bhead.profile();
  len += Get_LIN1(&buf[len], bhead.crc());

  len += Get_Profile4_Dynamic_Chain(bhead.dyn__chain(), &buf[len], dat);

  Log_hexdump(buf, len);
  Log_function_name_on_leave();

  return len;
}

int
Get_Profile4_ROHC_Header_r1(unsigned char *buf,
                            Profile2__R__1__header & bhead,
                            t_dat *dat, int cid,
                            OPTIONAL < Profile2__Extension > &ext)
{
  int len = 0;
  unsigned char ctemp;

  Log_function_name_on_enter();

  dat->context.pkt() =  Packet__type(Packet__type::R__1);
  bhead.cid() = cid;
  ctemp = (buf[len] >> 6) & 0x03;
  bhead.type__ind() = BITSTRING(2, &ctemp);
  bhead.sn() = buf[len] & 0x3F;
  len += 1;
  ctemp = ShiftDownBit(buf[len], 7);
  bhead.x__bit() = BITSTRING(1, &ctemp);
  bhead.ip__id() = buf[len] & 0x7F;
  len += 1;

  if (*bhead.x__bit())
  {
    len += Get_Profile2_extensions_opt(&buf[len], ext,
                                       dat, *bhead.x__bit());
  }

  Log_hexdump(buf, len);
  Log_function_name_on_leave();

  return len;
}

int
Get_Profile4_ROHC_Header_uor2(unsigned char *buf,
                              Profile2__UOR__2__header & bhead,
                              t_dat *dat, int cid,
                              OPTIONAL < Profile2__Extension > &ext)
{
  int len = 0;
  unsigned char ctemp;

  Log_function_name_on_enter();

  dat->context.pkt() =  Packet__type(Packet__type::UOR__2);
  bhead.cid() = cid;
  ctemp = (buf[len] >> 5) & 0x07;
  bhead.type__ind() = BITSTRING(3, &ctemp);
  bhead.sn() = buf[len] & 0x1F;
  len += 1;
  ctemp = ShiftDownBit(buf[len], 7);
  bhead.x__bit() = BITSTRING(1, &ctemp);
  bhead.crc() = buf[len] & 0x7F;
  len += 1;

  if (*bhead.x__bit())
  {
    len += Get_Profile2_extensions_opt(&buf[len], ext,
                                       dat, *bhead.x__bit());
  }

  Log_hexdump(buf, len);
  Log_function_name_on_leave();

  return len;
}

int
Get_Profile4_ROHC_Header(unsigned char *buf, int length,
                         Profile4__headers & hdr, t_dat *dat,
                         int cid)
{
  int len = 0;

  Log_function_name_on_enter();

  if (isIR(buf[len]))
  {
    len += Get_Profile4_ROHC_Header_ir(&buf[len],
                                       hdr.base__header().ir(), dat, cid);
  }
  else if (isIRDYN(buf[len]))
  {
    len += Get_Profile4_ROHC_Header_irdyn(&buf[len],
                                          hdr.base__header().ir__dyn(),
                                          dat, cid);
  }
  else if (isUOR2(buf[len]))
  {
    len += Get_Profile4_ROHC_Header_uor2(&buf[len],
                                         hdr.base__header().uor__2(),
                                         dat, cid, hdr.ext());
  }
  else if (dat->context.mode() == ROHC_mode_R)
  {
    if ((buf[len] & 0xC0) == 0)
    {
      len += Get_Profile1_ROHC_Header_r0(&buf[len],
                                         hdr.base__header().r__0(),
                                         dat, cid);
    }
    else if ((buf[len] & 0xC0) == 0x40)
    {
      len += Get_Profile1_ROHC_Header_r0crc(&buf[len],
                                            hdr.base__header().r__0__crc(),
                                            dat, cid);
    }
    else if ((buf[len] & 0xC0) == 0x80)
    {
      len += Get_Profile4_ROHC_Header_r1(&buf[len],
                                         hdr.base__header().r__1(),
                                         dat, cid, hdr.ext());
    }
    else
    {
      TTCN_error("Unknown compressed packet (R mode) (0x%02x)", buf[len]);
    }
  }
  else if (dat->context.mode() != ROHC_mode_R) // U-O modes
  {
    if ((buf[len] & 0x80) == 0)
    {
      len += Get_Profile1_ROHC_Header_uo0(&buf[len],
                                          hdr.base__header().uo__0(),
                                          dat, cid);
    }
    else if ((buf[len] & 0xC0) == 0x80)
    {
      len += Get_Profile2_ROHC_Header_uo1(&buf[len],
                                          hdr.base__header().uo__1(),
                                          dat, cid);
    }
    else
    {
      TTCN_error("Unknown compressed packet (U-O modes) (0x%02x)", buf[len]);
    }
  }

  if (!hdr.ext().is_bound())
    hdr.ext() = OMIT_VALUE;
  len += Get_IP_ID_compressed_opt(&buf[len], hdr.ip__id__outer(),
                                  dat, Outer_IP);
  len += Get_AH_data_opt(&buf[len], hdr.ah__outer(), dat, Outer_IP);
  len += Get_GRE_cksum_opt(&buf[len], hdr.gre__cksum1(), dat, Outer_IP);
  len += Get_IP_ID_compressed_opt(&buf[len], hdr.ip__id__inner(),
                                  dat, Inner_IP);
  len += Get_AH_data_opt(&buf[len], hdr.ah__inner(), dat, Inner_IP);
  len += Get_GRE_cksum_opt(&buf[len], hdr.gre__cksum2(), dat, Inner_IP);
  len += Get_Profile4_Additional_Dynamic_Chain_opt(hdr.additional__IP(),
                                                   &buf[len], dat);

  Log_hexdump(buf, len);
  Log_function_name_on_leave();

  return len;
}                               /* Get_Profile4_ROHC_Header */

int
Get_ROHC_Header(const unsigned char *p_buf,
                int length, Header & p_header, ROHC__config & config)
{
  unsigned char buf[MAX_PACKET_SIZE];
  int len = 0, cidlen = 0;
  INTEGER cid;
    
  Log_function_name_on_enter();

  memcpy(buf, p_buf, length);

  if (config.large__cid())
  {
    /* Skip packet type octet */
    cidlen = Get_CID(&p_buf[1], cid, config);
    /* Move the packet type octet right after the CID */
    buf[cidlen] = buf[0];
  }
  else
  {
    cidlen = Get_CID(&p_buf[0], cid, config);
  }
  len += cidlen;
  
  if (isIR(buf[len]) || isIRDYN(buf[len]))
  {
    config.context()[cid].profile() = buf[len + 1];
  }
  
  t_dat dat = initTDAT(config, cid);
  initCIDcontext(&dat);
  
  switch (config.context()[cid].profile())
  {
  case 0:
    len += Get_Profile0_ROHC_Header(&buf[len], &dat,
                                    length - cidlen, p_header.prof0(),
                                    (int) cid);
    break;
  case 1:
    len += Get_Profile1_ROHC_Header(&buf[len], &dat, length - cidlen,
                                    p_header.prof1(), (int) cid);
    break;
  case 2:
    len += Get_Profile2_ROHC_Header(&buf[len], length - cidlen,
                                    p_header.prof2(), &dat,
                                    (int) cid);
    break;
  case 4:
    len += Get_Profile4_ROHC_Header(&buf[len], length - cidlen,
                                    p_header.prof4(), &dat,
                                    (int) cid);
    break;
  default:
    break;
  }
  
  Log_hexdump(buf, len);
  Log_function_name_on_leave();

  return len;
}

ROHC__packet__u
f__ROHC__dec(OCTETSTRING const &data, ROHC__config & p_config)
{
  ROHC__packet__u packet;
  OPTIONAL < Padding > padding;
  OPTIONAL < Feedback > feedback;
  const unsigned char *buf = (const unsigned char *) data;
  int len = 0;

  Log_function_name_on_enter();

  Log_object(p_config);
  
  // Processing padding
  Log_hexdump(buf, data.lengthof() - len);
  len += Get_ROHC_Padding(&buf[len], data.lengthof() - len, padding, p_config);
  // Processing feedback
  len += Get_ROHC_Feedback(&buf[len], data.lengthof() - len, feedback, p_config);
  // Processing ROHC header
  if ((buf[len] & 0xFE) == 0xFE)
  {
    packet.segment().padding() = padding;
    packet.segment().feedback() = feedback;
    packet.segment().segment__header__id() = 0x7F;
    if (ShiftDownBit(buf[len], 0))
    {
      packet.segment().final() = BOOLEAN(true);
      len += 1;
      if (data.lengthof() - len < 4)
      {
        packet.segment().payload() =
          OCTETSTRING(data.lengthof() - len, &buf[len]);
        len = data.lengthof();
        packet.segment().crc() = OMIT_VALUE;
      }
      else
      {
        packet.segment().payload() =
          OCTETSTRING(data.lengthof() - len - 4, &buf[len]);
        len = data.lengthof() - 4;
        packet.segment().crc() =
          ((buf[len] << 24) & 0xFF000000) +
          ((buf[len + 1] << 16) & 0xFF0000) +
          ((buf[len + 2] << 8) & 0xFF00) + (buf[len + 3] & 0xFF);
      }
    }
    else
    {
      packet.segment().final() = BOOLEAN(false);
      len += 1;
      packet.segment().payload() =
        OCTETSTRING(data.lengthof() - len, &buf[len]);
      packet.segment().crc() = OMIT_VALUE;
    }

  }
  else
  {
    packet.rohc().padding() = padding;
    packet.rohc().feedback() = feedback;
    len += Get_ROHC_Header(&buf[len],
                           data.lengthof() - len, packet.rohc().header(),
                           p_config);
    if (data.lengthof() == len)
    {
      packet.rohc().payload() = OMIT_VALUE;
    }
    else
    {
      packet.rohc().payload() = OCTETSTRING(data.lengthof() - len, &buf[len]);
    }
  }

  TTCN_logger.begin_event(TTCN_DEBUG);
  p_config.log();
  packet.log();
  TTCN_logger.end_event();

  Log_function_name_on_leave();

  return packet;
}

INTEGER
f__ROHC__CRC(OCTETSTRING const &data, INTEGER const &crclen)
{
  const unsigned char *buf = (const unsigned char *) data;
  INTEGER crcval;
  int crclength = crclen;

  Log_function_name_on_enter();

  TTCN_logger.log(TTCN_DEBUG, "Calculating CRC-%d over:", crclength);
  Log_hexdump(buf, data.lengthof());

  crcval = INTEGER(ComputeCRC(&(buf[0]), data.lengthof(), crclength));
  Log_function_name_on_leave();
  return crcval;
}

OCTETSTRING
f__FBCK__enc(Feedback__data const &feedback_data, ROHC__config const &config)
{
  unsigned char buf[MAX_PACKET_SIZE];
  int len = 0, crcpos = 0;
  INTEGER cid;

  Log_function_name_on_enter();

  cid = feedback_data.cid();
  if (config.large__cid() == false)
  {
    if (cid > 0)
    {
      buf[len] = 0xE0 + (cid & 0x0F);
      len += 1;
    }
  }
  else
    len += Set_SDVL_field(&buf[len], cid, 0);
  switch (feedback_data.feedback__type().get_selection())
  {
  case Feedback__type::ALT_feedback1:
    const unsigned char *tempbuf;
    tempbuf =
      (const unsigned char *) feedback_data.feedback__type().feedback1();
    buf[len] = tempbuf[0];
    len += 1;
    break;
  case Feedback__type::ALT_feedback2:
    {
      Feedback2 fback2;
      fback2 = feedback_data.feedback__type().feedback2();
      buf[len] = (fback2.acktype() << 6) & 0xC0;
      buf[len] += (fback2.mode() << 4) & 0x30;
      buf[len] += (fback2.sn() >> 8) & 0x0F;
      len += 1;
      buf[len] = fback2.sn() & 0xFF;
      len += 1;
      if (fback2.feedback__opts().ispresent())
      {
        Feedback__opts & fbopts = fback2.feedback__opts();
        for (int num2 = 0; num2 < fbopts.size_of(); num2++)
        {
          buf[len] = (fbopts[num2].opt__type() << 4) & 0xF0;
          buf[len] += fbopts[num2].opt__len() & 0x0F;
          len += 1;
          len += Set_octetstring_opt(&buf[len], fbopts[num2].opt__data());
          if (fbopts[num2].opt__data().ispresent())
          {
            OCTETSTRING opt;
            opt = fbopts[num2].opt__data();
            if (fbopts[num2].opt__len() != opt.lengthof())
              TTCN_logger.
                log(TTCN_WARNING,
                    "Feedback option length mismatch: %d, %d",
                    (int) fbopts[num2].opt__len(), opt.lengthof());
            else
              if ((fbopts[num2].opt__len() == 1)
                  && (fbopts[num2].opt__type() == 1) && (buf[len - 1] == 0))
              crcpos = len - 1;
          }
        }
      }
      break;
    }

  default:
    break;
  }

  if ((crcpos > 0) && (buf[crcpos] == 0))
    buf[crcpos] = ComputeCRC(&(buf[0]), len, 8);

  Log_function_name_on_leave();

  return OCTETSTRING(len, &(buf[0]));
}

Feedback__data
f__FBCK__dec(OCTETSTRING const &data, ROHC__config const &config)
{
  Feedback__data feedback_data;
  const unsigned char *buf;
  unsigned char ctemp;
  int len = 0, fbsize, cidlen = 0;
  buf = (const unsigned char *) data;
  fbsize = data.lengthof();

  Log_function_name_on_enter();

  if (config.large__cid() == false)
  {
    if ((buf[len] & 0xF0) == 0xE0)
    {
      feedback_data.cid() = buf[len] & 0x0F;
      len += 1;
      fbsize -= 1;
    }
    else
      feedback_data.cid() = 0;
  }
  else
  {
    cidlen = Get_SDVL_field(&buf[len], feedback_data.cid());
    len += cidlen;
    fbsize -= cidlen;
  }

  if (fbsize == 1)
  {
    feedback_data.feedback__type().feedback1() = OCTETSTRING(1, &buf[len]);
    len += 1;
  }
  else if (fbsize >= 1)
  {
    int optnum;
    Feedback__opts fbopts;
    Feedback2 feedback2;
    feedback2.acktype() = (buf[len] >> 6) & 0x03;
    feedback2.mode() = (buf[len] >> 4) & 0x03;
    ctemp = buf[len] & 0x0F;
    len += 1;
    fbsize -= 1;
    feedback2.sn() = ((ctemp << 8) & 0x0F00) + buf[len];
    len += 1;
    fbsize -= 1;
    optnum = 0;
    feedback2.feedback__opts() = OMIT_VALUE;
    while (fbsize)
    {
      fbopts[optnum].opt__type() = (buf[len] >> 4) & 0x0F;
      fbopts[optnum].opt__len() = buf[len] & 0x0F;
      len += 1;
      fbsize -= 1;
      if (fbopts[optnum].opt__len())
      {
        len += Get_octetstring(&buf[len], fbopts[optnum].opt__data(),
                               fbopts[optnum].opt__len());
        fbsize -= fbopts[optnum].opt__len();
      }
      else
        fbopts[optnum].opt__data() = OMIT_VALUE;
      feedback2.feedback__opts() = fbopts;
      optnum += 1;
    }

    feedback_data.feedback__type().feedback2() = feedback2;
  }
  else
    TTCN_logger.log(TTCN_WARNING, "Size mismatch in feedback(%d)", fbsize);

  Log_function_name_on_leave();

  return feedback_data;
}

}//namespace
