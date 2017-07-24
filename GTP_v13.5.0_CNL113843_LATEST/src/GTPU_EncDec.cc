///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// Copyright Test Competence Center (TCC) ETH 2016                           //
//                                                                           //
// The copyright to the computer  program(s) herein  is the property of TCC. //
// The program(s) may be used and/or copied only with the written permission //
// of TCC or in accordance with  the terms and conditions  stipulated in the //
// agreement/contract under which the program(s) has been supplied.          //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////
//
//  File:               GTPU_EncDec.cc
//  Rev:                R1B
//  Prodnr:             CNL 113 843
//  Contact:            http://ttcn.ericsson.se
//  Reference:          3GPP TS 29.060 v13.5.0

#include "GTPU_Types.hh"

namespace GTPU__Types {

// find the length of the optional part and decode optional part into OPT_PART   
int find_optpart_length(const unsigned char * opt_part_ptr,GTPU__Header__optional__part& OPT_PART)// pointer to opt part start
{
  int opt_part_length = 4; // mandatory minimum length of opt_part  
  OPT_PART.sequenceNumber() = OCTETSTRING(2,opt_part_ptr);
  OPT_PART.npduNumber() = OCTETSTRING(1,opt_part_ptr+2);
  OPT_PART.nextExtHeader() = OCTETSTRING(1,opt_part_ptr+3);
  OPT_PART.gTPU__extensionHeader__List() = OMIT_VALUE; 
   
      int i = 0;    
      bool opt_part_end = false;
      while(!opt_part_end) 
      {                  
        if (opt_part_ptr[opt_part_length-1] != 0x00) // 0x00 means end of optional part
            {
             unsigned char lengthfield = opt_part_ptr[opt_part_length];
             
             OPT_PART.gTPU__extensionHeader__List()()[i].lengthfield() = lengthfield;
             OPT_PART.gTPU__extensionHeader__List()()[i].content() = 
               OCTETSTRING(4*lengthfield-2,opt_part_ptr + opt_part_length +1);
             OPT_PART.gTPU__extensionHeader__List()()[i].nextExtHeader() = 
               OCTETSTRING(1,opt_part_ptr + opt_part_length + 4*lengthfield - 1);
                   
             opt_part_length = opt_part_length + 4*lengthfield;
             i++;
            }
        else            
            {opt_part_end = true;}                         
      }
       
  return  opt_part_length;
}

//////////////////////////////////
// Decoding function for GTPC__DialoguePDU
//////////////////////////////////
PDU__GTPU dec__PDU__GTPU(const OCTETSTRING& udp__pdu)
{
  TTCN_Buffer buf;
  PDU__GTPU pdu;  
       
      const unsigned char *gtpu_message = (const unsigned char *) udp__pdu;
       
      int opt_part_length = 0;
      if  ( gtpu_message[0] & 0x07 ) // opt_part is present 
      { 
         GTPU__Header__optional__part    OPT_PART;
         
         // find the length of the optional part and decode optional part into OPT_PART                       
         opt_part_length = find_optpart_length(gtpu_message+8,OPT_PART); 
         if(  ((gtpu_message[2] << 8) + gtpu_message[3] - opt_part_length) < 0  )
           {TTCN_error("Decoding error, lengthf field is shorter that decoded length of opt_part");};         
                                                             
         // build PDU without optional part 
         unsigned int gtpu_IEs_length = (gtpu_message[2] << 8) + gtpu_message[3] - opt_part_length;
         unsigned char gtpuBuf[8 + gtpu_IEs_length];
         memcpy(gtpuBuf,gtpu_message,8);
         memcpy(gtpuBuf+8,gtpu_message+8+opt_part_length,gtpu_IEs_length);
         
         // substitute dummy bits (indicating there is no optional part)
         gtpuBuf[0] = gtpuBuf[0] & 0xf8;
         
         // substitute dummy length (not including optional part)
         gtpuBuf[2] = (gtpu_IEs_length & 0xff00) >> 8;
         gtpuBuf[3] =  gtpu_IEs_length & 0xff;
         
         // RAW decoding
         buf.put_s(8 + gtpu_IEs_length,gtpuBuf);        
         pdu.decode(PDU__GTPU_descr_, buf, TTCN_EncDec::CT_RAW);
         buf.clear(); 
         
         // put back the original values               
         unsigned char pn = gtpu_message[0] & 0x01;
         pdu.pn__bit() = BITSTRING(1,&pn);
    
         unsigned char s = (gtpu_message[0] & 0x02) >> 1;
         pdu.s__bit() =  BITSTRING(1,&s);
      
         unsigned char e = (gtpu_message[0] & 0x04) >> 2;
         pdu.e__bit() = BITSTRING(1,&e);
         
         pdu.lengthf() = (gtpu_message[2] << 8) + gtpu_message[3];
         
         pdu.opt__part() = OPT_PART;
                
         return pdu;                 
      }   
      else // opt_part is not present 
      {
         buf.put_os(udp__pdu);
         pdu.decode(PDU__GTPU_descr_, buf, TTCN_EncDec::CT_RAW);
         buf.clear(); 
         return pdu;             
      }                  
}  // end of function


}//namespace
