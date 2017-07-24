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
//  File:               GTPC_EncDec.cc
//  Rev:                R1B
//  Prodnr:             CNL 113 843
//  Contact:            http://ttcn.ericsson.se
//  Reference:          3GPP TS 29.060 v13.5.0

#include "GTPC_Types.hh"

namespace GTPC__Types {


// find the length of the optional part and decode optional part into OPT_PART   
int find_optpart_length(const unsigned char * opt_part_ptr,GTPC__Header__optional__part& OPT_PART)// pointer to opt part start
{
  int opt_part_length = 4; // mandatory minimum length of opt_part  
  OPT_PART.sequenceNumber() = OCTETSTRING(2,opt_part_ptr);
  OPT_PART.npduNumber() = OCTETSTRING(1,opt_part_ptr+2);
  OPT_PART.nextExtHeader() = OCTETSTRING(1,opt_part_ptr+3);
  OPT_PART.gTPC__extensionHeader__List() = OMIT_VALUE; 
   
      int i = 0;    
      bool opt_part_end = false;
      while(!opt_part_end) 
      {                  
        if (opt_part_ptr[opt_part_length-1] != 0x00) // 0x00 means end of optional part
            {
             unsigned char lengthfield = opt_part_ptr[opt_part_length];
             
             OPT_PART.gTPC__extensionHeader__List()()[i].lengthfield() = lengthfield;
             OPT_PART.gTPC__extensionHeader__List()()[i].content() = 
               OCTETSTRING(4*lengthfield-2,opt_part_ptr + opt_part_length +1);
             OPT_PART.gTPC__extensionHeader__List()()[i].nextExtHeader() = 
               OCTETSTRING(1,opt_part_ptr + opt_part_length + 4*lengthfield - 1);
                   
             opt_part_length = opt_part_length + 4*lengthfield;
             i++;
            }
        else            
            {opt_part_end = true;}                         
      }
         
  return  opt_part_length;
}


void dec__PDU__GTPC_no_optional_part(const unsigned char * udp__pdu,const int pl_udp_pdu_length, const SystemUnderTest& pl__SystemUnderTest,PDU__GTPC& pdu)
{
  TTCN_Buffer buf;
  
 if ((pl__SystemUnderTest == SystemUnderTest::GGSN) || (pl__SystemUnderTest == SystemUnderTest::CGW))
  // testing GGSN or CGW (not default)
  {
    const unsigned char *gtpc_message = (const unsigned char *) udp__pdu;
    if ((gtpc_message[1] == 0x12) || (gtpc_message[1] == 0x13))  
    // if updatePDPContextRequest or updatePDPContextResponse message is received from GGSN or C-GW
    {   
      unsigned char pn = gtpc_message[0] & 0x01;
      pdu.pn__bit() = BITSTRING(1,&pn);
    
      unsigned char s = (gtpc_message[0] & 0x02) >> 1;
      pdu.s__bit() =  BITSTRING(1,&s);
      
      unsigned char e = (gtpc_message[0] & 0x04) >> 2;
      pdu.e__bit() = BITSTRING(1,&e);

      unsigned char spare = (gtpc_message[0] & 0x08) >> 3;
      pdu.spare() = BITSTRING(1,&spare ); 
        
      unsigned char pt = (gtpc_message[0] & 0x10) >> 4;
      pdu.pt() = BITSTRING(1,&pt );  
        
      unsigned char version = ((gtpc_message[0] & 0x80) >> 7) |
                            ((gtpc_message[0] & 0x40) >> 5) |
                            ((gtpc_message[0] & 0x20) >> 3) ;    
      pdu.version() =  BITSTRING(3,&version); 
    
      pdu.messageType() = OCTETSTRING(1,gtpc_message+1); 
    
      pdu.lengthf() = (gtpc_message[2] << 8) + gtpc_message[3];
    
      pdu.teid() = OCTETSTRING(4,gtpc_message+4); 
          
      pdu.opt__part() = OMIT_VALUE;           

      if (gtpc_message[1] == 0x12) //updatePDPContextRequest
      {
        if(pl__SystemUnderTest == SystemUnderTest::GGSN) // from GGSN
        {
         UpdatePDPContextRequestGGSN updatePDPContextRequestGGSN;           
         OCTETSTRING updatePDPContextRequestGGSN_string = OCTETSTRING(pl_udp_pdu_length-8,udp__pdu+8 );      
         buf.put_os(updatePDPContextRequestGGSN_string);      
         updatePDPContextRequestGGSN.decode(UpdatePDPContextRequestGGSN_descr_,buf,TTCN_EncDec::CT_RAW);    
         pdu.gtpc__pdu().updatePDPContextRequest().updatePDPContextRequestGGSN() = updatePDPContextRequestGGSN;
        } 
        else   // from CGW
        {
         UpdatePDPContextRequestCGW updatePDPContextRequestCGW;
         OCTETSTRING updatePDPContextRequestCGW_string = OCTETSTRING(pl_udp_pdu_length-8,udp__pdu+8 );       
         buf.put_os(updatePDPContextRequestCGW_string);        
         updatePDPContextRequestCGW.decode(UpdatePDPContextRequestCGW_descr_,buf,TTCN_EncDec::CT_RAW);         
         pdu.gtpc__pdu().updatePDPContextRequest().updatePDPContextRequestCGW() = updatePDPContextRequestCGW;        
        }  
      } 
      else //updatePDPContextResponse  
      { 
        if(pl__SystemUnderTest == SystemUnderTest::GGSN) // from GGSN
        {                  
         UpdatePDPContextResponseGGSN updatePDPContextResponseGGSN;           
         OCTETSTRING updatePDPContextResponseGGSN_string = OCTETSTRING(pl_udp_pdu_length-8,udp__pdu+8 );    
         buf.put_os(updatePDPContextResponseGGSN_string);      
         updatePDPContextResponseGGSN.decode(UpdatePDPContextResponseGGSN_descr_,buf,TTCN_EncDec::CT_RAW);    
         pdu.gtpc__pdu().updatePDPContextResponse().updatePDPContextResponseGGSN() = updatePDPContextResponseGGSN;
        }
        else   // from CGW
        {        
         UpdatePDPContextResponseCGW updatePDPContextResponseCGW;
         OCTETSTRING updatePDPContextResponseCGW_string = OCTETSTRING(pl_udp_pdu_length-8,udp__pdu+8 );  
         buf.put_os(updatePDPContextResponseCGW_string);  
         updatePDPContextResponseCGW.decode(UpdatePDPContextResponseCGW_descr_,buf,TTCN_EncDec::CT_RAW);
         pdu.gtpc__pdu().updatePDPContextResponse().updatePDPContextResponseCGW() = updatePDPContextResponseCGW;                
        } 
      }
        
      buf.clear();  
      return;      
    }
    else // if message not equal to updatePDPContextRequest or updatePDPContextResponse is received from GGSN or C-GW
    {
      buf.put_s(pl_udp_pdu_length,udp__pdu);
      pdu.decode(PDU__GTPC_descr_, buf, TTCN_EncDec::CT_RAW);
      buf.clear(); 
      return;
    } 
  }
  else  //pl__SystemUnderTest is SGSN or MME (this is default)
  {
    buf.put_s(pl_udp_pdu_length,udp__pdu);
    pdu.decode(PDU__GTPC_descr_, buf, TTCN_EncDec::CT_RAW);
    buf.clear();  
    return;
  }
}

//////////////////////////////////
// Decoding function for GTPC__DialoguePDU
//////////////////////////////////

PDU__GTPC dec__PDU__GTPC(const OCTETSTRING& udp__pdu,const SystemUnderTest& pl__SystemUnderTest)
{  
      PDU__GTPC pdu;  
       
      const unsigned char *gtpc_message = (const unsigned char *) udp__pdu;
       
      int opt_part_length = 0;
      if  ( gtpc_message[0] & 0x07 ) // opt_part is present 
      { 
         GTPC__Header__optional__part    OPT_PART;
         
         // find the length of the optional part and decode optional part into OPT_PART                       
         opt_part_length = find_optpart_length(gtpc_message+8,OPT_PART);                               
         if(  ((gtpc_message[2] << 8) + gtpc_message[3] - opt_part_length) < 0  )
           {TTCN_error("Decoding error, lengthf field is shorter that decoded length of opt_part");}  
                      
         // build PDU without optional part                  
         unsigned int gtpc_IEs_length = (gtpc_message[2] << 8) + gtpc_message[3] - opt_part_length;
         unsigned char gtpcBuf[8 + gtpc_IEs_length];
         memcpy(gtpcBuf,gtpc_message,8);
         memcpy(gtpcBuf+8,gtpc_message+8+opt_part_length,gtpc_IEs_length);
         
         // substitute dummy bits (indicating there is no optional part)
         gtpcBuf[0] = gtpcBuf[0] & 0xf8;
         
         // substitute dummy length (not including optional part)
         gtpcBuf[2] = (gtpc_IEs_length & 0xff00) >> 8;
         gtpcBuf[3] =  gtpc_IEs_length & 0xff;

         // call decoding function                 
         dec__PDU__GTPC_no_optional_part(gtpcBuf,udp__pdu.lengthof() - opt_part_length,pl__SystemUnderTest,pdu);
             
         // put back the original values               
         unsigned char pn = gtpc_message[0] & 0x01;
         pdu.pn__bit() = BITSTRING(1,&pn);
    
         unsigned char s = (gtpc_message[0] & 0x02) >> 1;
         pdu.s__bit() =  BITSTRING(1,&s);
      
         unsigned char e = (gtpc_message[0] & 0x04) >> 2;
         pdu.e__bit() = BITSTRING(1,&e);
         
         pdu.lengthf() = (gtpc_message[2] << 8) + gtpc_message[3];
         
         pdu.opt__part() = OPT_PART;
                
         return pdu;                 
      }   
      else // opt_part is not present 
      {       
         dec__PDU__GTPC_no_optional_part(gtpc_message,udp__pdu.lengthof(),pl__SystemUnderTest,pdu);
         return pdu;             
      }                  
}  // end of function



}//namespace
